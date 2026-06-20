#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/VocalAIPlusLookAndFeel.h"
#include "../UI/VocalPlusEditorShell.h"

namespace vocalaiplus
{

class VocalAIPlusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    static constexpr int kContentWidth  = 1180;
    static constexpr int kContentHeight = 820;

    explicit VocalAIPlusAudioProcessorEditor (VocalAIPlusAudioProcessor&);
    ~VocalAIPlusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class PresetButton;
    class ScrollableContent;

    void timerCallback() override;
    void loadPreset (int index);
    void paintContent (juce::Graphics& g);
    void layoutContent();
    void drawNeonPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const;
    void rebuildPresetButtons();
    void layoutModuleRow (juce::Rectangle<int>& area, juce::Slider& a, juce::Slider& b,
                          juce::Label& la, juce::Label& lb);

    VocalAIPlusAudioProcessor& processor;
    VocalAIPlusLookAndFeel lookAndFeel;
    vocalplus::ui::VocalPlusEditorShell editorShell;
    std::unique_ptr<ScrollableContent> scrollContent;

    juce::Label titleLabel { {}, "VocalAI+" };
    juce::Label subtitleLabel { {}, "AI vocal mastering by Neeberman" };
    juce::Label presetDescLabel, rationaleLabel;
    juce::Label analysisLabel, meterLabel;
    juce::TextButton aiMasterButton { "AI Master" };
    juce::ComboBox genreBox;

    juce::Slider noiseAmt, eqLow, eqLowMid, eqPres, eqAir;
    juce::Slider compThr, compRatioKnob, compAtk, compRel, compMakeup;
    juce::Slider deEssFreq, deEssCut, deEssSens, excAmt, excMix, revWet, revRoom;
    juce::Slider limCeil, limGain, limTarget, outputGain, mixSlider;

    juce::Label noiseLbl { {}, "NR" }, eqLowLbl { {}, "Low" }, eqLowMidLbl { {}, "LMid" };
    juce::Label eqPresLbl { {}, "Pres" }, eqAirLbl { {}, "Air" };
    juce::Label compThrLbl { {}, "Thr" }, compRatioLbl { {}, "Ratio" };
    juce::Label compAtkLbl { {}, "Atk" }, compRelLbl { {}, "Rel" }, compMakeupLbl { {}, "Mkup" };
    juce::Label deEssFreqLbl { {}, "Freq" }, deEssCutLbl { {}, "Cut" }, deEssSensLbl { {}, "Sens" };
    juce::Label excAmtLbl { {}, "Amt" }, excMixLbl { {}, "Mix" };
    juce::Label revWetLbl { {}, "Wet" }, revRoomLbl { {}, "Room" };
    juce::Label limCeilLbl { {}, "Ceil" }, limGainLbl { {}, "Gain" }, limTargetLbl { {}, "LUFS" };
    juce::Label outputLbl { {}, "Out" }, mixLbl { {}, "Mix" }, genreLbl { {}, "Genre" };

    juce::OwnedArray<PresetButton> presetButtons;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> noiseAmtAtt, eqLowAtt, eqLowMidAtt, eqPresAtt, eqAirAtt;
    std::unique_ptr<Attachment> compThrAtt, compRatioAtt, compAtkAtt, compRelAtt, compMakeupAtt;
    std::unique_ptr<Attachment> deEssFreqAtt, deEssCutAtt, deEssSensAtt;
    std::unique_ptr<Attachment> excAmtAtt, excMixAtt, revWetAtt, revRoomAtt;
    std::unique_ptr<Attachment> limCeilAtt, limGainAtt, limTargetAtt, outputAtt, mixAtt;
    std::unique_ptr<ComboAttachment> genreAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalAIPlusAudioProcessorEditor)
};

} // namespace vocalaiplus
