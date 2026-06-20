#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/EQPlusLookAndFeel.h"
#include "UI/EQCurveDisplay.h"
#include "../UI/VocalPlusEditorShell.h"

namespace eqplus
{

class EQPlusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    static constexpr int kContentWidth  = 1180;
    static constexpr int kContentHeight = 780;

    explicit EQPlusAudioProcessorEditor (EQPlusAudioProcessor&);
    ~EQPlusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ScrollableContent;

    void timerCallback() override;
    void updateSelectedBandControls();
    void layoutKnobRow (juce::Rectangle<int>& row, int numKnobs, juce::Slider* sliders[], juce::Label* labels[]);
    void paintContent (juce::Graphics& g);
    void layoutContent();

    EQPlusAudioProcessor& processor;
    EQPlusLookAndFeel lookAndFeel;
    vocalplus::ui::VocalPlusEditorShell editorShell;
    std::unique_ptr<ScrollableContent> scrollContent;
    std::unique_ptr<EQCurveDisplay> curveDisplay;

    juce::ComboBox presetBox, typeBox, channelBox, processingBox, displayRangeBox;
    juce::Label presetLabel { {}, "Preset" };
    juce::Label typeLabel { {}, "Type" }, channelLabel { {}, "Channel" };
    juce::Label processingLabel { {}, "Mode" }, displayRangeLabel { {}, "Range" };
    juce::Label bandTitle, peakLabel, autoGainLabel, statusLabel;
    juce::Label freqLabel { {}, "Freq" }, gainLabel { {}, "Gain" }, qLabel { {}, "Q" };
    juce::Label dynThresholdLabel { {}, "Thresh" }, dynRatioLabel { {}, "Ratio" };
    juce::Label dynAttackLabel { {}, "Attack" }, dynReleaseLabel { {}, "Release" };
    juce::Label outputLabel { {}, "Output" };

    juce::ToggleButton bandEnableToggle { "On" };
    juce::ToggleButton bandSoloToggle { "Solo" };
    juce::ToggleButton dynToggle { "Dynamic" };
    juce::ToggleButton autoGainToggle { "Auto Gain" };
    juce::ToggleButton phaseInvertToggle { "Invert" };
    juce::ToggleButton preSpectrumToggle { "Pre" };
    juce::ToggleButton postSpectrumToggle { "Post" };

    juce::Slider freqSlider, gainSlider, qSlider;
    juce::Slider dynThresholdSlider, dynRatioSlider, dynAttackSlider, dynReleaseSlider;
    juce::Slider outputSlider, spectrumSpeedSlider;
    juce::Label spectrumSpeedLabel { {}, "Speed" };

    juce::TextButton aiSuggestButton { "AI Suggest" };
    juce::TextButton eqMatchButton { "EQ Match" };
    juce::TextButton captureRefButton { "Capture" };
    juce::TextButton deEssButton { "De-Ess" };
    juce::TextButton abButton { "A/B" };
    juce::TextButton copyButton { "Copy" };
    juce::TextButton pasteButton { "Paste" };
    juce::TextButton freezeButton { "Freeze" };

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> freqAttachment, gainAttachment, qAttachment;
    std::unique_ptr<Attachment> dynThresholdAttachment, dynRatioAttachment, dynAttackAttachment, dynReleaseAttachment;
    std::unique_ptr<Attachment> outputAttachment, spectrumSpeedAttachment;
    std::unique_ptr<ButtonAttachment> bandEnableAttachment, bandSoloAttachment, dynAttachment;
    std::unique_ptr<ButtonAttachment> autoGainAttachment, phaseInvertAttachment;
    std::unique_ptr<ButtonAttachment> preSpectrumAttachment, postSpectrumAttachment;
    std::unique_ptr<ComboAttachment> typeAttachment, channelAttachment;
    std::unique_ptr<ComboAttachment> processingAttachment, displayRangeAttachment;

    int currentBand = 0;
    float lastPeakDisplay = -100.0f;
    float lastAutoGainDisplay = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQPlusAudioProcessorEditor)
};

} // namespace eqplus
