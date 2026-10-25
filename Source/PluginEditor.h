#pragma once

#include "PluginProcessor.h"
#include <array>

class KickEditor : public juce::AudioProcessorEditor
{
public:
    explicit KickEditor (KickProcessor&);
    ~KickEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KickProcessor& processor;
    std::array<Knob, 7> knobs;
    juce::TextButton triggerButton { "TRIGGER" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickEditor)
};
