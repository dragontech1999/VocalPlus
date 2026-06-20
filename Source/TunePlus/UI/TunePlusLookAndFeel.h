#pragma once

#include <JuceHeader.h>

namespace tuneplus
{

class TunePlusLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour kBackground     { 0xff0d0f14 };
    static inline const juce::Colour kPanel          { 0xff161a22 };
    static inline const juce::Colour kPanelBorder    { 0xff252b38 };
    static inline const juce::Colour kTextPrimary    { 0xffeef0f5 };
    static inline const juce::Colour kTextSecondary  { 0xff8a919e };
    static inline const juce::Colour kAccent         { 0xffff6b4a };
    static inline const juce::Colour kAccentAlt      { 0xff4ecdc4 };
    static inline const juce::Colour kKnobTrack      { 0xff2a3040 };
    static inline const juce::Colour kKnobFill       { 0xffff6b4a };
    static inline const juce::Colour kCompMeter      { 0xff4ecdc4 };

    TunePlusLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, kBackground);
        setColour (juce::Label::textColourId, kTextPrimary);
        setColour (juce::ComboBox::backgroundColourId, kPanel);
        setColour (juce::ComboBox::outlineColourId, kPanelBorder);
        setColour (juce::ComboBox::textColourId, kTextPrimary);
        setColour (juce::PopupMenu::backgroundColourId, kPanel);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, kAccent.withAlpha (0.35f));
        setColour (juce::PopupMenu::textColourId, kTextPrimary);
        setColour (juce::TextButton::buttonColourId, kPanel);
        setColour (juce::TextButton::buttonOnColourId, kAccent);
        setColour (juce::TextButton::textColourOffId, kTextPrimary);
        setColour (juce::TextButton::textColourOnId, kTextPrimary);
        setColour (juce::Slider::textBoxTextColourId, kTextPrimary);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ToggleButton::textColourId, kTextPrimary);
        setColour (juce::ToggleButton::tickColourId, kAccent);
        setColour (juce::ToggleButton::tickDisabledColourId, kKnobTrack);
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
        g.drawEllipse (knob.reduced (radius * 0.18f), 1.2f);

        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, radius * 0.82f, radius * 0.82f,
                                0.0f, startAngle, angle, true);
        g.setColour (slider.isEnabled() ? kKnobFill : kKnobTrack);
        g.strokePath (valueArc, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

        juce::Path indicator;
        indicator.startNewSubPath (centre.x, centre.y - radius * 0.55f);
        indicator.lineTo (centre.x, centre.y - radius * 0.78f);
        g.setColour (kTextPrimary.withAlpha (0.9f));
        g.strokePath (indicator, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded),
                      juce::AffineTransform::rotation (angle, centre.x, centre.y));
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused (shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        auto bounds = button.getLocalBounds().toFloat().reduced (0.0f, 2.0f);
        const bool on = button.getToggleState();

        juce::Rectangle<float> switchBounds = bounds.removeFromLeft (juce::jmin (36.0f, bounds.getHeight() * 1.6f)).reduced (0.0f, 3.0f);
        g.setColour (on ? kAccent.withAlpha (0.35f) : kKnobTrack);
        g.fillRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f);
        g.setColour (kPanelBorder);
        g.drawRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f, 1.0f);

        const float thumbSize = switchBounds.getHeight() - 4.0f;
        const float thumbX = on ? switchBounds.getRight() - thumbSize - 2.0f : switchBounds.getX() + 2.0f;
        g.setColour (on ? kAccent : kTextSecondary);
        g.fillEllipse (thumbX, switchBounds.getY() + 2.0f, thumbSize, thumbSize);

        g.setColour (kTextPrimary);
        g.setFont (juce::FontOptions (11.5f));
        g.drawText (button.getButtonText(), bounds.reduced (8.0f, 0.0f),
                    juce::Justification::centredLeft, true);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused (backgroundColour);
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        const bool accent = button.getToggleState() || shouldDrawButtonAsDown;

        g.setColour (accent ? kAccent.withAlpha (0.85f)
                            : (shouldDrawButtonAsHighlighted ? kPanel.brighter (0.08f) : kPanel));
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (kPanelBorder);
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
        g.setColour (box.hasKeyboardFocus (false) ? kAccent.withAlpha (0.5f) : kPanelBorder);
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 6.0f, 1.0f);

        juce::Path arrow;
        const float ax = static_cast<float> (width) - 14.0f;
        const float ay = static_cast<float> (height) * 0.5f;
        arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
        g.setColour (kTextSecondary);
        g.fillPath (arrow);
    }
};

} // namespace tuneplus
