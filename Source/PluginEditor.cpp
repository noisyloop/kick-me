#include "PluginEditor.h"

namespace
{
    using juce::Colour;

    const Colour cyan    { 0xff00f0ff };
    const Colour violet  { 0xff7b2fff };
    const Colour magenta { 0xffff00e5 };
    const Colour hotPink { 0xffff2975 };
    const Colour orange  { 0xffff8c00 };
    const Colour yellow  { 0xffffd319 };
    const Colour lime    { 0xff3dff8b };

    const Colour skyTop    { 0xff0d0221 };
    const Colour skyBottom { 0xff3b0a5e };

    struct KnobSpec { const char* id; const char* name; Colour from, to; };

    // Each knob hands the rainbow on to the next one
    const KnobSpec specs[] = {
        { "startHz",    "START",   cyan,    violet  },
        { "endHz",      "END",     violet,  magenta },
        { "pitchDecay", "P.DECAY", magenta, hotPink },
        { "ampDecay",   "A.DECAY", hotPink, orange  },
        { "click",      "CLICK",   orange,  yellow  },
        { "drive",      "DRIVE",   yellow,  lime    },
        { "level",      "LEVEL",   lime,    cyan    },
    };
}

//==============================================================================
KickLookAndFeel::KickLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha (0.35f));
    setColour (juce::Slider::textBoxHighlightColourId, magenta.withAlpha (0.5f));
    setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

void KickLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (6.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float lineW = radius * 0.16f;
    const float arcR = radius - lineW;
    const float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const auto from = slider.findColour (juce::Slider::rotarySliderFillColourId);
    const auto to   = slider.findColour (juce::Slider::thumbColourId);

    const juce::PathStrokeType stroke (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.strokePath (track, stroke);

    if (sliderPos > 0.0f)
    {
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, rotaryStartAngle, toAngle, true);

        g.setGradientFill (juce::ColourGradient (from, centre.x - radius, centre.y,
                                                 to,   centre.x + radius, centre.y, false));

        // Two soft passes for the glow, then the arc itself
        g.setOpacity (0.15f);
        g.strokePath (arc, juce::PathStrokeType (lineW * 2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setOpacity (0.3f);
        g.strokePath (arc, juce::PathStrokeType (lineW * 1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setOpacity (1.0f);
        g.strokePath (arc, stroke);
    }

    // Knob body: glossy candy ball
    const float bodyR = arcR - lineW * 1.2f;
    const auto body = juce::Rectangle<float> (bodyR * 2.0f, bodyR * 2.0f).withCentre (centre);

    juce::Path bodyPath;
    bodyPath.addEllipse (body);
    juce::DropShadow (juce::Colours::black.withAlpha (0.7f), (int) (bodyR * 0.5f), { 0, (int) (bodyR * 0.2f) })
        .drawForPath (g, bodyPath);

    g.setGradientFill (juce::ColourGradient (from.interpolatedWith (juce::Colours::white, 0.3f),
                                             centre.x - bodyR * 0.4f, centre.y - bodyR * 0.5f,
                                             to.darker (1.2f), centre.x + bodyR * 0.7f, centre.y + bodyR, true));
    g.fillEllipse (body);

    const auto gloss = body.reduced (bodyR * 0.18f, 0.0f).withHeight (bodyR * 0.95f).translated (0.0f, bodyR * 0.08f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.45f), centre.x, gloss.getY(),
                                             juce::Colours::white.withAlpha (0.0f), centre.x, gloss.getBottom(), false));
    g.fillEllipse (gloss);

    g.setColour (juce::Colours::white.withAlpha (0.4f));
    g.drawEllipse (body, 1.2f);

    juce::Path pointer;
    pointer.addRoundedRectangle (-lineW * 0.3f, -bodyR * 0.9f, lineW * 0.6f, bodyR * 0.5f, lineW * 0.3f);
    g.setColour (juce::Colours::white);
    g.fillPath (pointer, juce::AffineTransform::rotation (toAngle).translated (centre.x, centre.y));

    // Glowing bead at the end of the arc
    const auto end = centre.getPointOnCircumference (arcR, toAngle);
    g.setColour (to.withAlpha (0.55f));
    g.fillEllipse (juce::Rectangle<float> (lineW * 2.4f, lineW * 2.4f).withCentre (end));
    g.setColour (juce::Colours::white);
    g.fillEllipse (juce::Rectangle<float> (lineW * 1.1f, lineW * 1.1f).withCentre (end));
}

void KickLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                            bool isMouseOverButton, bool isButtonDown)
{
    const auto b = button.getLocalBounds().toFloat().reduced (8.0f);
    const float corner = b.getHeight() * 0.5f;

    auto from = button.findColour (juce::TextButton::buttonColourId);
    auto to   = button.findColour (juce::TextButton::buttonOnColourId);

    if (isButtonDown)
    {
        from = from.darker (0.3f);
        to   = to.darker (0.3f);
    }
    else if (isMouseOverButton)
    {
        from = from.brighter (0.25f);
        to   = to.brighter (0.25f);
    }

    juce::Path pill;
    pill.addRoundedRectangle (b, corner);
    juce::DropShadow (to.withAlpha (isMouseOverButton ? 0.95f : 0.65f), 16, {}).drawForPath (g, pill);

    g.setGradientFill (juce::ColourGradient (from, b.getX(), b.getY(), to, b.getRight(), b.getBottom(), false));
    g.fillPath (pill);

    const auto shine = b.reduced (corner * 0.5f, 2.0f).withHeight (b.getHeight() * 0.45f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.5f), 0.0f, shine.getY(),
                                             juce::Colours::white.withAlpha (0.0f), 0.0f, shine.getBottom(), false));
    g.fillRoundedRectangle (shine, shine.getHeight() * 0.5f);

    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.strokePath (pill, juce::PathStrokeType (1.3f));
}

