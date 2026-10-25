#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr auto kStartHz    = "startHz";
    constexpr auto kEndHz      = "endHz";
    constexpr auto kPitchDecay = "pitchDecay";
    constexpr auto kAmpDecay   = "ampDecay";
    constexpr auto kClick      = "click";
    constexpr auto kDrive      = "drive";
    constexpr auto kLevel      = "level";
}

KickProcessor::KickProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout KickProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Pitch envelope: starts at startHz, sweeps down to endHz
    params.push_back (std::make_unique<P> (juce::ParameterID { kStartHz, 1 }, "Start Pitch",
                                           R (60.0f, 800.0f, 0.0f, 0.4f), 220.0f));
    params.push_back (std::make_unique<P> (juce::ParameterID { kEndHz, 1 }, "End Pitch",
                                           R (25.0f, 120.0f, 0.0f, 0.6f), 48.0f));
    params.push_back (std::make_unique<P> (juce::ParameterID { kPitchDecay, 1 }, "Pitch Decay",
                                           R (3.0f, 300.0f, 0.0f, 0.4f), 45.0f));
    // Amplitude decay (the "length" of the kick)
    params.push_back (std::make_unique<P> (juce::ParameterID { kAmpDecay, 1 }, "Amp Decay",
                                           R (40.0f, 2000.0f, 0.0f, 0.4f), 420.0f));
    params.push_back (std::make_unique<P> (juce::ParameterID { kClick, 1 }, "Click",
                                           R (0.0f, 1.0f), 0.25f));
    params.push_back (std::make_unique<P> (juce::ParameterID { kDrive, 1 }, "Drive",
                                           R (0.0f, 1.0f), 0.30f));
    params.push_back (std::make_unique<P> (juce::ParameterID { kLevel, 1 }, "Level",
                                           R (-24.0f, 0.0f), -3.0f));

    return { params.begin(), params.end() };
}

void KickProcessor::prepareToPlay (double newSampleRate, int)
{
    sampleRate = newSampleRate;
    active = false;
}

bool KickProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void KickProcessor::trigger (float v)
{
    active = true;
    phase = 0.0;
    timeSec = 0.0;
    velocity = juce::jlimit (0.0f, 1.0f, v);
    clickState = 0.0f;
}

void KickProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    buffer.clear();

    const float startHz    = apvts.getRawParameterValue (kStartHz)->load();
    const float endHz      = apvts.getRawParameterValue (kEndHz)->load();
    const float pitchTau   = juce::jmax (0.0005f, apvts.getRawParameterValue (kPitchDecay)->load() * 0.001f);
    const float ampTau     = juce::jmax (0.005f,  apvts.getRawParameterValue (kAmpDecay)->load() * 0.001f);
    const float clickAmt   = apvts.getRawParameterValue (kClick)->load();
    const float drive      = apvts.getRawParameterValue (kDrive)->load();
    const float level      = juce::Decibels::decibelsToGain (apvts.getRawParameterValue (kLevel)->load());

    const float driveGain = 1.0f + drive * 9.0f;
    const float driveNorm = 1.0f / std::tanh (driveGain);
    const double dt = 1.0 / sampleRate;

    if (uiTrigger.exchange (false))
        trigger (1.0f);

    auto midiIt = midi.begin();
    auto* left = buffer.getWritePointer (0);
    auto* right = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        // Fire any MIDI note-ons that land on this sample
        while (midiIt != midi.end() && (*midiIt).samplePosition <= i)
        {
            const auto msg = (*midiIt).getMessage();
            if (msg.isNoteOn())
                trigger (msg.getFloatVelocity());
            ++midiIt;
        }

        float out = 0.0f;

        if (active)
        {
            const double t = timeSec;

            // Exponential pitch sweep from startHz down to endHz
            const double freq = endHz + (startHz - endHz) * std::exp (-t / pitchTau);
            phase += freq * dt;
            if (phase >= 1.0)
                phase -= 1.0;

            const float body = (float) std::sin (juce::MathConstants<double>::twoPi * phase);
            const float env = (float) std::exp (-t / ampTau);

            // Click: a very short burst of lightly filtered noise
            const float noise = rng.nextFloat() * 2.0f - 1.0f;
            clickState += 0.35f * (noise - clickState);
            const float click = clickState * (float) std::exp (-t / 0.002) * clickAmt * 0.8f;

            float x = body * env + click;
            x = std::tanh (x * driveGain) * driveNorm;
            out = x * velocity * level;

            timeSec += dt;
            if (env < 1.0e-4f && t > 0.01)
                active = false;
        }

        left[i] = out;
        if (right != nullptr)
            right[i] = out;
    }
}

juce::AudioProcessorEditor* KickProcessor::createEditor()
{
    return new KickEditor (*this);
}

void KickProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void KickProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KickProcessor();
}
