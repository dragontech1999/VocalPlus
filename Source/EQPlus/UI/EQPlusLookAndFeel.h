#pragma once

#include <JuceHeader.h>

namespace eqplus
{

class EQPlusLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour kBackground     { 0xff080810 };
    static inline const juce::Colour kPanel          { 0xff10101a };
    static inline const juce::Colour kPanelBorder    { 0xff222233 };
    static inline const juce::Colour kTextPrimary    { 0xffe8eaf0 };
    static inline const juce::Colour kTextSecondary  { 0xff787890 };
    static inline const juce::Colour kAccent         { 0xff50c8e0 };
    static inline const juce::Colour kNeonCyan       { 0xff50c8e0 };
    static inline const juce::Colour kNeonMagenta    { 0xffe050a0 };
    static inline const juce::Colour kNeonPurple     { 0xff9070ff };
    static inline const juce::Colour kNeonTeal       { 0xff40d0b0 };
    static inline const juce::Colour kBandBlue       { 0xff5090ff };
    static inline const juce::Colour kKnobTrack      { 0xff181824 };
    static inline const juce::Colour kKnobFill       { 0xff50c8e0 };

    EQPlusLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, kBackground);
        setColour (juce::Label::textColourId, kTextPrimary);
        setColour (juce::ComboBox::backgroundColourId, kPanel);
        setColour (juce::ComboBox::outlineColourId, kPanelBorder);
        setColour (juce::ComboBox::textColourId, kTextPrimary);
        setColour (juce::PopupMenu::backgroundColourId, kPanel);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, kAccent.withAlpha (0.25f));
        setColour (juce::PopupMenu::textColourId, kTextPrimary);
        setColour (juce::TextButton::buttonColourId, kPanel);
        setColour (juce::TextButton::buttonOnColourId, kAccent.withAlpha (0.3f));
        setColour (juce::TextButton::textColourOffId, kTextPrimary);
        setColour (juce::TextButton::textColourOnId, kAccent);
        setColour (juce::Slider::textBoxTextColourId, kTextPrimary);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ToggleButton::textColourId, kTextSecondary);
        setColour (juce::ToggleButton::tickColourId, kAccent);
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return juce::FontOptions (11.0f);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider& slider) override
    {
        juce::ignoreUnused (slider);
        const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                    static_cast<float> (width), static_cast<float> (height))
                                .reduced (4.0f);
        const float diameter = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const auto knob = bounds.withSizeKeepingCentre (diameter, diameter);
        const auto centre = knob.getCentre();
        const float radius = diameter * 0.5f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);

        g.setColour (kKnobTrack);
        g.fillEllipse (knob.reduced (2.0f));
        g.setColour (kPanelBorder);
        g.drawEllipse (knob.reduced (2.0f), 1.0f);

        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, radius * 0.78f, radius * 0.78f,
                                0.0f, startAngle, angle, true);
        g.setColour (kAccent);
        g.strokePath (valueArc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

        g.setColour (kAccent.withAlpha (0.85f));
        g.fillEllipse (centre.x - 3.0f, centre.y - 3.0f, 6.0f, 6.0f);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused (backgroundColour);
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        const bool accent = button.getToggleState() || shouldDrawButtonAsDown;

        g.setColour (accent ? kAccent.withAlpha (0.18f)
                            : (shouldDrawButtonAsHighlighted ? kPanel.brighter (0.08f) : kPanel));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (accent ? kAccent.withAlpha (0.55f) : kPanelBorder);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool,
                       int, int, int, int, juce::ComboBox& box) override
    {
        const juce::Rectangle<int> bounds (0, 0, width, height);
        g.setColour (kPanel);
        g.fillRoundedRectangle (bounds.toFloat().reduced (0.5f), 4.0f);
        g.setColour (box.hasKeyboardFocus (false) ? kAccent.withAlpha (0.45f) : kPanelBorder);
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 4.0f, 1.0f);
    }
};

} // namespace eqplus
