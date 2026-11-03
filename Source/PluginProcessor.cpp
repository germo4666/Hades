#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace hd
{
// Source mode re-voices every module so the same knob positions behave well on that kind of sound.
struct Voicing { float heat, tape, drift, glue, grain, space; float forceLowHz; float heatLoFloor; float transStrength; };
static const Voicing kVoicing[6] = {
    //  heat  tape  drift glue  grain space  lowHz  heatLo trans
    { 1.0f, 1.0f, 0.60f, 1.10f, 0.9f, 1.0f,  0.f,   0.f,  0.85f }, // Drums
    { 1.0f, 1.0f, 0.35f, 1.00f, 0.5f, 0.4f, 90.f,   0.f,  0.70f }, // Bass
    { 1.0f, 1.0f, 1.15f, 1.00f, 1.0f, 1.0f,  0.f,   0.f,  0.70f }, // Keys
    { 1.1f, 1.0f, 1.00f, 1.00f, 1.0f, 1.1f,  0.f,   0.f,  0.70f }, // Guitar
    { 0.8f, 1.0f, 0.50f, 1.10f, 0.8f, 1.0f,  0.f, 150.f,  0.70f }, // Vocal
    { 0.5f, 0.6f, 0.40f, 0.80f, 0.5f, 0.5f,  0.f,   0.f,  0.70f }, // Bus
};

HadesProcessor::HadesProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "HADES", createLayout())
{
    for (int i = 0; i < NUM_PARAMS; ++i) raw[(size_t) i] = apvts.getRawParameterValue (paramDefs()[i].id);
    os[1] = std::make_unique<juce::dsp::Oversampling<float>> (2, 1, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
    os[2] = std::make_unique<juce::dsp::Oversampling<float>> (2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
    heatFluxR.seed (61); glueFluxR.seed (62);
    loadPreset (0);
}

bool HadesProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    if (l.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    const auto in = l.getMainInputChannelSet();
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

void HadesProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    maxBlock = std::max (samplesPerBlock, kChunk);
    for (auto* b : { &io, &dryIn, &work, &colorDry, &lowBand, &wetG, &wetS, &tmp }) b->setSize (2, maxBlock, false, true, false);
    for (int i = 1; i < 3; ++i) { os[i]->reset(); os[i]->initProcessing ((size_t) kChunk); }

    drift.prepare (sr); glue.prepare (sr); grain.prepare (sr); space.prepare (sr); dust.prepare (sr); luMatch.prepare (sr);
    lowDelay.prepare (8192);
    for (auto& d : colorDelay) d.prepare (2048);
    fastEnv.prepare (sr, 0.5f, 25.f);
    slowEnv.prepare (sr, 30.f, 160.f);
    osIndex = -1; // force re-init of heat/tape at the oversampled rate
    for (auto& e : eqSet) e = -1;
    lastTilt = 999.f; lastLowFreq = -1;
    resetEverything();
    updateLatency();
}

void HadesProcessor::resetEverything()
{
    for (int c = 0; c < 2; ++c)
    {
        eqHp[c].reset(); eqLs[c].reset(); eqPk[c].reset(); eqHs[c].reset();
        tiltLs[c].reset(); tiltHs[c].reset();
        lpA[c].reset(); lpB[c].reset(); hpA[c].reset(); hpB[c].reset();
        dcX[c] = dcY[c] = 0;
        colorDelay[c].reset();
    }
    lowDelay.reset();
    heat.reset(); tape.reset(); drift.reset(); glue.reset(); grain.reset(); space.reset();
}

void HadesProcessor::updateLatency()
{
    const int want = juce::jlimit (0, 2, geti (oversample));
    if (want != osIndex)
    {
        osIndex = want;
        const double osr = sr * (double) (1 << osIndex);
        heat.prepare (osr); tape.prepare (osr);
        if (osIndex > 0) os[osIndex]->reset();
        osLatency = osIndex > 0 ? (int) std::round (os[osIndex]->getLatencyInSamples()) : 0;
        totalLatency = osLatency + drift.latency();
        setLatencySamples (totalLatency);
    }
}

double HadesProcessor::currentBpm() const
{
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                return juce::jlimit (30.0, 300.0, *bpm);
    return 120.0;
}

void HadesProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (n <= 0) return;
    if (n > io.getNumSamples())
        for (auto* b : { &io, &dryIn, &work, &colorDry, &lowBand, &wetG, &wetS, &tmp }) b->setSize (2, n, false, true, true);

    const int numIn = std::max (1, getTotalNumInputChannels());
    for (int c = 0; c < 2; ++c) io.copyFrom (c, 0, buffer, std::min (c, numIn - 1), 0, n);

    float inPk = 0;
    for (int c = 0; c < 2; ++c) inPk = std::max (inPk, io.getMagnitude (c, 0, n));
    inLevel.store (std::max (inPk, inLevel.load() * 0.9f));

    updateLatency();
    bpmCache = currentBpm();

    for (int pos = 0; pos < n; pos += kChunk) processChunk (pos, std::min (kChunk, n - pos));

    // safety: never let a NaN/inf reach the host
    bool bad = false;
    for (int c = 0; c < 2 && ! bad; ++c)
    {
        const float* p = io.getReadPointer (c);
        for (int i = 0; i < n; ++i) if (! std::isfinite (p[i])) { bad = true; break; }
    }
    if (bad) { io.clear(); resetEverything(); }

    for (int c = 0; c < buffer.getNumChannels(); ++c) buffer.copyFrom (c, 0, io, std::min (c, 1), 0, n);

    float outPk = 0;
    for (int c = 0; c < 2; ++c) outPk = std::max (outPk, io.getMagnitude (c, 0, n));
    outLevel.store (std::max (outPk, outLevel.load() * 0.9f));

    int wpos = scopeWrite.load();
    const float* l = io.getReadPointer (0);
    const float* r = io.getReadPointer (1);
    for (int i = 0; i < n; ++i) { scope[(size_t) wpos] = 0.5f * (l[i] + r[i]); wpos = (wpos + 1) & 4095; }
    scopeWrite.store (wpos);
}

void HadesProcessor::processChunk (int start, int len)
{
    float* x[2] = { io.getWritePointer (0) + start, io.getWritePointer (1) + start };
    float* dry[2] = { dryIn.getWritePointer (0) + start, dryIn.getWritePointer (1) + start };
    float* low = lowBand.getWritePointer (0) + start;
    float* wg[2] = { wetG.getWritePointer (0) + start, wetG.getWritePointer (1) + start };
    float* ws[2] = { wetS.getWritePointer (0) + start, wetS.getWritePointer (1) + start };

    const Voicing& v = kVoicing[juce::jlimit (0, 5, geti (source))];
    const float age = get (P::age);
    const float fluxRateV = get (fluxRate), chaos = get (fluxChaos);

    // 1. loudness reference = the raw plugin input
    luMatch.measureIn (x, len);

    // 2. input gain
    const float ig = dbToGain (get (inGain));
    for (int c = 0; c < 2; ++c) { juce::FloatVectorOperations::multiply (x[c], ig, len); juce::FloatVectorOperations::copy (dry[c], x[c], len); }

    // 3. low protect (LR4 split; lows bypass the chain, mono, latency-aligned)
    const bool lpOn = get (lowProt) > 0.5f || v.forceLowHz > 0.f;
    const float lpHz = get (lowProt) > 0.5f ? get (lowProtFreq) : v.forceLowHz;
    if (std::abs (lpHz - lastLowFreq) > 0.5f)
    {
        lastLowFreq = lpHz;
        for (int c = 0; c < 2; ++c)
        {
            lpA[c].lowpass (sr, lpHz, 0.7071); lpB[c] = lpA[c]; lpB[c].reset();
            hpA[c].highpass (sr, lpHz, 0.7071); hpB[c] = hpA[c]; hpB[c].reset();
        }
    }
    for (int i = 0; i < len; ++i)
    {
        float mono = 0;
        if (lpOn)
            for (int c = 0; c < 2; ++c)
            {
                const float s = x[c][i];
                mono += 0.5f * lpB[c].process (lpA[c].process (s));
                x[c][i] = hpB[c].process (hpA[c].process (s));
            }
        lowDelay.push (mono);
        low[i] = lowDelay.readInt (totalLatency);
    }

    // 4. soft EQ
    const bool eqActive = get (eqOn) > 0.5f;
    const bool eqIsPost = geti (eqPost) == 1;
    {
        const float t[8] = { get (hpFreq), get (lowGain), get (lowFreq), get (midGain), get (midFreq), get (midQ), get (highGain), get (highFreq) };
        bool changed = false;
        for (int k = 0; k < 8; ++k)
        {
            eqCur[k] = eqSet[k] < -0.5f ? t[k] : eqCur[k] + 0.3f * (t[k] - eqCur[k]);
            if (std::abs (eqCur[k] - eqSet[k]) > 1.0e-3f * std::max (1.f, std::abs (t[k]))) changed = true;
        }
        if (changed)
        {
            for (int c = 0; c < 2; ++c)
            {
                if (eqCur[0] > 21.f) eqHp[c].highpass (sr, eqCur[0], 0.707); else eqHp[c].identity();
                eqLs[c].lowShelf (sr, eqCur[2], 0.55, eqCur[1]);
                eqPk[c].peak (sr, eqCur[4], eqCur[5], eqCur[3]);
                eqHs[c].highShelf (sr, eqCur[7], 0.55, eqCur[6]);
            }
            std::copy (eqCur, eqCur + 8, eqSet);
        }
    }
    auto runEq = [&] {
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < len; ++i)
                x[c][i] = eqHs[c].process (eqPk[c].process (eqLs[c].process (eqHp[c].process (x[c][i]))));
    };
    if (eqActive && ! eqIsPost) runEq();

    // 5. colour stage: HEAT + TAPE at the oversampled rate
    {
        const bool hOn = get (heatOn) > 0.5f, tOn = get (tapeOn) > 0.5f;
        float* cd[2] = { colorDry.getWritePointer (0) + start, colorDry.getWritePointer (1) + start };
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < len; ++i) { colorDelay[c].push (x[c][i]); cd[c][i] = colorDelay[c].readInt (osLatency); }

        if (hOn || tOn)
        {
            heatFluxR.setRate (0.2f + fluxRateV * 3.f, sr / kChunk);
            const float hf = heatFluxR.next (chaos) * get (P::heatFlux) * 0.12f;
            const float hAmt = clamp01 (get (heatAmt) * age * v.heat + hf);
            const float tAmt = clamp01 (get (tapeAmt) * age * v.tape);
            const int factor = 1 << osIndex;
            auto runColour = [&] (float* const* p, int m) {
                if (hOn)
                {
                    heat.setFocus (std::max (get (heatLo), v.heatLoFloor), get (heatHi));
                    heat.process (p, m, geti (heatMode), hAmt, get (heatTone), get (heatMix), factor);
                }
                else heat.level.store (0);
                if (tOn)
                    tape.process (p, m, tAmt, geti (tapeSpeed), get (tapeBump), get (tapeWear), get (tapeDrops) * age,
                                  get (tapeDrift) * age, get (tapeMix), get (P::tapeFlux), fluxRateV, chaos);
            };
            if (osIndex == 0) runColour (x, len);
            else
            {
                juce::dsp::AudioBlock<float> blk (x, 2, (size_t) len);
                auto up = os[osIndex]->processSamplesUp (blk);
                float* up_[2] = { up.getChannelPointer (0), up.getChannelPointer (1) };
                runColour (up_, (int) up.getNumSamples());
                os[osIndex]->processSamplesDown (blk);
            }
        }
        else
        {
            heat.level.store (0);
            for (int c = 0; c < 2; ++c) juce::FloatVectorOperations::copy (x[c], cd[c], len); // keep latency constant
        }

        // transient keep: let the attack of each hit through clean
        const float tk = get (transKeep) > 0.5f ? v.transStrength : 0.f;
        for (int i = 0; i < len; ++i)
        {
            const float m = 0.5f * (std::abs (cd[0][i]) + std::abs (cd[1][i]));
            const float f = fastEnv.process (m), s = slowEnv.process (m);
            const float target = s > 1.0e-5f ? clamp01 ((f / s - 1.f) * 1.5f) : 0.f;
            transAmt += (target > transAmt ? 0.3f : 0.004f) * (target - transAmt);
            if (tk > 0.f)
            {
                const float k = transAmt * tk;
                for (int c = 0; c < 2; ++c) x[c][i] += k * (cd[c][i] - x[c][i]);
            }
        }
    }

    // 6. DRIFT (always runs: constant centre delay)
    drift.process (x, len, get (driftOn) > 0.5f, geti (driftMode), clamp01 (get (driftAmt) * age * v.drift),
                   get (driftRate), get (driftSpread), get (driftMix), get (P::driftFlux), fluxRateV, chaos);

    // 7. GLUE
    glueFluxR.setRate (0.2f + fluxRateV * 2.f, sr / kChunk);
    glue.process (x, len, get (glueOn) > 0.5f, geti (glueMode), clamp01 (get (glueAmt) * age * v.glue), get (glueAttack),
                  get (glueRelease), get (glueAuto) > 0.5f, get (glueScHpf), get (glueMix), glueFluxR.next (chaos) * get (P::glueFlux) * 2.f);

    // 8. GRAIN + SPACE
    {
        static const float divs[4] = { 1.f, 0.75f, 0.5f, 0.25f };
        const int tIdx = geti (grainTime);
        const float delayMs = tIdx >= 4 ? get (grainFree) : (float) (60000.0 / bpmCache) * divs[tIdx];
        const bool parallel = geti (routing) == 1;
        const float* gin[2] = { x[0], x[1] };
        grain.process (gin, wg, len, get (grainOn) > 0.5f, clamp01 (get (grainAmt) * age * v.grain), delayMs, get (grainSize),
                       get (grainDensity), get (grainSpray), get (grainPitch), get (grainReverse), get (grainFeedback),
                       get (grainDuck), get (grainFreeze) > 0.5f, get (grainLo), get (grainHi), get (P::grainFlux), fluxRateV, chaos);
        if (! parallel) for (int c = 0; c < 2; ++c) juce::FloatVectorOperations::add (x[c], wg[c], len);
        const float* sin_[2] = { x[0], x[1] };
        space.process (sin_, ws, len, get (spaceOn) > 0.5f, geti (spaceMode), clamp01 (get (spaceAmt) * age * v.space), get (spacePre),
                       get (spaceSize), get (spaceDecay), get (spaceDamp), get (spaceMod), get (spaceWidth), get (spaceDuck),
                       get (spaceLo), get (spaceHi), get (P::spaceFlux), fluxRateV, chaos);
        for (int c = 0; c < 2; ++c)
        {
            juce::FloatVectorOperations::add (x[c], ws[c], len);
            if (parallel) juce::FloatVectorOperations::add (x[c], wg[c], len);
        }
    }

    // 9. post EQ, low band back in, dust layer
    if (eqActive && eqIsPost) runEq();
    for (int c = 0; c < 2; ++c) juce::FloatVectorOperations::add (x[c], low, len);
    dust.process (dry, x, len, geti (dustType), get (dustLevel) * age);

    // 10. tone tilt + DC block
    const float tl = get (tilt);
    if (std::abs (tl - lastTilt) > 0.01f)
    {
        lastTilt = tl;
        for (int c = 0; c < 2; ++c) { tiltLs[c].lowShelf (sr, 700, 0.5, -tl * 0.5); tiltHs[c].highShelf (sr, 700, 0.5, tl * 0.5); }
    }
    const float dcR = 1.f - (kTwoPi * 8.f / (float) sr);
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < len; ++i)
        {
            float s = x[c][i];
            if (std::abs (tl) > 0.01f) s = tiltHs[c].process (tiltLs[c].process (s));
            const float y = s - dcX[c] + dcR * dcY[c];
            dcX[c] = s; dcY[c] = y;
            x[c][i] = y;
        }

    // 11. loudness match, output gain
    luMatch.apply (x, len, get (P::luMatch) > 0.5f);
    const float og = dbToGain (get (outGain));
    for (int c = 0; c < 2; ++c) juce::FloatVectorOperations::multiply (x[c], og, len);
}

