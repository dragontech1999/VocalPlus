#include "PluginEditor.h"
#include "ParameterIDs.h"

namespace tuneplus
{

using namespace ParamIDs;

class TunePlusAudioProcessorEditor::ScrollableContent : public juce::Component
{
public:
    explicit ScrollableContent (TunePlusAudioProcessorEditor& ownerIn) : owner (ownerIn) {}

    void paint (juce::Graphics& g) override { owner.paintContent (g); }
    void resized() override { owner.layoutContent(); }

private:
    TunePlusAudioProcessorEditor& owner;
};

namespace
{
    void styleLabel (juce::Label& label, bool secondary = false)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId,
                         secondary ? TunePlusLookAndFeel::kTextSecondary
                                   : TunePlusLookAndFeel::kTextPrimary);
        label.setFont (juce::FontOptions (secondary ? 10.5f : 11.0f));
    }

    void styleSlider (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 16);
        slider.setColour (juce::Slider::textBoxTextColourId, TunePlusLookAndFeel::kTextSecondary);
        slider.setScrollWheelEnabled (true);
    }
}

TunePlusAudioProcessorEditor::TunePlusAudioProcessorEditor (TunePlusAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      editorShell (*this, TunePlusLookAndFeel::kAccent)
{
    setLookAndFeel (&lookAndFeel);

    scrollContent = std::make_unique<ScrollableContent> (*this);
    addAndMakeVisible (editorShell);
    editorShell.setScrollContent (scrollContent.get(), kContentWidth, kContentHeight);

    auto addToContent = [this] (juce::Component& c) { scrollContent->addAndMakeVisible (c); };

    for (int i = 0; i < processor.getPresetManager().getNumGenrePresets(); ++i)
        genrePresetBox.addItem (processor.getPresetManager().getGenrePreset (i).name, i + 1);

    for (int i = 0; i < processor.getPresetManager().getNumCompressorPresets(); ++i)
        compressorPresetBox.addItem (processor.getPresetManager().getCompressorPreset (i).name, i + 1);

    genrePresetBox.setSelectedId (processor.getCurrentGenreProgram() + 1, juce::dontSendNotification);
    compressorPresetBox.setSelectedId (processor.getCurrentCompressorProgram() + 1, juce::dontSendNotification);

    genrePresetBox.onChange = [this]
    {
        loadGenrePreset (genrePresetBox.getSelectedItemIndex());
        compressorPresetBox.setSelectedId (processor.getCurrentCompressorProgram() + 1, juce::dontSendNotification);
    };

    compressorPresetBox.onChange = [this]
    {
        loadCompressorPreset (compressorPresetBox.getSelectedItemIndex());
    };

    styleLabel (genreLabel, true);
    styleLabel (compressorLabel, true);
    styleLabel (presetDescLabel, true);
    styleLabel (pitchLabel);
    styleLabel (correctionLabel, true);
    styleLabel (grLabel, true);
    styleLabel (statusLabel, true);

    presetDescLabel.setText (processor.getPresetManager().getGenrePreset (0).description,
                             juce::dontSendNotification);
    pitchLabel.setText ("—", juce::dontSendNotification);
    correctionLabel.setText ("Correction 0 ct", juce::dontSendNotification);
    grLabel.setText ("GR 0 dB", juce::dontSendNotification);
    statusLabel.setText ("Ready", juce::dontSendNotification);

    addToContent (genrePresetBox);
    addToContent (compressorPresetBox);
    addToContent (genreLabel);
    addToContent (compressorLabel);
    addToContent (presetDescLabel);
    addToContent (captureButton);
    addToContent (exportButton);
    addToContent (statusLabel);
    addToContent (pitchLabel);
    addToContent (correctionLabel);
    addToContent (grLabel);

    auto& apvts = processor.getAPVTS();

    addToContent (autoTuneToggle);
    autoTuneAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoTuneEnabled, autoTuneToggle);

    const juce::StringArray autoTuneKnobs { "Retune", "Tolerance", "Formant", "Detune" };
    juce::Slider* autoTuneSliders[] { &retuneSlider, &toleranceSlider, &formantSlider, &detuneSlider };
    juce::Label* autoTuneLabels[] { &retuneLabel, &toleranceLabel, &formantLabel, &detuneLabel };

    for (int i = 0; i < 4; ++i)
    {
        styleSlider (*autoTuneSliders[i]);
        autoTuneLabels[i]->setText (autoTuneKnobs[i], juce::dontSendNotification);
        styleLabel (*autoTuneLabels[i], true);
        addToContent (*autoTuneSliders[i]);
        addToContent (*autoTuneLabels[i]);
    }

    retuneAttachment = std::make_unique<Attachment> (apvts, ParamIDs::retuneSpeed, retuneSlider);
    toleranceAttachment = std::make_unique<Attachment> (apvts, ParamIDs::tolerance, toleranceSlider);
    formantAttachment = std::make_unique<Attachment> (apvts, ParamIDs::formantPreserve, formantSlider);
    detuneAttachment = std::make_unique<Attachment> (apvts, ParamIDs::detune, detuneSlider);

    for (int i = 0; i < TunePlusPresetManager::getNoteNames().size(); ++i)
        keyBox.addItem (TunePlusPresetManager::getNoteNames()[i], i + 1);
    for (int i = 0; i < TunePlusPresetManager::getScaleNames().size(); ++i)
        scaleBox.addItem (TunePlusPresetManager::getScaleNames()[i], i + 1);

    addToContent (keyBox);
    addToContent (scaleBox);
    addToContent (keyLabel);
    addToContent (scaleLabel);
    keyAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::rootNote, keyBox);
    scaleAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::scaleType, scaleBox);

    juce::Slider* mixSliders[] { &drySlider, &wetSlider, &widthSlider };
    juce::Label* mixLabels[] { &dryLabel, &wetLabel, &widthLabel };
    const juce::StringArray mixNames { "Dry", "Wet", "Width" };

    for (int i = 0; i < 3; ++i)
    {
        styleSlider (*mixSliders[i]);
        mixLabels[i]->setText (mixNames[i], juce::dontSendNotification);
        styleLabel (*mixLabels[i], true);
        addToContent (*mixSliders[i]);
        addToContent (*mixLabels[i]);
    }

    dryAttachment = std::make_unique<Attachment> (apvts, ParamIDs::dryLevel, drySlider);
    wetAttachment = std::make_unique<Attachment> (apvts, ParamIDs::wetLevel, wetSlider);
    widthAttachment = std::make_unique<Attachment> (apvts, ParamIDs::stereoWidth, widthSlider);

    addToContent (monoBlendToggle);
    monoBlendAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::blendToMono, monoBlendToggle);

    addToContent (compToggle);
    compToggleAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::compEnabled, compToggle);

    const juce::StringArray compKnobs { "Threshold", "Ratio", "Attack", "Release", "Makeup", "HPF", "Mix" };
    juce::Slider* compSliders[] { &compThresholdSlider, &compRatioSlider, &compAttackSlider, &compReleaseSlider,
                                  &compMakeupSlider, &compHighPassSlider, &compMixSlider };
    juce::Label* compLabels[] { &compThresholdLabel, &compRatioLabel, &compAttackLabel, &compReleaseLabel,
                                &compMakeupLabel, &compHighPassLabel, &compMixLabel };

    for (int i = 0; i < 7; ++i)
    {
        styleSlider (*compSliders[i]);
        compLabels[i]->setText (compKnobs[i], juce::dontSendNotification);
        styleLabel (*compLabels[i], true);
        addToContent (*compSliders[i]);
        addToContent (*compLabels[i]);
    }

    compThresholdAttachment = std::make_unique<Attachment> (apvts, ParamIDs::compThreshold, compThresholdSlider);
    compRatioAttachment = std::make_unique<Attachment> (apvts, ParamIDs::compRatio, compRatioSlider);
    compAttackAttachment = std::make_unique<Attachment> (apvts, ParamIDs::compAttack, compAttackSlider);
    compReleaseAttachment = std::make_unique<Attachment> (apvts, ParamIDs::compRelease, compReleaseSlider);
    compMakeupAttachment = std::make_unique<Attachment> (apvts, ParamIDs::compMakeup, compMakeupSlider);
    compHighPassAttachment = std::make_unique<Attachment> (apvts, ParamIDs::compHighPass, compHighPassSlider);
    compMixAttachment = std::make_unique<Attachment> (apvts, ParamIDs::compMix, compMixSlider);

    const juce::StringArray voiceKnobNames { "Interval", "Level", "Pan", "Formant", "Delay", "Vibrato" };

    for (size_t i = 0; i < voices.size(); ++i)
    {
        auto& v = voices[i];
        v.title.setText ("Harmony " + juce::String (static_cast<int> (i) + 1), juce::dontSendNotification);
        styleLabel (v.title);
        v.title.setFont (juce::FontOptions (12.0f).withStyle ("Medium"));
        addToContent (v.title);
        addToContent (v.enabled);

        juce::Slider* voiceSliders[] { &v.interval, &v.level, &v.pan, &v.formant, &v.delay, &v.vibrato };
        juce::Label* voiceLabels[] { &v.intervalLabel, &v.levelLabel, &v.panLabel,
                                      &v.formantLabel, &v.delayLabel, &v.vibratoLabel };

        for (int k = 0; k < 6; ++k)
        {
            styleSlider (*voiceSliders[k]);
            voiceLabels[k]->setText (voiceKnobNames[k], juce::dontSendNotification);
            styleLabel (*voiceLabels[k], true);
            addToContent (*voiceSliders[k]);
            addToContent (*voiceLabels[k]);
        }

        v.interval.setRange (-24, 24, 1);
        v.interval.textFromValueFunction = [] (double val) { return juce::String (static_cast<int> (val)); };

        const auto idx = static_cast<int> (i);
        voiceAttachments[i].enabled = std::make_unique<ButtonAttachment> (apvts, ParamIDs::voiceEnabled (idx), v.enabled);
        voiceAttachments[i].interval = std::make_unique<Attachment> (apvts, ParamIDs::voiceInterval (idx), v.interval);
        voiceAttachments[i].level = std::make_unique<Attachment> (apvts, ParamIDs::voiceLevel (idx), v.level);
        voiceAttachments[i].pan = std::make_unique<Attachment> (apvts, ParamIDs::voicePan (idx), v.pan);
        voiceAttachments[i].formant = std::make_unique<Attachment> (apvts, ParamIDs::voiceFormant (idx), v.formant);
        voiceAttachments[i].delay = std::make_unique<Attachment> (apvts, ParamIDs::voiceDelay (idx), v.delay);
        voiceAttachments[i].vibrato = std::make_unique<Attachment> (apvts, ParamIDs::voiceVibrato (idx), v.vibrato);
    }

    captureButton.onClick = [this]
    {
        processor.beginStemCapture();
        statusLabel.setText ("Recording stems…", juce::dontSendNotification);
        exportButton.setEnabled (true);
    };

    exportButton.onClick = [this]
    {
        exportButton.setEnabled (false);
        statusLabel.setText ("Exporting WAV…", juce::dontSendNotification);
        juce::Component::SafePointer<TunePlusAudioProcessorEditor> safeThis (this);
        processor.exportStems ([safeThis] (bool success, juce::String path)
        {
            if (safeThis == nullptr)
                return;

            safeThis->statusLabel.setText (success ? "Exported: " + path : "Export failed",
                                           juce::dontSendNotification);
            safeThis->exportButton.setEnabled (true);
        });
    };

    exportButton.setEnabled (false);
    layoutContent();

    vocalplus::ui::VocalPlusEditorShell::configureEditorWindow (*this, kContentWidth, kContentHeight);
    startTimerHz (15);
}