juce::Font KickLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (17.0f, juce::Font::bold));
}

//==============================================================================
KickEditor::KickEditor (KickProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = knobs[i];

        k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 78, 20);
        k.slider.setColour (juce::Slider::rotarySliderFillColourId, specs[i].from);
        k.slider.setColour (juce::Slider::thumbColourId, specs[i].to);
        addAndMakeVisible (k.slider);

        k.label.setText (specs[i].name, juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        k.label.setColour (juce::Label::textColourId, specs[i].from.interpolatedWith (juce::Colours::white, 0.4f));
        k.label.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        addAndMakeVisible (k.label);

        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, specs[i].id, k.slider);
    }

    triggerButton.setColour (juce::TextButton::buttonColourId, hotPink);
    triggerButton.setColour (juce::TextButton::buttonOnColourId, orange);
    triggerButton.onClick = [this] { processor.uiTrigger = true; };
    addAndMakeVisible (triggerButton);

    randomButton.setColour (juce::TextButton::buttonColourId, cyan);
    randomButton.setColour (juce::TextButton::buttonOnColourId, violet);
    randomButton.onClick = [this] { processor.randomize(); };
    addAndMakeVisible (randomButton);

    setSize (760, 420);
    startTimerHz (30);
}

KickEditor::~KickEditor()
{
    setLookAndFeel (nullptr);
}

void KickEditor::timerCallback()
{
    float next = juce::jmax (processor.outputPeak.load(), pulse * 0.88f);
    if (next < 0.003f)
        next = 0.0f;

    if (std::abs (next - pulse) > 1.0e-4f)
    {
        pulse = next;
        repaint();
    }
}

