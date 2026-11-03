// Offline test: renders every preset through HADES and checks stability, level, latency and CPU.
// Also renders the real editor to a PNG so the UI can be checked without a DAW.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <chrono>

using namespace hd;

static juce::AudioBuffer<float> makeSignal (double sr, double seconds)
{
    const int n = (int) (sr * seconds);
    juce::AudioBuffer<float> b (2, n);
    juce::Random r (1);
    double ph[3] {};
    const double freqs[3] = { 110.0, 164.8, 220.0 };
    for (int i = 0; i < n; ++i)
    {
        const double t = i / sr;
        const double beat = std::fmod (t, 0.5);
        const double kick = std::sin (2 * juce::MathConstants<double>::pi * (50 + 120 * std::exp (-beat * 30)) * beat) * std::exp (-beat * 9) * 0.6;
        const double hb = std::fmod (t + 0.25, 0.5);
        const double hat = (r.nextFloat() * 2 - 1) * std::exp (-hb * 60) * 0.15;
        double pad = 0;
        for (int k = 0; k < 3; ++k) { ph[k] += freqs[k] / sr; ph[k] -= std::floor (ph[k]); pad += (2 * ph[k] - 1) * 0.06; }
        b.setSample (0, i, (float) (kick + hat + pad));
        b.setSample (1, i, (float) (kick + hat * 0.8 + pad * 0.9));
    }
    return b;
}

static void render (HadesProcessor& p, const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& out, int bs)
{
    out.setSize (2, in.getNumSamples());
    juce::AudioBuffer<float> blk (2, bs);
    juce::MidiBuffer m;
    for (int pos = 0; pos < in.getNumSamples(); pos += bs)
    {
        const int nn = std::min (bs, in.getNumSamples() - pos);
        blk.setSize (2, nn, false, false, true);
        for (int c = 0; c < 2; ++c) blk.copyFrom (c, 0, in, c, pos, nn);
        p.processBlock (blk, m);
        for (int c = 0; c < 2; ++c) out.copyFrom (c, pos, blk, c, 0, nn);
    }
}

static double kLoud (const juce::AudioBuffer<float>& b, double sr, int from)
{
    Biquad k1[2], k2[2];
    double acc = 0;
    for (int c = 0; c < 2; ++c)
    {
        k1[c].highShelf (sr, 1681, 0.707, 4.0); k2[c].highpass (sr, 38, 0.5);
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const float y = k2[c].process (k1[c].process (b.getSample (c, i)));
            if (i >= from) acc += (double) y * y;
        }
    }
    return 10.0 * std::log10 (acc / (2.0 * (b.getNumSamples() - from)) + 1e-20);
}