TunePlusAudioProcessorEditor::~TunePlusAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void TunePlusAudioProcessorEditor::layoutKnob (juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label)
{
    auto block = area.reduced (4);
    label.setBounds (block.removeFromTop (14));
    slider.setBounds (block);
}

void TunePlusAudioProcessorEditor::layoutVoiceKnobs (juce::Rectangle<int> area, VoiceControls& v)
{
    auto header = area.removeFromTop (44);
    v.title.setBounds (header.removeFromTop (18));
    v.enabled.setBounds (header.removeFromTop (22));

    auto row1 = area.removeFromTop (area.getHeight() / 2).reduced (0, 4);
    auto row2 = area.reduced (0, 4);
    const int colW1 = row1.getWidth() / 3;
    const int colW2 = row2.getWidth() / 3;

    layoutKnob (row1.removeFromLeft (colW1), v.interval, v.intervalLabel);
    layoutKnob (row1.removeFromLeft (colW1), v.level, v.levelLabel);
    layoutKnob (row1, v.pan, v.panLabel);

    layoutKnob (row2.removeFromLeft (colW2), v.formant, v.formantLabel);
    layoutKnob (row2.removeFromLeft (colW2), v.delay, v.delayLabel);
    layoutKnob (row2, v.vibrato, v.vibratoLabel);
}