void KickEditor::paint (juce::Graphics& g)
{
    const auto w = (float) getWidth();
    const auto h = (float) getHeight();
    const float horizon = h * 0.72f;
    const float glow = juce::jlimit (0.0f, 1.0f, pulse);

    // Sky
    const juce::ColourGradient sky (skyTop, 0.0f, 0.0f, skyBottom, 0.0f, horizon, false);
    g.setGradientFill (sky);
    g.fillRect (0.0f, 0.0f, w, horizon);

    juce::Random stars (4242);
    for (int i = 0; i < 80; ++i)
    {
        const float sx = stars.nextFloat() * w;
        const float sy = stars.nextFloat() * horizon * 0.85f;
        const float size = 0.6f + stars.nextFloat() * 1.8f;
        g.setColour (juce::Colours::white.withAlpha (0.2f + stars.nextFloat() * 0.7f));
        g.fillEllipse (sx, sy, size, size);
    }

    // The sun throbs with every kick
    const float sunR = h * 0.36f * (1.0f + 0.12f * glow);
    const juce::Point<float> sunCentre (w * 0.5f, horizon - h * 0.08f);

    g.setGradientFill (juce::ColourGradient (hotPink.withAlpha (0.35f + 0.35f * glow), sunCentre.x, sunCentre.y,
                                             hotPink.withAlpha (0.0f), sunCentre.x + sunR * 1.9f, sunCentre.y, true));
    g.fillEllipse (juce::Rectangle<float> (sunR * 3.8f, sunR * 3.8f).withCentre (sunCentre));

    juce::ColourGradient sun (yellow, sunCentre.x, sunCentre.y - sunR, violet, sunCentre.x, sunCentre.y + sunR, false);
    sun.addColour (0.5, hotPink);
    g.setGradientFill (sun);
    g.setOpacity (0.6f + 0.4f * glow);
    g.fillEllipse (juce::Rectangle<float> (sunR * 2.0f, sunR * 2.0f).withCentre (sunCentre));
    g.setOpacity (1.0f);

    // Retro stripes cut through the lower half of the sun
    g.setGradientFill (sky);
    for (int i = 0; i < 6; ++i)
    {
        const float sy = sunCentre.y + sunR * (0.1f + 0.16f * (float) i);
        const float thickness = 1.5f + 1.6f * (float) i;
        g.fillRect (sunCentre.x - sunR, sy, sunR * 2.0f, thickness);
    }

    // Ground and perspective grid
    g.setGradientFill (juce::ColourGradient (Colour (0xff1a0033), 0.0f, horizon, juce::Colours::black, 0.0f, h, false));
    g.fillRect (0.0f, horizon, w, h - horizon);

    g.setColour (magenta.withAlpha (0.4f + 0.5f * glow));
    for (int i = 1; i <= 9; ++i)
    {
        const float t = (float) i / 9.0f;
        g.drawHorizontalLine ((int) (horizon + (h - horizon) * t * t), 0.0f, w);
    }
    for (int i = -14; i <= 14; ++i)
        g.drawLine (w * 0.5f + (float) i * w * 0.012f, horizon, w * 0.5f + (float) i * w * 0.11f, h, 1.0f);

    g.setColour (cyan.withAlpha (0.8f));
    g.fillRect (0.0f, horizon - 1.0f, w, 2.0f);

    // Title: chrome-ish gradient letters with a hot pink drop
    juce::GlyphArrangement title;
    title.addLineOfText (juce::Font (juce::FontOptions (42.0f, juce::Font::bold | juce::Font::italic)),
                         "INFINATI KICK", 24.0f, 54.0f);
    juce::Path titlePath;
    title.createPath (titlePath);
    const auto tb = titlePath.getBounds();

    g.setColour (magenta.withAlpha (0.75f));
    g.fillPath (titlePath, juce::AffineTransform::translation (4.0f, 4.0f));

    juce::ColourGradient chrome (cyan, tb.getX(), tb.getY(), yellow, tb.getRight(), tb.getBottom(), false);
    chrome.addColour (0.5, juce::Colours::white);
    g.setGradientFill (chrome);
    g.fillPath (titlePath);
    g.setColour (juce::Colours::white.withAlpha (0.6f + 0.4f * glow));
    g.strokePath (titlePath, juce::PathStrokeType (1.0f));

    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.setColour (cyan);
    g.drawText ("/// MAXIMUM KICK TECHNOLOGY ///", getWidth() - 300, 20, 280, 18, juce::Justification::centredRight);
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.drawText ("play any MIDI note", getWidth() - 300, 38, 280, 18, juce::Justification::centredRight);

    // Frosted glass panel behind the knobs
    const auto pf = panel.toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.16f), 0.0f, pf.getY(),
                                             juce::Colours::white.withAlpha (0.04f), 0.0f, pf.getBottom(), false));
    g.fillRoundedRectangle (pf, 20.0f);

    juce::ColourGradient rim (cyan, pf.getX(), 0.0f, magenta, pf.getRight(), 0.0f, false);
    rim.addColour (0.5, yellow);
    g.setGradientFill (rim);
    g.setOpacity (0.55f + 0.45f * glow);
    g.drawRoundedRectangle (pf, 20.0f, 1.6f);
    g.setOpacity (1.0f);
}

void KickEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (60);

    auto buttons = area.removeFromBottom (62).withSizeKeepingCentre (420, 58);
    triggerButton.setBounds (buttons.removeFromLeft (200));
    randomButton.setBounds (buttons.removeFromRight (200));

    panel = area.reduced (0, 6);

    auto inner = panel.reduced (8, 12);
    const int w = inner.getWidth() / (int) knobs.size();
    for (auto& k : knobs)
    {
        auto col = inner.removeFromLeft (w);
        k.label.setBounds (col.removeFromTop (20));
        k.slider.setBounds (col);
    }
}
