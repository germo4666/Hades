#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

namespace hd
{
class HadesLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HadesLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
    juce::Font getPopupMenuFont() override;
};

class Canvas;

class HadesEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit HadesEditor (HadesProcessor&);
    ~HadesEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    HadesLookAndFeel lnf;
    std::unique_ptr<Canvas> canvas;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HadesEditor)
};
} // namespace hd