void TunePlusAudioProcessorEditor::loadGenrePreset (int index)
{
    if (index < 0)
        return;

    processor.applyGenrePreset (index);
    presetDescLabel.setText (processor.getPresetManager().getGenrePreset (index).description,
                             juce::dontSendNotification);
}

void TunePlusAudioProcessorEditor::loadCompressorPreset (int index)
{
    if (index < 0)
        return;

    processor.applyCompressorPreset (index);
}

void TunePlusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (TunePlusLookAndFeel::kBackground);
}

void TunePlusAudioProcessorEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (TunePlusLookAndFeel::kBackground);

    auto drawPanel = [&g] (juce::Rectangle<int> r, const juce::String& title, juce::Colour accent)
    {
        g.setColour (TunePlusLookAndFeel::kPanel);
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (TunePlusLookAndFeel::kPanelBorder);
        g.drawRoundedRectangle (r.toFloat(), 8.0f, 1.0f);

        auto titleArea = r.reduced (14, 10).removeFromTop (18);
        g.setColour (accent);
        g.setFont (juce::FontOptions (11.0f).withStyle ("Medium"));
        g.drawText (title.toUpperCase(), titleArea, juce::Justification::centredLeft);
    };

    g.setColour (TunePlusLookAndFeel::kPanel);
    g.fillRect (0, 0, kContentWidth, 72);
    g.setColour (TunePlusLookAndFeel::kPanelBorder);
    g.drawHorizontalLine (72, 0.0f, static_cast<float> (kContentWidth));

    g.setColour (TunePlusLookAndFeel::kAccent);
    g.setFont (juce::FontOptions (24.0f).withStyle ("Bold"));
    g.drawText ("Tune+", 20, 12, 100, 30, juce::Justification::centredLeft);

    g.setColour (TunePlusLookAndFeel::kTextSecondary);
    g.setFont (juce::FontOptions (11.5f));
    g.drawText ("Professional Auto-Tune · Harmony · Vocal Compressor", 20, 42, 420, 16,
                juce::Justification::centredLeft);

    const juce::Rectangle<int> meter (kContentWidth - 320, 16, 300, 40);
    g.setColour (TunePlusLookAndFeel::kPanel);
    g.fillRoundedRectangle (meter.toFloat(), 6.0f);
    g.setColour (TunePlusLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle (meter.toFloat(), 6.0f, 1.0f);

    drawPanel ({ 16, 88, 520, 220 }, "Auto-Tune", TunePlusLookAndFeel::kAccent);
    drawPanel ({ 544, 88, 520, 220 }, "Mix", TunePlusLookAndFeel::kAccentAlt);
    drawPanel ({ 16, 318, 1048, 200 }, "Vocal Compressor", TunePlusLookAndFeel::kCompMeter);
    drawPanel ({ 16, 528, 1048, 220 }, "Harmony Voices", TunePlusLookAndFeel::kAccent);
    drawPanel ({ 16, 758, 1048, 52 }, "Stem Export", TunePlusLookAndFeel::kTextSecondary);
}