//==============================================================================
void HadesProcessor::setParam (P p, float realValue)
{
    if (auto* rp = apvts.getParameter (paramDefs()[p].id))
        rp->setValueNotifyingHost (rp->convertTo0to1 (realValue));
}

int HadesProcessor::getNumPrograms() { return (int) factoryPresets().size(); }
const juce::String HadesProcessor::getProgramName (int index)
{
    const auto& ps = factoryPresets();
    return juce::isPositiveAndBelow (index, (int) ps.size()) ? juce::String (ps[(size_t) index].name) : juce::String();
}

void HadesProcessor::loadPreset (int index)
{
    const auto& ps = factoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) ps.size())) return;
    presetIndex = index;
    for (int i = 0; i < NUM_PARAMS; ++i)
    {
        const auto p = (P) i;
        if (p == inGain || p == outGain || p == P::luMatch || p == oversample) continue; // user settings, not sound design
        setParam (p, paramDefs()[i].def);
    }
    for (const auto& [id, val] : ps[(size_t) index].v)
        if (auto* rp = apvts.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (val));
}

void HadesProcessor::randomize()
{
    juce::Random r;
    auto rf = [&] (float a, float b) { return a + (b - a) * r.nextFloat(); };
    auto chance = [&] (float p) { return r.nextFloat() < p ? 1.f : 0.f; };
    setParam (heatOn, chance (0.85f)); setParam (heatAmt, rf (0.1f, 0.55f)); setParam (heatMode, (float) r.nextInt (7));
    setParam (heatTone, rf (-0.5f, 0.4f)); setParam (heatMix, rf (0.5f, 1.f));
    setParam (tapeOn, chance (0.85f)); setParam (tapeAmt, rf (0.2f, 0.7f)); setParam (tapeSpeed, (float) r.nextInt (3));
    setParam (tapeBump, rf (0.2f, 0.8f)); setParam (tapeWear, rf (0.f, 0.5f)); setParam (tapeDrops, chance (0.3f) * rf (0.1f, 0.4f));
    setParam (tapeDrift, rf (0.f, 0.4f));
    setParam (dustLevel, chance (0.45f) * rf (0.1f, 0.35f)); setParam (dustType, (float) r.nextInt (4));
    setParam (driftOn, chance (0.8f)); setParam (driftAmt, rf (0.05f, 0.45f)); setParam (driftMode, (float) r.nextInt (5));
    setParam (driftRate, rf (0.1f, 0.7f)); setParam (driftMix, rf (0.5f, 1.f));
    setParam (glueOn, chance (0.8f)); setParam (glueAmt, rf (0.1f, 0.5f)); setParam (glueMode, (float) (r.nextFloat() < 0.85f ? r.nextInt (3) : 3));
    setParam (glueMix, rf (0.5f, 1.f));
    setParam (grainOn, chance (0.35f)); setParam (grainAmt, rf (0.15f, 0.45f)); setParam (grainTime, (float) r.nextInt (5));
    setParam (grainSize, rf (30.f, 300.f)); setParam (grainDensity, rf (0.2f, 0.9f)); setParam (grainSpray, rf (0.f, 0.8f));
    setParam (grainPitch, r.nextFloat() < 0.6f ? 0.f : (r.nextBool() ? 12.f : -12.f)); setParam (grainReverse, rf (0.f, 0.6f));
    setParam (grainFeedback, rf (0.1f, 0.6f));
    setParam (spaceOn, chance (0.85f)); setParam (spaceAmt, rf (0.1f, 0.4f)); setParam (spaceMode, (float) r.nextInt (5));
    setParam (spaceDecay, rf (0.6f, 5.f)); setParam (spaceSize, rf (0.2f, 0.9f)); setParam (spaceDamp, rf (0.2f, 0.7f));
    setParam (spaceMod, rf (0.1f, 0.6f));
    for (auto p : { P::heatFlux, P::tapeFlux, P::driftFlux, P::grainFlux, P::spaceFlux }) setParam (p, rf (0.f, 0.6f));
}

void HadesProcessor::selectAB (int slot)
{
    if (slot == abSlot) return;
    if (auto xml = apvts.copyState().createXml()) { abStates[abSlot].reset(); copyXmlToBinary (*xml, abStates[abSlot]); }
    if (abStates[slot].getSize() == 0) abStates[slot] = abStates[abSlot];
    else if (auto xml = getXmlFromBinary (abStates[slot].getData(), (int) abStates[slot].getSize()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
    abSlot = slot;
}

void HadesProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("preset", presetIndex, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void HadesProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            presetIndex = (int) vt.getProperty ("preset", 0);
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* HadesProcessor::createEditor() { return new HadesEditor (*this); }

//==============================================================================
#include "Presets.inc"

} // namespace hd

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new hd::HadesProcessor(); }
