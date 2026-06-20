#pragma once

#include <JuceHeader.h>

namespace voxplus
{

class VoxPlusLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour kBackground    { 0xff080c0a };
    static inline const juce::Colour kPanel           { 0xff0e1410 };
    static inline const juce::Colour kPanelBorder     { 0xff1a3d28 };
    static inline const juce::Colour kNeonGreen       { 0xff00ff88 };
    static inline const juce::Colour kNeonGreenDim    { 0xff00cc6a };
    static inline const juce::Colour kNeonGlow        { 0x4000ff88 };
    static inline const juce::Colour kTextPrimary   { 0xffe8f5ee };
    static inline const juce::Colour kTextSecondary { 0xff7a9a88 };
    static inline const juce::Colour kKnobTrack       { 0xff1a2820 };
    static inline const juce::Colour kKnobFill        { 0xff00ff88 };
    static inline const juce::Colour kMeterGreen      { 0xff00ff88 };
    static inline const juce::Colour kMeterBg         { 0xff111916 };

    VoxPlusLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, kBackground);
        setColour (juce::Label::textColourId, kTextPrimary);
        setColour (juce::ComboBox::backgroundColourId, kPanel);
        setColour (juce::ComboBox::outlineColourId, kPanelBorder);
        setColour (juce::ComboBox::textColourId, kTextPrimary);
        setColour (juce::PopupMenu::backgroundColourId, kPanel);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, kNeonGreen.withAlpha (0.25f));
        setColour (juce::PopupMenu::textColourId, kTextPrimary);
        setColour (juce::TextButton::buttonColourId, kPanel);
        setColour (juce::TextButton::buttonOnColourId, kNeonGreen);
        setColour (juce::TextButton::textColourOffId, kTextPrimary);
        setColour (juce::TextButton::textColourOnId, kTextPrimary);
        setColour (juce::Slider::textBoxTextColourId, kTextPrimary);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ToggleButton::textColourId, kTextPrimary);
        setColour (juce::ToggleButton::tickColourId, kNeonGreen);
        setColour (juce::ToggleButton::tickDisabledColourId, kKnobTrack);
        setColour (juce::ScrollBar::backgroundColourId, kBackground);
        setColour (juce::ScrollBar::thumbColourId, kNeonGreenDim.withAlpha (0.6f));
        setColour (juce::ScrollBar::trackColourId, kPanel);
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

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused (shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        auto bounds = button.getLocalBounds().toFloat().reduced (0.0f, 2.0f);
        const bool on = button.getToggleState();

        juce::Rectangle<float> switchBounds = bounds.removeFromLeft (juce::jmin (36.0f, bounds.getHeight() * 1.6f)).reduced (0.0f, 3.0f);
        g.setColour (on ? kNeonGreen.withAlpha (0.3f) : kKnobTrack);
        g.fillRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f);
        g.setColour (on ? kNeonGreen : kPanelBorder);
        g.drawRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f, 1.0f);

        const float thumbSize = switchBounds.getHeight() - 4.0f;
        const float thumbX = on ? switchBounds.getRight() - thumbSize - 2.0f : switchBounds.getX() + 2.0f;
        g.setColour (on ? kNeonGreen : kTextSecondary);
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

        g.setColour (accent ? kNeonGreen.withAlpha (0.75f)
                            : (shouldDrawButtonAsHighlighted ? kPanel.brighter (0.08f) : kPanel));
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (accent ? kNeonGreen : kPanelBorder);
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
        g.setColour (box.hasKeyboardFocus (false) ? kNeonGreen.withAlpha (0.6f) : kPanelBorder);
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 6.0f, 1.0f);

        juce::Path arrow;
        const float ax = static_cast<float> (width) - 14.0f;
        const float ay = static_cast<float> (height) * 0.5f;
        arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
        g.setColour (kNeonGreenDim);
        g.fillPath (arrow);
    }

    void drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar,
                        int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override
    {
        juce::ignoreUnused (scrollbar, isMouseOver, isMouseDown);
        g.fillAll (kBackground);

        juce::Rectangle<int> thumbBounds;

        if (isScrollbarVertical)
            thumbBounds = { x + 2, thumbStartPosition, width - 4, thumbSize };
        else
            thumbBounds = { thumbStartPosition, y + 2, thumbSize, height - 4 };

        g.setColour (kNeonGreen.withAlpha (0.35f));
        g.fillRoundedRectangle (thumbBounds.toFloat(), 4.0f);
        g.setColour (kNeonGreenDim.withAlpha (0.5f));
        g.drawRoundedRectangle (thumbBounds.toFloat(), 4.0f, 1.0f);
    }
};

} // namespace voxplus
