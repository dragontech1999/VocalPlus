#include "PluginEditor.h"
#include "ParameterIDs.h"

namespace vocalplus
{

using namespace ParamIDs;

class VocalPlusAudioProcessorEditor::ScrollableContent : public juce::Component
{
public:
    explicit ScrollableContent (VocalPlusAudioProcessorEditor& ownerIn) : owner (ownerIn) {}

    void paint (juce::Graphics& g) override { owner.paintContent (g); }
    void resized() override { owner.layoutContent(); }

private:
    VocalPlusAudioProcessorEditor& owner;
};

namespace
{
    void styleLabel (juce::Label& label, bool secondary = false)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId,
                         secondary ? VocalPlusLookAndFeel::kTextSecondary
                                   : VocalPlusLookAndFeel::kTextPrimary);
        label.setFont (juce::FontOptions (secondary ? 10.5f : 11.0f));
    }

    void styleSlider (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 16);
        slider.setColour (juce::Slider::textBoxTextColourId, VocalPlusLookAndFeel::kTextSecondary);
        slider.setScrollWheelEnabled (true);
    }
}

VocalPlusAudioProcessorEditor::VocalPlusAudioProcessorEditor (VocalPlusAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      editorShell (*this, VocalPlusLookAndFeel::kAccent)
{
    setLookAndFeel (&lookAndFeel);

    scrollContent = std::make_unique<ScrollableContent> (*this);
    addAndMakeVisible (editorShell);
    editorShell.setScrollContent (scrollContent.get(), kContentWidth, kContentHeight);

    auto addToContent = [this] (juce::Component& c) { scrollContent->addAndMakeVisible (c); };

    for (int i = 0; i < processor.getPresetManager().getNumPresets(); ++i)
        presetBox.addItem (processor.getPresetManager().getPreset (i).name, i + 1);

    presetBox.setSelectedId (processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { loadPreset (presetBox.getSelectedItemIndex()); };

    styleLabel (presetLabel, true);
    styleLabel (pitchLabel);
    styleLabel (correctionLabel, true);
    styleLabel (statusLabel, true);
    styleLabel (keyLabel, true);
    styleLabel (scaleLabel, true);

    pitchLabel.setText ("—", juce::dontSendNotification);
    correctionLabel.setText ("Correction 0 ct", juce::dontSendNotification);
    statusLabel.setText ("Ready", juce::dontSendNotification);

    addToContent (presetBox);
    addToContent (presetLabel);
    addToContent (captureButton);
    addToContent (exportButton);
    addToContent (statusLabel);
    addToContent (pitchLabel);
    addToContent (correctionLabel);

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

    for (int i = 0; i < PresetManager::getNoteNames().size(); ++i)
        keyBox.addItem (PresetManager::getNoteNames()[i], i + 1);
    for (int i = 0; i < PresetManager::getScaleNames().size(); ++i)
        scaleBox.addItem (PresetManager::getScaleNames()[i], i + 1);

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
        juce::Component::SafePointer<VocalPlusAudioProcessorEditor> safeThis (this);
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
    ui::VocalPlusEditorShell::configureEditorWindow (*this, kContentWidth, kContentHeight);
    startTimerHz (15);
}

VocalPlusAudioProcessorEditor::~VocalPlusAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void VocalPlusAudioProcessorEditor::layoutKnob (juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label)
{
    auto block = area.reduced (4);
    label.setBounds (block.removeFromTop (14));
    slider.setBounds (block);
}

void VocalPlusAudioProcessorEditor::layoutVoiceKnobs (juce::Rectangle<int> area, VoiceControls& v)
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

void VocalPlusAudioProcessorEditor::loadPreset (int index)
{
    if (index < 0)
        return;

    processor.applyPreset (index);
    processor.setCurrentProgram (index);
}

void VocalPlusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (VocalPlusLookAndFeel::kBackground);
}

void VocalPlusAudioProcessorEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (VocalPlusLookAndFeel::kBackground);

    auto drawPanel = [&g] (juce::Rectangle<int> r, const juce::String& title)
    {
        g.setColour (VocalPlusLookAndFeel::kPanel);
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (VocalPlusLookAndFeel::kPanelBorder);
        g.drawRoundedRectangle (r.toFloat(), 8.0f, 1.0f);

        auto titleArea = r.reduced (14, 10).removeFromTop (18);
        g.setColour (VocalPlusLookAndFeel::kTextSecondary);
        g.setFont (juce::FontOptions (11.0f).withStyle ("Medium"));
        g.drawText (title.toUpperCase(), titleArea, juce::Justification::centredLeft);
    };

    g.setColour (VocalPlusLookAndFeel::kPanel);
    g.fillRect (0, 0, kContentWidth, 64);
    g.setColour (VocalPlusLookAndFeel::kPanelBorder);
    g.drawHorizontalLine (64, 0.0f, static_cast<float> (kContentWidth));

    g.setColour (VocalPlusLookAndFeel::kTextPrimary);
    g.setFont (juce::FontOptions (22.0f).withStyle ("Bold"));
    g.drawText ("Vocal+", 20, 14, 120, 28, juce::Justification::centredLeft);

    g.setColour (VocalPlusLookAndFeel::kTextSecondary);
    g.setFont (juce::FontOptions (11.5f));
    g.drawText ("Professional Vocal Harmony & Pitch Correction · by Neeberman", 20, 40, 420, 16,
                juce::Justification::centredLeft);

    const juce::Rectangle<int> meter (kContentWidth - 250, 14, 230, 36);
    g.setColour (VocalPlusLookAndFeel::kPanel);
    g.fillRoundedRectangle (meter.toFloat(), 6.0f);
    g.setColour (VocalPlusLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle (meter.toFloat(), 6.0f, 1.0f);

    drawPanel ({ 16, 80, 490, 230 }, "Auto-Tune");
    drawPanel ({ 514, 80, 490, 230 }, "Mix");
    drawPanel ({ 16, 322, 988, 340 }, "Harmony Voices");
    drawPanel ({ 16, 674, 988, 70 }, "Stem Export");
}

void VocalPlusAudioProcessorEditor::resized()
{
    editorShell.setBounds (getLocalBounds());
    layoutContent();
}

void VocalPlusAudioProcessorEditor::layoutContent()
{
    presetLabel.setBounds (140, 20, 44, 18);
    presetBox.setBounds (140, 36, 220, 24);

    pitchLabel.setBounds (kContentWidth - 248, 18, 80, 18);
    correctionLabel.setBounds (kContentWidth - 160, 18, 140, 18);

    const juce::Rectangle<int> autoPanel (16, 80, 490, 230);
    const juce::Rectangle<int> mixPanel (514, 80, 490, 230);
    const juce::Rectangle<int> voicePanel (16, 322, 988, 340);
    const juce::Rectangle<int> stemPanel (16, 674, 988, 70);

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

    auto voiceContent = voicePanel.reduced (16, 34);
    const int voiceColW = voiceContent.getWidth() / 3;

    for (auto& voice : voices)
    {
        auto col = voiceContent.removeFromLeft (voiceColW).reduced (6, 0);
        layoutVoiceKnobs (col, voice);
    }

    auto stemContent = stemPanel.reduced (16, 22);
    captureButton.setBounds (stemContent.removeFromLeft (120).reduced (0, 4));
    exportButton.setBounds (stemContent.removeFromLeft (120).reduced (0, 4));
    statusLabel.setBounds (stemContent.reduced (8, 6));
}

void VocalPlusAudioProcessorEditor::timerCallback()
{
    const float hz = processor.getDetectedPitchHz();
    const float correction = processor.getCorrectionAmount();

    if (hz > 0.0f)
    {
        const int midi = static_cast<int> (std::round (69.0f + 12.0f * std::log2 (hz / 440.0f)));
        const juce::String noteName = PresetManager::getNoteNames()[juce::jlimit (0, 11, midi % 12)];
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

    captureButton.setButtonText (processor.isStemCapturing() ? "Recording…" : "Record Stems");
}

} // namespace vocalplus
