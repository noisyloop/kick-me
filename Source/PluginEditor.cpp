#include "PluginEditor.h"

namespace
{
    struct KnobSpec { const char* id; const char* name; const char* suffix; };

    const KnobSpec specs[] = {
        { "startHz",    "START",   " Hz" },
        { "endHz",      "END",     " Hz" },
        { "pitchDecay", "P.DECAY", " ms" },
        { "ampDecay",   "A.DECAY", " ms" },
        { "click",      "CLICK",   ""    },
        { "drive",      "DRIVE",   ""    },
        { "level",      "LEVEL",   " dB" },
    };

    const juce::Colour bg      { 0xff0b0b12 };
    const juce::Colour accent  { 0xff7df9c8 };
    const juce::Colour dim     { 0xff8a8fa3 };
}

KickEditor::KickEditor (KickProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = knobs[i];

        k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 74, 18);
        k.slider.setTextValueSuffix (specs[i].suffix);
        k.slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
        k.slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff23253a));
        k.slider.setColour (juce::Slider::thumbColourId, accent);
        k.slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        k.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (k.slider);

        k.label.setText (specs[i].name, juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        k.label.setColour (juce::Label::textColourId, dim);
        k.label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        addAndMakeVisible (k.label);

        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, specs[i].id, k.slider);
    }

    triggerButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1c2e));
    triggerButton.setColour (juce::TextButton::textColourOffId, accent);
    triggerButton.onClick = [this] { processor.uiTrigger = true; };
    addAndMakeVisible (triggerButton);

    setSize (600, 280);
}

void KickEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    g.setColour (accent);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("INFINATI KICK", 20, 12, 300, 30, juce::Justification::centredLeft);

    g.setColour (dim);
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("play any MIDI note", getWidth() - 220, 12, 200, 30, juce::Justification::centredRight);
}

void KickEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (36);

    auto bottom = area.removeFromBottom (40);
    triggerButton.setBounds (bottom.withSizeKeepingCentre (140, 32));

    const int w = area.getWidth() / (int) knobs.size();
    for (auto& k : knobs)
    {
        auto col = area.removeFromLeft (w);
        k.label.setBounds (col.removeFromTop (18));
        k.slider.setBounds (col);
    }
}
