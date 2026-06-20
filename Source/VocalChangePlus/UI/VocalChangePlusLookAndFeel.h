#pragma once

#include <JuceHeader.h>

namespace vocalchangeplus
{

class VocalChangePlusLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour kBackground    { 0xff121018 };
    static inline const juce::Colour kPanel           { 0xff1a1420 };
    static inline const juce::Colour kPanelBorder     { 0xff3d1528 };
    static inline const juce::Colour kNeonRed         { 0xffff1744 };
    static inline const juce::Colour kNeonRedDim      { 0xffcc1136 };
    static inline const juce::Colour kNeonGlow        { 0x40ff1744 };
    static inline const juce::Colour kTextPrimary   { 0xfff5e8ec };
    static inline const juce::Colour kTextSecondary { 0xff9a7888 };
    static inline const juce::Colour kKnobTrack       { 0xff281820 };
    static inline const juce::Colour kKnobFill        { 0xffff1744 };
    static inline const juce::Colour kPresetOff       { 0xff1e1624 };
    static inline const juce::Colour kPresetHover     { 0xff2a1828 };

    VocalChangePlusLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, kBackground);
        setColour (juce::Label::textColourId, kTextPrimary);
        setColour (juce::ComboBox::backgroundColourId, kPanel);
        setColour (juce::ComboBox::outlineColourId, kPanelBorder);
        setColour (juce::ComboBox::textColourId, kTextPrimary);
        setColour (juce::PopupMenu::backgroundColourId, kPanel);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, kNeonRed.withAlpha (0.25f));
        setColour (juce::PopupMenu::textColourId, kTextPrimary);
        setColour (juce::TextButton::buttonColourId, kPanel);
        setColour (juce::TextButton::buttonOnColourId, kNeonRed);
        setColour (juce::TextButton::textColourOffId, kTextPrimary);
        setColour (juce::TextButton::textColourOnId, kTextPrimary);
        setColour (juce::Slider::textBoxTextColourId, kTextPrimary);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
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
        g.drawEllipse (knob.reduced (radius * 0.18f), 1.0f);

        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, radius * 0.82f, radius * 0.82f,
                                0.0f, startAngle, angle, true);
        g.setColour (slider.isEnabled() ? kKnobFill : kKnobTrack);
        g.strokePath (valueArc, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

        g.setColour (kNeonGlow);
        g.strokePath (valueArc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

        juce::Path indicator;
        indicator.startNewSubPath (centre.x, centre.y - radius * 0.55f);
        indicator.lineTo (centre.x, centre.y - radius * 0.78f);
        g.setColour (kTextPrimary.withAlpha (0.9f));
        g.strokePath (indicator, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded),
                      juce::AffineTransform::rotation (angle, centre.x, centre.y));
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused (backgroundColour);
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        const bool accent = button.getToggleState() || shouldDrawButtonAsDown;

        g.setColour (accent ? kNeonRed.withAlpha (0.75f)
                            : (shouldDrawButtonAsHighlighted ? kPresetHover : kPresetOff));
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (accent ? kNeonRed : kPanelBorder);
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override
    {
        juce::ignoreUnused (isButtonDown, buttonX, buttonY, buttonW, buttonH);
        const juce::Rectangle<int> bounds (0, 0, width, height);
        g.setColour (kPanel);
        g.fillRoundedRectangle (bounds.toFloat().reduced (0.5f), 6.0f);
        g.setColour (box.hasKeyboardFocus (false) ? kNeonRed.withAlpha (0.6f) : kPanelBorder);
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 6.0f, 1.0f);

        juce::Path arrow;
        const float ax = static_cast<float> (width) - 14.0f;
        const float ay = static_cast<float> (height) * 0.5f;
        arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
        g.setColour (kNeonRedDim);
        g.fillPath (arrow);
    }
};

} // namespace vocalchangeplus
