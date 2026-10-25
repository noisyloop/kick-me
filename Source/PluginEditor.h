#pragma once

#include "PluginProcessor.h"
#include <array>

// Neon synthwave look: gradient knobs with glowing arcs, glossy pill buttons.
class KickLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KickLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
};

class KickEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit KickEditor (KickProcessor&);
    ~KickEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KickLookAndFeel lnf;
    KickProcessor& processor;
    std::array<Knob, 7> knobs;
    juce::TextButton triggerButton { "TRIGGER" };
    juce::TextButton randomButton { "RANDOMIZE" };

    juce::Rectangle<int> panel;
    float pulse = 0.0f; // follows the output level so the scene throbs with each kick

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickEditor)
};
