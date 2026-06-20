#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/TunePlusLookAndFeel.h"
#include "../UI/VocalPlusEditorShell.h"

namespace tuneplus
{

class TunePlusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    static constexpr int kContentWidth  = 1080;
    static constexpr int kContentHeight = 820;

    explicit TunePlusAudioProcessorEditor (TunePlusAudioProcessor&);
    ~TunePlusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ScrollableContent;

    void timerCallback() override;
    void loadGenrePreset (int index);
    void loadCompressorPreset (int index);
    void layoutKnob (juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label);
    void paintContent (juce::Graphics& g);
    void layoutContent();

    struct VoiceControls
    {
        juce::ToggleButton enabled { "Enable" };
        juce::Slider interval, level, pan, formant, delay, vibrato;
        juce::Label title;
        juce::Label intervalLabel, levelLabel, panLabel, formantLabel, delayLabel, vibratoLabel;
    };

    void layoutVoiceKnobs (juce::Rectangle<int> area, VoiceControls& voice);

    TunePlusAudioProcessor& processor;
    TunePlusLookAndFeel lookAndFeel;

    vocalplus::ui::VocalPlusEditorShell editorShell;
    std::unique_ptr<ScrollableContent> scrollContent;

    juce::ComboBox genrePresetBox, compressorPresetBox;
    juce::Label genreLabel { {}, "Genre" }, compressorLabel { {}, "Compressor" };
    juce::Label presetDescLabel;
    juce::TextButton captureButton { "Record Stems" };
    juce::TextButton exportButton { "Export WAV" };
    juce::Label statusLabel, pitchLabel, correctionLabel, grLabel;

    juce::ToggleButton autoTuneToggle { "Auto-Tune" };
    juce::Slider retuneSlider, toleranceSlider, formantSlider, detuneSlider;
    juce::Label retuneLabel, toleranceLabel, formantLabel, detuneLabel;
    juce::ComboBox keyBox, scaleBox;
    juce::Label keyLabel { {}, "Key" }, scaleLabel { {}, "Scale" };

    juce::Slider drySlider, wetSlider, widthSlider;
    juce::Label dryLabel, wetLabel, widthLabel;
    juce::ToggleButton monoBlendToggle { "Mono Output" };

    juce::ToggleButton compToggle { "Compressor" };
    juce::Slider compThresholdSlider, compRatioSlider, compAttackSlider, compReleaseSlider;
    juce::Slider compMakeupSlider, compHighPassSlider, compMixSlider;
    juce::Label compThresholdLabel, compRatioLabel, compAttackLabel, compReleaseLabel;
    juce::Label compMakeupLabel, compHighPassLabel, compMixLabel;

    std::array<VoiceControls, 3> voices;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ButtonAttachment> autoTuneAttachment, monoBlendAttachment, compToggleAttachment;
    std::unique_ptr<Attachment> retuneAttachment, toleranceAttachment, formantAttachment, detuneAttachment;
    std::unique_ptr<Attachment> dryAttachment, wetAttachment, widthAttachment;
    std::unique_ptr<ComboAttachment> keyAttachment, scaleAttachment;
    std::unique_ptr<Attachment> compThresholdAttachment, compRatioAttachment, compAttackAttachment;
    std::unique_ptr<Attachment> compReleaseAttachment, compMakeupAttachment, compHighPassAttachment;
    std::unique_ptr<Attachment> compMixAttachment;

    struct VoiceAttachments
    {
        std::unique_ptr<ButtonAttachment> enabled;
        std::unique_ptr<Attachment> interval, level, pan, formant, delay, vibrato;
    };
    std::array<VoiceAttachments, 3> voiceAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunePlusAudioProcessorEditor)
};

} // namespace tuneplus
