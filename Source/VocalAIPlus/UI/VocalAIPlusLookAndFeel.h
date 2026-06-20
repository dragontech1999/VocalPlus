#pragma once

#include <JuceHeader.h>

namespace vocalaiplus
{

class VocalAIPlusLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour kBackground    { 0xff080808 };
    static inline const juce::Colour kPanel           { 0xff101810 };
    static inline const juce::Colour kPanelBorder     { 0xff00ff88 };
    static inline const juce::Colour kNeonGreen         { 0xff00ff88 };
    static inline const juce::Colour kNeonGlow          { 0x4000ff88 };
    static inline const juce::Colour kTextPrimary       { 0xffe8fff0 };
    static inline const juce::Colour kTextSecondary     { 0xff7a9a88 };
    static inline const juce::Colour kKnobTrack         { 0xff182018 };
    static inline const juce::Colour kKnobFill          { 0xff00ff88 };
    static inline const juce::Colour kPresetOff         { 0xff121812 };
    static inline const juce::Colour kPresetHover       { 0xff1a2818 };

    VocalAIPlusLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, kBackground);
        setColour (juce::Label::textColourId, kTextPrimary);
        setColour (juce::ComboBox::backgroundColourId, kPanel);
        setColour (juce::ComboBox::outlineColourId, kPanelBorder.withAlpha (0.5f));
        setColour (juce::ComboBox::textColourId, kTextPrimary);
        setColour (juce::PopupMenu::backgroundColourId, kPanel);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, kNeonGreen.withAlpha (0.25f));
        setColour (juce::PopupMenu::textColourId, kTextPrimary);
        setColour (juce::TextButton::buttonColourId, kPanel);
        setColour (juce::TextButton::buttonOnColourId, kNeonGreen.withAlpha (0.35f));
        setColour (juce::TextButton::textColourOffId, kTextPrimary);
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
                                .reduced (3.0f);
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
        g.setColour (kKnobFill);
        g.strokePath (valueArc, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        g.setColour (kNeonGlow);
        g.strokePath (valueArc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused (backgroundColour);
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        const auto base = button.getToggleState() ? kNeonGreen.withAlpha (0.25f)
                          : shouldDrawButtonAsDown ? kPresetHover
                          : shouldDrawButtonAsHighlighted ? kPresetHover
                          : kPresetOff;
        g.setColour (base);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (kPanelBorder.withAlpha (button.getToggleState() ? 0.9f : 0.45f));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }
};

} // namespace vocalaiplus
