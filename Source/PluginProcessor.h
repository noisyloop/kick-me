#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

class KickProcessor : public juce::AudioProcessor
{
public:
    KickProcessor();
    ~KickProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Set by the UI's TRIGGER button so you can audition in the standalone app.
    std::atomic<bool> uiTrigger { false };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void trigger (float velocity);

    double sampleRate = 44100.0;

    // Voice state (one-shot, monophonic: a new hit restarts the kick)
    bool   active = false;
    double phase  = 0.0;
    double timeSec = 0.0;
    float  velocity = 1.0f;
    float  clickState = 0.0f;
    juce::Random rng;

    // Declick: when a new hit cuts off the previous one, crossfade from the
    // old output into the new hit over ~1.5 ms instead of jumping.
    float  lastOut = 0.0f;
    float  tail = 0.0f;
    float  tailGain = 0.0f;
    float  tailCoeff = 0.0f;

    juce::SmoothedValue<float> levelSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickProcessor)
};
