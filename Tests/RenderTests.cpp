// Offline render tests: drive KickProcessor the way a host would and check the audio.

#include "PluginProcessor.h"
#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
    constexpr double kSampleRate = 48000.0;
    constexpr int    kBlock      = 512;

    int failures = 0;

    void check (bool ok, const char* what)
    {
        std::printf ("%s  %s\n", ok ? "PASS" : "FAIL", what);
        if (! ok)
            ++failures;
    }

    void setParam (KickProcessor& p, const char* id, float value)
    {
        auto* param = p.apvts.getParameter (id);
        param->setValueNotifyingHost (param->convertTo0to1 (value));
    }

    struct Hit { int sample; float velocity; };

    // Renders numSamples of left-channel output, firing note-ons at the given positions.
    std::vector<float> render (KickProcessor& p, int numSamples, const std::vector<Hit>& hits)
    {
        p.prepareToPlay (kSampleRate, kBlock);

        std::vector<float> out;
        juce::AudioBuffer<float> buffer (2, kBlock);

        for (int start = 0; start < numSamples; start += kBlock)
        {
            const int n = std::min (kBlock, numSamples - start);
            buffer.setSize (2, n, false, false, true);

            juce::MidiBuffer midi;
            for (auto& h : hits)
                if (h.sample >= start && h.sample < start + n)
                    midi.addEvent (juce::MidiMessage::noteOn (1, 36, h.velocity), h.sample - start);

            p.processBlock (buffer, midi);

            for (int i = 0; i < n; ++i)
            {
                out.push_back (buffer.getSample (0, i));
                if (std::abs (buffer.getSample (1, i) - buffer.getSample (0, i)) > 0.0f)
                    out.back() = NAN; // channels must match
            }
        }

        p.releaseResources();
        return out;
    }

    float peak (const std::vector<float>& x, size_t from = 0, size_t to = SIZE_MAX)
    {
        float m = 0.0f;
        for (size_t i = from; i < std::min (to, x.size()); ++i)
            m = std::max (m, std::abs (x[i]));
        return m;
    }

    bool allFinite (const std::vector<float>& x)
    {
        for (auto v : x)
            if (! std::isfinite (v))
                return false;
        return true;
    }
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    {
        KickProcessor p;
        auto x = render (p, 48000, {});
        check (peak (x) <= 0.0f, "no MIDI -> silence");
    }

    {
        KickProcessor p;
        const int hitAt = 1000;
        auto x = render (p, (int) kSampleRate * 5, { { hitAt, 1.0f } });

        check (allFinite (x), "output is finite and both channels match");
        check (peak (x, 0, hitAt) <= 0.0f, "silent before the note-on");
        check (peak (x, hitAt, hitAt + 2400) > 0.3f, "note-on produces a loud hit");
        check (peak (x) <= 1.0f, "peak stays at or below 0 dBFS at default settings");
        check (peak (x, x.size() - 4800) < 1.0e-5f, "hit decays to silence");
    }

    {
        KickProcessor p;
        setParam (p, "drive", 0.0f);
        auto loud  = render (p, 24000, { { 0, 1.0f } });
        auto quiet = render (p, 24000, { { 0, 0.5f } });
        const float ratio = peak (quiet) / peak (loud);
        check (ratio > 0.45f && ratio < 0.55f, "velocity scales the output");
    }

    {
        // Retrigger while the previous hit is still loud: no step in the waveform.
        KickProcessor p;
        setParam (p, "click", 0.0f);
        setParam (p, "drive", 0.0f);

        auto single = render (p, 4800, { { 0, 1.0f } });
        int at = 240;
        while (at < 2400 && std::abs (single[(size_t) at]) < 0.4f)
            ++at;

        auto x = render (p, 9600, { { 0, 1.0f }, { at, 1.0f } });
        float maxStep = 0.0f;
        for (int i = at - 4; i < at + 96; ++i)
            maxStep = std::max (maxStep, std::abs (x[(size_t) i] - x[(size_t) i - 1]));

        std::printf ("      retrigger at %.1f ms over %.2f, max step %.3f\n",
                     at * 1000.0 / kSampleRate, single[(size_t) at], maxStep);
        check (maxStep < 0.1f, "retrigger is click-free");
    }

    {
        // Extreme settings must stay finite.
        KickProcessor p;
        setParam (p, "startHz", 800.0f);
        setParam (p, "endHz", 120.0f);
        setParam (p, "pitchDecay", 3.0f);
        setParam (p, "ampDecay", 2000.0f);
        setParam (p, "click", 1.0f);
        setParam (p, "drive", 1.0f);
        setParam (p, "level", 0.0f);

        std::vector<Hit> hits;
        for (int s = 0; s < 96000; s += 37)
            hits.push_back ({ s, 1.0f });

        auto x = render (p, 96000, hits);
        check (allFinite (x) && peak (x) <= 1.0f, "extreme settings and rapid retriggers stay bounded");
    }

    {
        KickProcessor a;
        setParam (a, "startHz", 400.0f);
        setParam (a, "ampDecay", 900.0f);

        juce::MemoryBlock state;
        a.getStateInformation (state);

        KickProcessor b;
        b.setStateInformation (state.getData(), (int) state.getSize());

        const float s = b.apvts.getRawParameterValue ("startHz")->load();
        const float d = b.apvts.getRawParameterValue ("ampDecay")->load();
        check (std::abs (s - 400.0f) < 0.5f && std::abs (d - 900.0f) < 0.5f, "state save/restore round-trips");
        check (std::abs (b.getTailLengthSeconds() - 9.3 * 0.9) < 0.01, "tail length follows amp decay");
    }

    {
        KickProcessor p;
        auto text = [&p] (const char* id) { auto* q = p.apvts.getParameter (id); return q->getText (q->getValue(), 32); };
        check (text ("startHz") == "220 Hz" && text ("click") == "25%" && text ("level") == "-3.0 dB",
               "parameter values read as whole Hz, percent and dB");
    }

    {
        KickProcessor p;
        auto value = [&p] (const char* id) { return p.apvts.getRawParameterValue (id)->load(); };
        const float before = value ("startHz");
        bool inRange = true, changed = false;

        for (int i = 0; i < 50; ++i)
        {
            p.randomize();
            inRange = inRange && value ("startHz") >= 109.0f && value ("startHz") <= 521.0f
                              && value ("endHz") >= 31.0f && value ("endHz") <= 73.0f
                              && value ("ampDecay") >= 149.0f && value ("ampDecay") <= 1101.0f
                              && value ("drive") <= 0.86f;
            changed = changed || std::abs (value ("startHz") - before) > 1.0f;
        }

        check (inRange && changed, "randomize lands inside kick-friendly ranges");
        check (std::abs (value ("level") + 3.0f) < 0.01f, "randomize leaves level alone");

        auto x = render (p, 4800, {});
        check (peak (x) > 0.1f, "randomize plays a hit");
    }

    std::printf (failures == 0 ? "\nall tests passed\n" : "\n%d test(s) failed\n", failures);
    return failures == 0 ? 0 : 1;
}
