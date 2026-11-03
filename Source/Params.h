// HADES by GERMO - parameter table.
// X(id, display name, type, min, max, default, skewCentre (0 = linear), step (0 = continuous), unit, choices)
// type: F = float, B = bool, C = choice (min/max ignored, default = index, choices separated by '|')
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#define HADES_PARAMS(X) \
    /* ---- global ---- */ \
    X (age,          "Age",               F, 0.f,   1.f,    1.f,    0.f,   0.f, "%",  "") \
    X (inGain,       "Input",             F, -18.f, 18.f,   0.f,    0.f,   0.f, "dB", "") \
    X (outGain,      "Output",            F, -18.f, 18.f,   0.f,    0.f,   0.f, "dB", "") \
    X (source,       "Source",            C, 0.f,   0.f,    2.f,    0.f,   0.f, "",   "Drums|Bass|Keys|Gtr|Vox|Bus") \
    X (lowProt,      "Low Protect",       B, 0.f,   1.f,    0.f,    0.f,   0.f, "",   "") \
    X (lowProtFreq,  "Low Protect Freq",  F, 30.f,  300.f,  90.f,   100.f, 0.f, "Hz", "") \
    X (transKeep,    "Transient Keep",    B, 0.f,   1.f,    0.f,    0.f,   0.f, "",   "") \
    X (routing,      "FX Routing",        C, 0.f,   0.f,    0.f,    0.f,   0.f, "",   "Series|Parallel") \
    X (tilt,         "Tone Tilt",         F, -6.f,  6.f,    0.f,    0.f,   0.f, "dB", "") \
    X (luMatch,      "Loudness Match",    B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (oversample,   "Oversampling",      C, 0.f,   0.f,    1.f,    0.f,   0.f, "",   "1x|2x|4x") \
    X (fluxRate,     "Flux Rate",         F, 0.f,   1.f,    0.35f,  0.f,   0.f, "%",  "") \
    X (fluxChaos,    "Flux Chaos",        F, 0.f,   1.f,    0.2f,   0.f,   0.f, "%",  "") \
    /* ---- soft EQ ---- */ \
    X (eqOn,         "EQ On",             B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (eqPost,       "EQ Position",       C, 0.f,   0.f,    0.f,    0.f,   0.f, "",   "Pre|Post") \
    X (hpFreq,       "EQ High-Pass",      F, 20.f,  600.f,  20.f,   80.f,  0.f, "Hz", "") \
    X (lowGain,      "EQ Low Gain",       F, -12.f, 12.f,   0.f,    0.f,   0.f, "dB", "") \
    X (lowFreq,      "EQ Low Freq",       F, 30.f,  500.f,  110.f,  120.f, 0.f, "Hz", "") \
    X (midGain,      "EQ Mid Gain",       F, -12.f, 12.f,   0.f,    0.f,   0.f, "dB", "") \
    X (midFreq,      "EQ Mid Freq",       F, 150.f, 8000.f, 800.f,  1000.f,0.f, "Hz", "") \
    X (midQ,         "EQ Mid Q",          F, 0.3f,  3.f,    0.7f,   1.f,   0.f, "",   "") \
    X (highGain,     "EQ High Gain",      F, -12.f, 12.f,   0.f,    0.f,   0.f, "dB", "") \
    X (highFreq,     "EQ High Freq",      F, 1500.f,18000.f,8000.f, 6000.f,0.f, "Hz", "") \
    /* ---- HEAT ---- */ \
    X (heatOn,       "Heat On",           B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (heatAmt,      "Heat Amount",       F, 0.f,   1.f,    0.3f,   0.f,   0.f, "%",  "") \
    X (heatMode,     "Heat Mode",         C, 0.f,   0.f,    0.f,    0.f,   0.f, "",   "Tape Sat|Tube|Transformer|Fold|Crush|Rectify|Fuzz") \
    X (heatTone,     "Heat Tone",         F, -1.f,  1.f,    0.f,    0.f,   0.f, "",   "") \
    X (heatLo,       "Heat Focus Low",    F, 20.f,  2000.f, 20.f,   200.f, 0.f, "Hz", "") \
    X (heatHi,       "Heat Focus High",   F, 1000.f,20000.f,20000.f,5000.f,0.f, "Hz", "") \
    X (heatMix,      "Heat Mix",          F, 0.f,   1.f,    1.f,    0.f,   0.f, "%",  "") \
    X (heatFlux,     "Heat Flux",         F, 0.f,   1.f,    0.2f,   0.f,   0.f, "%",  "") \
    /* ---- TAPE ---- */ \
    X (tapeOn,       "Tape On",           B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (tapeAmt,      "Tape Amount",       F, 0.f,   1.f,    0.4f,   0.f,   0.f, "%",  "") \
    X (tapeSpeed,    "Tape Speed",        C, 0.f,   0.f,    1.f,    0.f,   0.f, "",   "7.5 ips|15 ips|30 ips") \
    X (tapeBump,     "Head Bump",         F, 0.f,   1.f,    0.5f,   0.f,   0.f, "%",  "") \
    X (tapeWear,     "Tape Wear",         F, 0.f,   1.f,    0.2f,   0.f,   0.f, "%",  "") \
    X (tapeDrops,    "Dropouts",          F, 0.f,   1.f,    0.f,    0.f,   0.f, "%",  "") \
    X (tapeDrift,    "Volume Drift",      F, 0.f,   1.f,    0.15f,  0.f,   0.f, "%",  "") \
    X (tapeMix,      "Tape Mix",          F, 0.f,   1.f,    1.f,    0.f,   0.f, "%",  "") \
    X (tapeFlux,     "Tape Flux",         F, 0.f,   1.f,    0.3f,   0.f,   0.f, "%",  "") \
    X (dustLevel,    "Dust Level",        F, 0.f,   1.f,    0.f,    0.f,   0.f, "%",  "") \
    X (dustType,     "Dust Type",         C, 0.f,   0.f,    0.f,    0.f,   0.f, "",   "Hiss|Vinyl|Room|Hum") \
    /* ---- DRIFT ---- */ \
    X (driftOn,      "Drift On",          B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (driftAmt,     "Drift Amount",      F, 0.f,   1.f,    0.2f,   0.f,   0.f, "%",  "") \
    X (driftMode,    "Drift Mode",        C, 0.f,   0.f,    0.f,    0.f,   0.f, "",   "Wow/Flutter|Juno|Dimension|Ensemble|Vibrato") \
    X (driftRate,    "Drift Rate",        F, 0.f,   1.f,    0.4f,   0.f,   0.f, "%",  "") \
    X (driftSpread,  "Drift Spread",      F, 0.f,   1.f,    0.5f,   0.f,   0.f, "%",  "") \
    X (driftMix,     "Drift Mix",         F, 0.f,   1.f,    1.f,    0.f,   0.f, "%",  "") \
    X (driftFlux,    "Drift Flux",        F, 0.f,   1.f,    0.4f,   0.f,   0.f, "%",  "") \
    /* ---- GLUE ---- */ \
    X (glueOn,       "Glue On",           B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (glueAmt,      "Glue Amount",       F, 0.f,   1.f,    0.3f,   0.f,   0.f, "%",  "") \
    X (glueMode,     "Glue Mode",         C, 0.f,   0.f,    0.f,    0.f,   0.f, "",   "VCA|Opto|FET|Pump") \
    X (glueAttack,   "Glue Attack",       F, 0.1f,  50.f,   10.f,   5.f,   0.f, "ms", "") \
    X (glueRelease,  "Glue Release",      F, 20.f,  1200.f, 200.f,  200.f, 0.f, "ms", "") \
    X (glueAuto,     "Glue Auto Release", B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (glueScHpf,    "Glue SC HPF",       F, 20.f,  400.f,  90.f,   90.f,  0.f, "Hz", "") \
    X (glueMix,      "Glue Mix",          F, 0.f,   1.f,    1.f,    0.f,   0.f, "%",  "") \
    X (glueFlux,     "Glue Flux",         F, 0.f,   1.f,    0.f,    0.f,   0.f, "%",  "") \
    /* ---- GRAIN ---- */ \
    X (grainOn,      "Grain On",          B, 0.f,   1.f,    0.f,    0.f,   0.f, "",   "") \
    X (grainAmt,     "Grain Amount",      F, 0.f,   1.f,    0.25f,  0.f,   0.f, "%",  "") \
    X (grainTime,    "Grain Time",        C, 0.f,   0.f,    1.f,    0.f,   0.f, "",   "1/4|1/8 D|1/8|1/16|Free") \
    X (grainFree,    "Grain Free Time",   F, 20.f,  2000.f, 300.f,  300.f, 0.f, "ms", "") \
    X (grainSize,    "Grain Size",        F, 10.f,  500.f,  80.f,   100.f, 0.f, "ms", "") \
    X (grainDensity, "Grain Density",     F, 0.f,   1.f,    0.5f,   0.f,   0.f, "%",  "") \
    X (grainSpray,   "Grain Spray",       F, 0.f,   1.f,    0.3f,   0.f,   0.f, "%",  "") \
    X (grainPitch,   "Grain Pitch",       F, -12.f, 12.f,   0.f,    0.f,   1.f, "st", "") \
    X (grainReverse, "Grain Reverse",     F, 0.f,   1.f,    0.1f,   0.f,   0.f, "%",  "") \
    X (grainFeedback,"Grain Feedback",    F, 0.f,   0.95f,  0.3f,   0.f,   0.f, "%",  "") \
    X (grainDuck,    "Grain Duck",        F, 0.f,   1.f,    0.5f,   0.f,   0.f, "%",  "") \
    X (grainFreeze,  "Grain Freeze",      B, 0.f,   1.f,    0.f,    0.f,   0.f, "",   "") \
    X (grainLo,      "Grain Focus Low",   F, 20.f,  2000.f, 150.f,  200.f, 0.f, "Hz", "") \
    X (grainHi,      "Grain Focus High",  F, 1000.f,20000.f,9000.f, 5000.f,0.f, "Hz", "") \
    X (grainFlux,    "Grain Flux",        F, 0.f,   1.f,    0.4f,   0.f,   0.f, "%",  "") \
    /* ---- SPACE ---- */ \
    X (spaceOn,      "Space On",          B, 0.f,   1.f,    1.f,    0.f,   0.f, "",   "") \
    X (spaceAmt,     "Space Amount",      F, 0.f,   1.f,    0.2f,   0.f,   0.f, "%",  "") \
    X (spaceMode,    "Space Mode",        C, 0.f,   0.f,    0.f,    0.f,   0.f, "",   "Room|Plate|Spring|Hall|Haze") \
    X (spacePre,     "Space Pre-Delay",   F, 0.f,   150.f,  10.f,   30.f,  0.f, "ms", "") \
    X (spaceSize,    "Space Size",        F, 0.f,   1.f,    0.45f,  0.f,   0.f, "%",  "") \
    X (spaceDecay,   "Space Decay",       F, 0.2f,  20.f,   1.5f,   2.5f,  0.f, "s",  "") \
    X (spaceDamp,    "Space Damping",     F, 0.f,   1.f,    0.5f,   0.f,   0.f, "%",  "") \
    X (spaceMod,     "Space Mod",         F, 0.f,   1.f,    0.3f,   0.f,   0.f, "%",  "") \
    X (spaceWidth,   "Space Width",       F, 0.f,   1.f,    0.8f,   0.f,   0.f, "%",  "") \
    X (spaceDuck,    "Space Duck",        F, 0.f,   1.f,    0.2f,   0.f,   0.f, "%",  "") \
    X (spaceLo,      "Space Focus Low",   F, 20.f,  2000.f, 200.f,  200.f, 0.f, "Hz", "") \
    X (spaceHi,      "Space Focus High",  F, 1000.f,20000.f,8000.f, 5000.f,0.f, "Hz", "") \
    X (spaceFlux,    "Space Flux",        F, 0.f,   1.f,    0.15f,  0.f,   0.f, "%",  "")

namespace hd
{
enum PType { F, B, C };

enum P
{
#define HD_ENUM(id, ...) id,
    HADES_PARAMS (HD_ENUM)
#undef HD_ENUM
    NUM_PARAMS
};

struct PDef
{
    const char* id; const char* name; PType type;
    float min, max, def, centre, step; const char* unit; const char* choices;
};

inline const PDef* paramDefs()
{
    static const PDef d[] = {
#define HD_DEF(id, name, type, mn, mx, df, ce, st, un, ch) { #id, name, type, mn, mx, df, ce, st, un, ch },
        HADES_PARAMS (HD_DEF)
#undef HD_DEF
    };
    return d;
}

inline juce::String formatValue (const PDef& d, float v)
{
    const juce::String u (d.unit);
    if (u == "dB")  return (v > 0.05f ? "+" : "") + juce::String (v, 1) + " dB";
    if (u == "Hz")  return v >= 1000.f ? juce::String (v / 1000.f, v >= 10000.f ? 1 : 2) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz";
    if (u == "ms")  return v < 10.f ? juce::String (v, 1) + " ms" : juce::String (juce::roundToInt (v)) + " ms";
    if (u == "s")   return juce::String (v, v < 10.f ? 1 : 0) + " s";
    if (u == "%")   return juce::String (juce::roundToInt (v * 100.f)) + "%";
    if (u == "st")  return (v > 0.f ? "+" : "") + juce::String (juce::roundToInt (v)) + " st";
    return juce::String (v, 2);
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < NUM_PARAMS; ++i)
    {
        const auto& d = paramDefs()[i];
        const juce::ParameterID pid { d.id, 1 };
        if (d.type == F)
        {
            juce::NormalisableRange<float> r (d.min, d.max, d.step);
            if (d.centre > 0.f) r.setSkewForCentre (d.centre);
            const PDef* dp = &d;
            layout.add (std::make_unique<juce::AudioParameterFloat> (pid, d.name, r, d.def,
                juce::AudioParameterFloatAttributes().withStringFromValueFunction ([dp] (float v, int) { return formatValue (*dp, v); })));
        }
        else if (d.type == B)
            layout.add (std::make_unique<juce::AudioParameterBool> (pid, d.name, d.def > 0.5f));
        else
            layout.add (std::make_unique<juce::AudioParameterChoice> (pid, d.name, juce::StringArray::fromTokens (d.choices, "|", ""), (int) d.def));
    }
    return layout;
}
} // namespace hd
