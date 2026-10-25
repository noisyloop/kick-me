#include "PluginEditor.h"

namespace
{
    using juce::Colour;

    const Colour amber      { 0xffffa634 };
    const Colour faceTop    { 0xff2a2a2c };
    const Colour faceBottom { 0xff18181a };
    const Colour metalLight { 0xffc9c7c1 };
    const Colour metalDark  { 0xff8f8d88 };
    const Colour ink        { 0xff1b1b1c };
    const Colour silkscreen { 0xffd8d3c7 };
    const Colour cream      { 0xffece4d0 };

    struct KnobSpec { const char* id; const char* name; };

    const KnobSpec specs[] = {
        { "startHz",    "START"   },
        { "endHz",      "END"     },
        { "pitchDecay", "P.DECAY" },
        { "ampDecay",   "A.DECAY" },
        { "click",      "CLICK"   },
        { "drive",      "DRIVE"   },
        { "level",      "LEVEL"   },
    };

    // Faint horizontal streaks, like brushed metal
    void drawBrushed (juce::Graphics& g, juce::Rectangle<float> area, float strength, int seed)
    {
        juce::Random r (seed);
        for (float y = area.getY(); y < area.getBottom(); y += 1.0f)
        {
            const bool light = r.nextBool();
            g.setColour ((light ? juce::Colours::white : juce::Colours::black).withAlpha (r.nextFloat() * strength));
            g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
        }
    }

    void drawScrew (juce::Graphics& g, juce::Point<float> c)
    {
        const auto r = juce::Rectangle<float> (11.0f, 11.0f).withCentre (c);
        g.setGradientFill (juce::ColourGradient (Colour (0xffd5d3ce), r.getX(), r.getY(),
                                                 Colour (0xff6d6b67), r.getRight(), r.getBottom(), false));
        g.fillEllipse (r);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawEllipse (r, 0.8f);
        g.setColour (Colour (0xff3a3937));
        g.drawLine (c.x - 3.5f, c.y + 2.0f, c.x + 3.5f, c.y - 2.0f, 1.4f);
    }
}

//==============================================================================
KickLookAndFeel::KickLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, amber);
    setColour (juce::Slider::textBoxOutlineColourId, Colour (0xff3a3a3c));
    setColour (juce::Slider::textBoxBackgroundColourId, Colour (0xff0c0c0d));
    setColour (juce::Slider::textBoxHighlightColourId, amber.withAlpha (0.3f));
    setColour (juce::TextButton::textColourOffId, silkscreen);
    setColour (juce::TextButton::textColourOnId, amber);
    setColour (juce::ComboBox::textColourId, amber);
    setColour (juce::ComboBox::arrowColourId, amber);
    setColour (juce::PopupMenu::backgroundColourId, Colour (0xff1c1c1e));
    setColour (juce::PopupMenu::textColourId, silkscreen);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, amber.withAlpha (0.85f));
    setColour (juce::PopupMenu::highlightedTextColourId, ink);
}

void KickLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Scale ticks printed on the faceplate
    const float tickOuter = radius;
    const float tickInner = radius * 0.86f;
    for (int i = 0; i <= 10; ++i)
    {
        const float a = rotaryStartAngle + (float) i / 10.0f * (rotaryEndAngle - rotaryStartAngle);
        const bool major = i % 5 == 0;
        g.setColour (silkscreen.withAlpha (major ? 0.8f : 0.4f));
        g.drawLine (juce::Line<float> (centre.getPointOnCircumference (major ? tickInner * 0.96f : tickInner, a),
                                       centre.getPointOnCircumference (tickOuter, a)),
                    major ? 1.6f : 1.0f);
    }

    // Knurled skirt
    const float skirtR = radius * 0.78f;
    const auto skirt = juce::Rectangle<float> (skirtR * 2.0f, skirtR * 2.0f).withCentre (centre);

    juce::Path skirtPath;
    skirtPath.addEllipse (skirt);
    juce::DropShadow (juce::Colours::black.withAlpha (0.75f), (int) (skirtR * 0.45f), { 0, (int) (skirtR * 0.18f) })
        .drawForPath (g, skirtPath);

    g.setGradientFill (juce::ColourGradient (Colour (0xff4a4a4c), centre.x, skirt.getY(),
                                             Colour (0xff141415), centre.x, skirt.getBottom(), false));
    g.fillEllipse (skirt);

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    for (int i = 0; i < 48; ++i)
    {
        const float a = angle + (float) i * juce::MathConstants<float>::twoPi / 48.0f;
        g.drawLine (juce::Line<float> (centre.getPointOnCircumference (skirtR * 0.8f, a),
                                       centre.getPointOnCircumference (skirtR * 0.98f, a)), 1.0f);
    }

    // Cream cap with a soft top light
    const float capR = skirtR * 0.72f;
    const auto cap = juce::Rectangle<float> (capR * 2.0f, capR * 2.0f).withCentre (centre);

    g.setGradientFill (juce::ColourGradient (cream.brighter (0.15f), centre.x - capR * 0.35f, centre.y - capR * 0.6f,
                                             cream.darker (0.45f), centre.x + capR * 0.5f, centre.y + capR, true));
    g.fillEllipse (cap);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawEllipse (cap, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawEllipse (cap.reduced (1.2f), 0.8f);

    // Engraved pointer line from the cap out across the skirt
    const auto inner = centre.getPointOnCircumference (capR * 0.25f, angle);
    const auto outer = centre.getPointOnCircumference (skirtR * 0.95f, angle);
    g.setColour (ink);
    g.drawLine (juce::Line<float> (inner, outer), 2.6f);
}

void KickLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                            bool isMouseOverButton, bool isButtonDown)
{
    const auto b = button.getLocalBounds().toFloat().reduced (4.0f);
    const float corner = 6.0f;

    juce::Path cap;
    cap.addRoundedRectangle (b, corner);

    if (! isButtonDown)
        juce::DropShadow (juce::Colours::black.withAlpha (0.7f), 8, { 0, 3 }).drawForPath (g, cap);

    const auto top    = Colour (isButtonDown ? 0xff252527 : (isMouseOverButton ? 0xff47474a : 0xff3d3d40));
    const auto bottom = Colour (isButtonDown ? 0xff2e2e31 : 0xff222224);
    g.setGradientFill (juce::ColourGradient (top, 0.0f, b.getY(), bottom, 0.0f, b.getBottom(), false));
    g.fillPath (cap);

    g.setColour (juce::Colours::black.withAlpha (0.8f));
    g.strokePath (cap, juce::PathStrokeType (1.0f));

    if (! isButtonDown)
    {
        g.setColour (juce::Colours::white.withAlpha (0.14f));
        g.drawHorizontalLine ((int) b.getY() + 1, b.getX() + corner, b.getRight() - corner);
    }
}

juce::Font KickLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (13.0f, juce::Font::bold)).withExtraKerningFactor (0.12f);
}

void KickLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box)
{
    const auto b = juce::Rectangle<float> ((float) width, (float) height).reduced (0.5f);
    g.setColour (Colour (0xff0c0c0d));
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (b, 4.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawHorizontalLine ((int) b.getBottom() - 1, b.getX() + 4.0f, b.getRight() - 4.0f);

    juce::Path arrow;
    const float ax = b.getRight() - 16.0f, ay = b.getCentreY();
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

juce::Font KickLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (14.0f, juce::Font::bold));
}

juce::Font KickLookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (14.0f));
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
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 74, 20);
        for (auto id : { juce::Slider::textBoxTextColourId, juce::Slider::textBoxOutlineColourId,
                         juce::Slider::textBoxBackgroundColourId, juce::Slider::textBoxHighlightColourId })
            k.slider.setColour (id, lnf.findColour (id));
        addAndMakeVisible (k.slider);

        k.label.setText (specs[i].name, juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        k.label.setColour (juce::Label::textColourId, silkscreen);
        k.label.setFont (juce::Font (juce::FontOptions (11.5f, juce::Font::bold)).withExtraKerningFactor (0.15f));
        addAndMakeVisible (k.label);

        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, specs[i].id, k.slider);
    }

    triggerButton.onClick = [this] { processor.uiTrigger = true; };
    addAndMakeVisible (triggerButton);

    randomButton.onClick = [this] { processor.randomize(); };
    addAndMakeVisible (randomButton);

    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetBox.addItem (processor.getProgramName (i), i + 1);
    presetBox.setSelectedId (processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        processor.setCurrentProgram (presetBox.getSelectedId() - 1);
        processor.uiTrigger = true;
    };
    addAndMakeVisible (presetBox);

    setSize (760, 420);
    startTimerHz (30);
}

KickEditor::~KickEditor()
{
    setLookAndFeel (nullptr);
}

void KickEditor::timerCallback()
{
    // The host can change programs too; keep the display in step
    const int id = processor.getCurrentProgram() + 1;
    if (presetBox.getSelectedId() != id)
        presetBox.setSelectedId (id, juce::dontSendNotification);

    float next = juce::jmax (processor.outputPeak.load(), pulse * 0.8f);
    if (next < 0.01f)
        next = 0.0f;

    if (std::abs (next - pulse) > 1.0e-3f)
    {
        pulse = next;
        repaint (led.expanded (12));
    }
}

void KickEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Charcoal faceplate
    g.setGradientFill (juce::ColourGradient (faceTop, 0.0f, 0.0f, faceBottom, 0.0f, bounds.getBottom(), false));
    g.fillAll();
    drawBrushed (g, bounds, 0.035f, 7);

    // Brushed aluminium header strip with engraved name
    const auto hf = header.toFloat();
    g.setGradientFill (juce::ColourGradient (metalLight, 0.0f, hf.getY(), metalDark, 0.0f, hf.getBottom(), false));
    g.fillRect (hf);
    drawBrushed (g, hf, 0.08f, 11);
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.drawHorizontalLine ((int) hf.getY(), hf.getX(), hf.getRight());
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawHorizontalLine ((int) hf.getBottom(), hf.getX(), hf.getRight());

    const auto titleFont = juce::Font (juce::FontOptions (30.0f, juce::Font::bold)).withExtraKerningFactor (0.18f);
    const auto titleArea = hf.withTrimmedLeft (34.0f).withWidth (260.0f);
    g.setFont (titleFont);
    g.setColour (juce::Colours::white.withAlpha (0.55f));
    g.drawText ("KICK ME", titleArea.translated (0.0f, 1.0f), juce::Justification::centredLeft);
    g.setColour (ink);
    g.drawText ("KICK ME", titleArea, juce::Justification::centredLeft);

    g.setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)).withExtraKerningFactor (0.2f));
    g.setColour (ink.withAlpha (0.7f));
    g.drawText ("KICK DRUM SYNTHESIZER", hf.withTrimmedLeft (200.0f).withWidth (200.0f), juce::Justification::centredLeft);

    g.drawText ("PRESET", presetBox.getBounds().toFloat().translated (-64.0f, 0.0f).withWidth (56.0f),
                juce::Justification::centredRight);

    // Hit LED: dark red-orange glass when idle, glowing amber on each kick
    const auto lf = led.toFloat();
    const float glow = juce::jlimit (0.0f, 1.0f, pulse * 1.6f);
    if (glow > 0.0f)
    {
        g.setGradientFill (juce::ColourGradient (amber.withAlpha (0.6f * glow), lf.getCentreX(), lf.getCentreY(),
                                                 amber.withAlpha (0.0f), lf.getCentreX() + lf.getWidth() * 1.4f, lf.getCentreY(), true));
        g.fillEllipse (lf.expanded (lf.getWidth() * 1.2f));
    }
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (lf.expanded (1.5f));
    g.setGradientFill (juce::ColourGradient (Colour (0xff3a1d05).interpolatedWith (amber.brighter (0.4f), glow),
                                             lf.getCentreX() - 2.0f, lf.getCentreY() - 2.0f,
                                             Colour (0xff1e0f03).interpolatedWith (amber.darker (0.3f), glow),
                                             lf.getRight(), lf.getBottom(), true));
    g.fillEllipse (lf);
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.fillEllipse (lf.reduced (lf.getWidth() * 0.3f).translated (-1.0f, -1.5f).withHeight (lf.getHeight() * 0.25f));

    // Recessed control panel
    const auto pf = panel.toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (pf, 8.0f);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawRoundedRectangle (pf, 8.0f, 1.2f);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawHorizontalLine ((int) pf.getBottom() + 1, pf.getX() + 8.0f, pf.getRight() - 8.0f);

    // Divider between pitch and tone sections
    const float colW = (float) panel.reduced (8, 0).getWidth() / (float) knobs.size();
    const float divX = (float) panel.getX() + 8.0f + colW * 4.0f;
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawVerticalLine ((int) divX, pf.getY() + 18.0f, pf.getBottom() - 18.0f);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawVerticalLine ((int) divX + 1, pf.getY() + 18.0f, pf.getBottom() - 18.0f);

    for (auto c : { juce::Point<float> (14.0f, 14.0f), juce::Point<float> (bounds.getRight() - 14.0f, 14.0f),
                    juce::Point<float> (14.0f, bounds.getBottom() - 14.0f),
                    juce::Point<float> (bounds.getRight() - 14.0f, bounds.getBottom() - 14.0f) })
        drawScrew (g, c);
}

void KickEditor::resized()
{
    auto area = getLocalBounds();

    header = area.removeFromTop (64).reduced (28, 8);
    auto hb = header.reduced (12, 0);
    led = hb.removeFromRight (14).withSizeKeepingCentre (12, 12);
    hb.removeFromRight (14);
    presetBox.setBounds (hb.removeFromRight (180).withSizeKeepingCentre (180, 28));

    area.reduce (28, 0);
    area.removeFromTop (14);

    auto buttons = area.removeFromBottom (70).withSizeKeepingCentre (360, 46);
    triggerButton.setBounds (buttons.removeFromLeft (170));
    randomButton.setBounds (buttons.removeFromRight (170));

    panel = area;

    auto inner = panel.reduced (8, 14);
    const int w = inner.getWidth() / (int) knobs.size();
    for (auto& k : knobs)
    {
        auto col = inner.removeFromLeft (w);
        k.label.setBounds (col.removeFromTop (18));
        k.slider.setBounds (col.reduced (2, 0));
    }
}