static void prep (HadesProcessor& p, double sr, int bs)
{
    p.setPlayConfigDetails (2, 2, sr, bs);
    p.prepareToPlay (sr, bs);
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    int failures = 0;
    const auto fail = [&] (const juce::String& s) { std::printf ("FAIL: %s\n", s.toRawUTF8()); ++failures; };

    // ---------- 1. null test + latency ----------
    for (int osIdx : { 0, 1, 2 })
    {
        HadesProcessor p;
        for (auto id : { heatOn, tapeOn, driftOn, glueOn, grainOn, spaceOn, eqOn, P::luMatch, lowProt, transKeep }) p.setParam (id, 0);
        p.setParam (dustLevel, 0); p.setParam (oversample, (float) osIdx); p.setParam (source, 2);
        prep (p, 48000, 512);
        juce::AudioBuffer<float> in (2, 48000), out; in.clear();
        in.setSample (0, 1000, 1.f); in.setSample (1, 1000, 1.f);
        render (p, in, out, 512);
        int pk = 0; float pv = 0;
        for (int i = 0; i < out.getNumSamples(); ++i) if (std::abs (out.getSample (0, i)) > pv) { pv = std::abs (out.getSample (0, i)); pk = i; }
        const int lat = p.getLatencySamples();
        std::printf ("null os=%d: latency=%d, impulse at +%d (%.4f)\n", osIdx, lat, pk - 1000, pv);
        if (pk - 1000 != lat || std::abs (pv - 1.f) > 0.02f) fail ("impulse/latency mismatch os=" + juce::String (osIdx));
    }

    // ---------- 2. every preset at 48k, a spread at 44.1k / 96k ----------
    struct Cfg { double sr; int bs; int step; };
    for (auto cfg : { Cfg { 48000, 512, 1 }, Cfg { 44100, 256, 7 }, Cfg { 96000, 1024, 9 } })
    {
        const auto sig = makeSignal (cfg.sr, 6.0);
        const double inL = kLoud (sig, cfg.sr, (int) (cfg.sr * 3));
        double worstDelta = 0; float worstPeak = 0;
        for (int i = 0; i < (int) factoryPresets().size(); i += cfg.step)
        {
            HadesProcessor p;
            p.loadPreset (i);
            prep (p, cfg.sr, cfg.bs);
            juce::AudioBuffer<float> out;
            render (p, sig, out, cfg.bs);
            bool finite = true; float peak = 0;
            for (int c = 0; c < 2; ++c)
                for (int k = 0; k < out.getNumSamples(); ++k) { const float v = out.getSample (c, k); if (! std::isfinite (v)) finite = false; peak = std::max (peak, std::abs (v)); }
            const double outL = kLoud (out, cfg.sr, (int) (cfg.sr * 3));
            const double d = outL - inL;
            if (std::abs (d) > std::abs (worstDelta)) worstDelta = d;
            worstPeak = std::max (worstPeak, peak);
            const juce::String nm = juce::String (factoryPresets()[(size_t) i].name) + " @" + juce::String ((int) cfg.sr);
            if (! finite) fail (nm + " produced NaN/inf");
            if (peak > 4.f) fail (nm + " peak " + juce::String (peak));
            if (std::abs (d) > 2.0) fail (nm + " loudness off by " + juce::String (d, 2) + " dB with LU match on");
            if (cfg.step == 1 && (argc > 1)) std::printf ("  %-22s peak %.2f  dLU %+.2f\n", factoryPresets()[(size_t) i].name, peak, d);
        }
        std::printf ("presets @%d: worst LU delta %+.2f dB, worst peak %.2f\n", (int) cfg.sr, worstDelta, worstPeak);
    }

    // ---------- 3. LU match off: level change stays sane ----------
    {
        const auto sig = makeSignal (48000, 6.0);
        const double inL = kLoud (sig, 48000, 48000 * 3);
        double lo = 99, hi = -99;
        for (int i = 0; i < (int) factoryPresets().size(); ++i)
        {
            HadesProcessor p; p.loadPreset (i); p.setParam (P::luMatch, 0);
            prep (p, 48000, 512);
            juce::AudioBuffer<float> out; render (p, sig, out, 512);
            const double d = kLoud (out, 48000, 48000 * 3) - inL;
            lo = std::min (lo, d); hi = std::max (hi, d);
            if (argc > 2) std::printf ("  raw %-22s %+.1f dB\n", factoryPresets()[(size_t) i].name, d);
            if (d > 9.0 || d < -15.0) fail (juce::String (factoryPresets()[(size_t) i].name) + " raw level change " + juce::String (d, 1) + " dB");
        }
        std::printf ("LU match off: raw level change range %+.1f .. %+.1f dB\n", lo, hi);
    }

    // ---------- 4. CPU ----------
    for (int presetIdx : { 0, 73, 93 })
    {
        HadesProcessor p; p.loadPreset (presetIdx); p.setParam (oversample, 2); p.setParam (grainOn, 1);
        prep (p, 48000, 256);
        const auto sig = makeSignal (48000, 10.0);
        juce::AudioBuffer<float> out;
        const auto t0 = std::chrono::high_resolution_clock::now();
        render (p, sig, out, 256);
        const double secs = std::chrono::duration<double> (std::chrono::high_resolution_clock::now() - t0).count();
        std::printf ("CPU %-16s (4x OS, grain on): %.1f%% of one core\n", factoryPresets()[(size_t) presetIdx].name, secs / 10.0 * 100.0);
    }

    // ---------- 5. state round-trip + A/B ----------
    {
        HadesProcessor a; a.loadPreset (42);
        juce::MemoryBlock mb; a.getStateInformation (mb);
        HadesProcessor b; b.setStateInformation (mb.getData(), (int) mb.getSize());
        for (int i = 0; i < NUM_PARAMS; ++i)
            if (std::abs (a.get ((P) i) - b.get ((P) i)) > 1e-4f) fail (juce::String ("state mismatch ") + paramDefs()[i].id);
        if (b.getPresetIndex() != 42) fail ("preset index not restored");
        a.selectAB (1); a.setParam (heatAmt, 0.9f); a.selectAB (0);
        if (std::abs (a.get (heatAmt) - 0.9f) < 1e-3f) fail ("A/B did not restore A");
        a.selectAB (1);
        if (std::abs (a.get (heatAmt) - 0.9f) > 1e-3f) fail ("A/B did not restore B");
    }

    // ---------- 6. editor snapshot ----------
    {
        HadesProcessor p; p.loadPreset (0); prep (p, 48000, 512);
        const auto sig = makeSignal (48000, 1.0);
        juce::AudioBuffer<float> out; render (p, sig, out, 512);
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (1200, 860);

        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.f);
        juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile ("hades_editor.png");
        f.deleteFile();
        juce::FileOutputStream os (f);
        juce::PNGImageFormat().writeImageToStream (img, os);
        std::printf ("editor snapshot -> %s\n", f.getFullPathName().toRawUTF8());
    }

    std::printf (failures == 0 ? "ALL TESTS PASSED\n" : "%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}