void TunePlusAudioProcessorEditor::resized()
{
    editorShell.setBounds (getLocalBounds());
    layoutContent();
}

void TunePlusAudioProcessorEditor::layoutContent()
{
    genreLabel.setBounds (140, 18, 40, 18);
    genrePresetBox.setBounds (140, 34, 200, 24);
    compressorLabel.setBounds (360, 18, 72, 18);
    compressorPresetBox.setBounds (360, 34, 200, 24);
    presetDescLabel.setBounds (140, 58, 520, 14);

    pitchLabel.setBounds (kContentWidth - 318, 20, 80, 18);
    correctionLabel.setBounds (kContentWidth - 230, 20, 110, 18);
    grLabel.setBounds (kContentWidth - 110, 20, 90, 18);

    const juce::Rectangle<int> autoPanel (16, 88, 520, 220);
    const juce::Rectangle<int> mixPanel (544, 88, 520, 220);
    const juce::Rectangle<int> compPanel (16, 318, 1048, 200);
    const juce::Rectangle<int> voicePanel (16, 528, 1048, 220);
    const juce::Rectangle<int> stemPanel (16, 758, 1048, 52);

    auto autoContent = autoPanel.reduced (16, 34);
    autoTuneToggle.setBounds (autoContent.removeFromTop (24));
    autoContent.removeFromTop (6);

    auto keyRow = autoContent.removeFromTop (26);
    keyLabel.setBounds (keyRow.removeFromLeft (28));
    keyBox.setBounds (keyRow.removeFromLeft (56).reduced (0, 2));
    keyRow.removeFromLeft (12);
    scaleLabel.setBounds (keyRow.removeFromLeft (38));
    scaleBox.setBounds (keyRow.removeFromLeft (150).reduced (0, 2));

    autoContent.removeFromTop (8);
    auto knobRow = autoContent;
    const int ak = knobRow.getWidth() / 4;
    layoutKnob (knobRow.removeFromLeft (ak), retuneSlider, retuneLabel);
    layoutKnob (knobRow.removeFromLeft (ak), toleranceSlider, toleranceLabel);
    layoutKnob (knobRow.removeFromLeft (ak), formantSlider, formantLabel);
    layoutKnob (knobRow, detuneSlider, detuneLabel);

    auto mixContent = mixPanel.reduced (16, 34);
    monoBlendToggle.setBounds (mixContent.removeFromTop (24));
    mixContent.removeFromTop (12);
    auto mixRow = mixContent;
    const int mk = mixRow.getWidth() / 3;
    layoutKnob (mixRow.removeFromLeft (mk), drySlider, dryLabel);
    layoutKnob (mixRow.removeFromLeft (mk), wetSlider, wetLabel);
    layoutKnob (mixRow, widthSlider, widthLabel);

    auto compContent = compPanel.reduced (16, 34);
    compToggle.setBounds (compContent.removeFromTop (24));
    compContent.removeFromTop (8);
    auto compRow = compContent;
    const int ck = compRow.getWidth() / 7;
    layoutKnob (compRow.removeFromLeft (ck), compThresholdSlider, compThresholdLabel);
    layoutKnob (compRow.removeFromLeft (ck), compRatioSlider, compRatioLabel);
    layoutKnob (compRow.removeFromLeft (ck), compAttackSlider, compAttackLabel);
    layoutKnob (compRow.removeFromLeft (ck), compReleaseSlider, compReleaseLabel);
    layoutKnob (compRow.removeFromLeft (ck), compMakeupSlider, compMakeupLabel);
    layoutKnob (compRow.removeFromLeft (ck), compHighPassSlider, compHighPassLabel);
    layoutKnob (compRow, compMixSlider, compMixLabel);

    auto voiceContent = voicePanel.reduced (16, 34);
    const int voiceColW = voiceContent.getWidth() / 3;

    for (auto& voice : voices)
    {
        auto col = voiceContent.removeFromLeft (voiceColW).reduced (6, 0);
        layoutVoiceKnobs (col, voice);
    }

    auto stemContent = stemPanel.reduced (16, 10);
    captureButton.setBounds (stemContent.removeFromLeft (120).reduced (0, 2));
    exportButton.setBounds (stemContent.removeFromLeft (120).reduced (0, 2));
    statusLabel.setBounds (stemContent.reduced (8, 2));
}

void TunePlusAudioProcessorEditor::timerCallback()
{
    const float hz = processor.getDetectedPitchHz();
    const float correction = processor.getCorrectionAmount();
    const float gr = processor.getCompressorGainReductionDb();

    if (hz > 0.0f)
    {
        const int midi = static_cast<int> (std::round (69.0f + 12.0f * std::log2 (hz / 440.0f)));
        const juce::String noteName = TunePlusPresetManager::getNoteNames()[juce::jlimit (0, 11, midi % 12)];
        const int octave = midi / 12 - 1;
        pitchLabel.setText (noteName + juce::String (octave), juce::dontSendNotification);
        correctionLabel.setText ("Correction " + juce::String (correction * 100.0f, 0) + " ct",
                                 juce::dontSendNotification);
    }
    else
    {
        pitchLabel.setText ("—", juce::dontSendNotification);
        correctionLabel.setText ("No pitch", juce::dontSendNotification);
    }

    grLabel.setText ("GR " + juce::String (gr, 1) + " dB", juce::dontSendNotification);
    captureButton.setButtonText (processor.isStemCapturing() ? "Recording…" : "Record Stems");
}

} // namespace tuneplus
