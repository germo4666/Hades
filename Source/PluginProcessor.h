#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"
#include "DSP.h"

namespace hd
{
struct Preset { const char* cat; const char* name; std::vector<std::pair<const char*, float>> v; };
const std::vector<Preset>& factoryPresets();

class HadesProcessor : public juce::AudioProcessor
{
public:
    HadesProcessor();
    ~HadesProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "HADES"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return presetIndex; }
    void setCurrentProgram (int index) override { loadPreset (index); }
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // --- used by the editor and the test ---
    juce::AudioProcessorValueTreeState apvts;
    float get (P p) const { return raw[p]->load(); }
    int  geti (P p) const { return (int) std::round (raw[p]->load()); }
    void setParam (P p, float realValue);
    void loadPreset (int index);
    int  getPresetIndex() const { return presetIndex; }
    void randomize();
    void selectAB (int slot);
    int  getABSlot() const { return abSlot; }

    // visual feedback
    std::atomic<float> inLevel { 0 }, outLevel { 0 };
    std::array<float, 4096> scope {};
    std::atomic<int> scopeWrite { 0 };
    float getLuDelta() const { return luMatch.deltaDb.load(); }
    float getGlueGr() const  { return glue.grDb.load(); }
    float getHeatLevel() const { return heat.level.load(); }
    float getDriftVis() const { return drift.vis.load(); }
    int   getGrainActive() const { return grain.active.load(); }
    float getSpaceLevel() const { return space.level.load(); }

private:
    void processChunk (int start, int len);
    void updateLatency();
    void resetEverything();
    double currentBpm() const;

    std::array<std::atomic<float>*, NUM_PARAMS> raw {};
    double sr = 48000.0;
    int maxBlock = 512;
    static constexpr int kChunk = 64;

    juce::AudioBuffer<float> io, dryIn, work, colorDry, lowBand, wetG, wetS, tmp;

    // oversampling (index 0 = none)
    std::unique_ptr<juce::dsp::Oversampling<float>> os[3];
    int osIndex = -1, osLatency = 0, totalLatency = 0;

    // EQ
    Biquad eqHp[2], eqLs[2], eqPk[2], eqHs[2];
    float eqCur[8] {}, eqSet[8] { -1, -1, -1, -1, -1, -1, -1, -1 };
    // tilt
    Biquad tiltLs[2], tiltHs[2];
    float lastTilt = 999.f;
    // low protect (LR4)
    Biquad lpA[2], lpB[2], hpA[2], hpB[2];
    float lastLowFreq = -1;
    DelayLine lowDelay, colorDelay[2];
    // transient keep
    EnvFollower fastEnv, slowEnv;
    float transAmt = 0;
    // dc
    float dcX[2] {}, dcY[2] {};

    Heat heat; Tape tape; Drift drift; Glue glue; Grain grain; Space space; Dust dust;
    LoudnessMatch luMatch;
    SmoothRandom heatFluxR, glueFluxR;
    double bpmCache = 120.0;

    int presetIndex = 0;
    int abSlot = 0;
    juce::MemoryBlock abStates[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HadesProcessor)
};
} // namespace hd
