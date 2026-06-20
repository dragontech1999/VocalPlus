#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "../DSP/ParameterHelpers.h"
#include "../DSP/AudioSafety.h"

namespace tuneplus
{

using namespace ParamIDs;
using namespace vocalplus;

TunePlusAudioProcessor::TunePlusAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "TunePlus", createParameterLayout())
{
    applyGenrePreset (0);
}

TunePlusAudioProcessor::~TunePlusAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout TunePlusAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterBool> (ParamIDs::autoTuneEnabled, "Auto-Tune", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::retuneSpeed, "Retune Speed",
        linearRange (0.01f, 1.0f, 0.01f), 0.85f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::tolerance, "Tolerance",
        linearRange (0.0f, 100.0f, 1.0f), 25.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::formantPreserve, "Formant Preserve",
        linearRange (0.0f, 1.0f, 0.01f), 0.75f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        ParamIDs::rootNote, "Key", TunePlusPresetManager::getNoteNames(), 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        ParamIDs::scaleType, "Scale", TunePlusPresetManager::getScaleNames(), 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::detune, "Detune",
        bipolarRange (-50.0f, 50.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::dryLevel, "Dry",
        linearRange (0.0f, 1.0f, 0.01f), 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::wetLevel, "Wet",
        linearRange (0.0f, 1.5f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::stereoWidth, "Width",
        linearRange (0.0f, 2.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (ParamIDs::blendToMono, "Blend Mono", false));

    params.push_back (std::make_unique<juce::AudioParameterBool> (ParamIDs::compEnabled, "Compressor", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::compThreshold, "Threshold",
        rangedRange (-60.0f, 0.0f, 0.1f), -18.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::compRatio, "Ratio",
        rangedRange (1.0f, 20.0f, 0.1f), 3.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::compAttack, "Attack",
        rangedRange (0.1f, 100.0f, 0.1f), 12.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::compRelease, "Release",
        rangedRange (10.0f, 1000.0f, 1.0f), 120.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::compMakeup, "Makeup",
        linearRange (0.0f, 24.0f, 0.1f), 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::compHighPass, "High Pass",
        rangedRange (20.0f, 300.0f, 1.0f), 80.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::compMix, "Comp Mix",
        linearRange (0.0f, 1.0f, 0.01f), 1.0f));

    for (int i = 0; i < 3; ++i)
    {
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            ParamIDs::voiceEnabled (i), "Voice " + juce::String (i + 1), i == 0));
        params.push_back (std::make_unique<juce::AudioParameterInt> (
            ParamIDs::voiceInterval (i), "Interval " + juce::String (i + 1), -24, 24, 4));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            ParamIDs::voiceLevel (i), "Level " + juce::String (i + 1),
            linearRange (0.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            ParamIDs::voicePan (i), "Pan " + juce::String (i + 1),
            bipolarRange (-1.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            ParamIDs::voiceFormant (i), "Formant " + juce::String (i + 1),
            linearRange (0.5f, 1.5f, 0.01f), 1.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            ParamIDs::voiceDelay (i), "Delay " + juce::String (i + 1),
            linearRange (0.0f, 80.0f, 0.1f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            ParamIDs::voiceVibrato (i), "Vibrato " + juce::String (i + 1),
            linearRange (0.0f, 0.15f, 0.001f), 0.0f));
    }

    return { params.begin(), params.end() };
}

void TunePlusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    stemExporter.prepare (sampleRate, getTotalNumOutputChannels());
    engine.setSettings (readSettingsFromParameters());
}

void TunePlusAudioProcessor::releaseResources()
{
    stemExporter.reset();
}

bool TunePlusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

TunePlusSettings TunePlusAudioProcessor::readSettingsFromParameters() const
{
    TunePlusSettings s;
    auto& h = s.harmony;
    auto& c = s.compressor;

    h.autoTune.enabled = readBoolParam (apvts, ParamIDs::autoTuneEnabled);
    h.autoTune.retuneSpeed = readFloatParam (apvts, ParamIDs::retuneSpeed);
    h.autoTune.toleranceCents = readFloatParam (apvts, ParamIDs::tolerance);
    h.autoTune.formantPreserve = readFloatParam (apvts, ParamIDs::formantPreserve);
    h.autoTune.rootNote = readChoiceParam (apvts, ParamIDs::rootNote);
    h.autoTune.scale = static_cast<vocalplus::ScaleType> (readChoiceParam (apvts, ParamIDs::scaleType));
    h.autoTune.detuneCents = readFloatParam (apvts, ParamIDs::detune);
    h.dryLevel = readFloatParam (apvts, ParamIDs::dryLevel);
    h.wetLevel = readFloatParam (apvts, ParamIDs::wetLevel);
    h.stereoWidth = readFloatParam (apvts, ParamIDs::stereoWidth);
    h.blendToMono = readBoolParam (apvts, ParamIDs::blendToMono);

    for (int i = 0; i < 3; ++i)
    {
        auto& v = h.voices[static_cast<size_t> (i)];
        v.enabled = readBoolParam (apvts, ParamIDs::voiceEnabled (i));
        v.intervalSemitones = readIntParam (apvts, ParamIDs::voiceInterval (i));
        v.level = readFloatParam (apvts, ParamIDs::voiceLevel (i));
        v.pan = readFloatParam (apvts, ParamIDs::voicePan (i));
        v.formantShift = readFloatParam (apvts, ParamIDs::voiceFormant (i));
        v.delayMs = readFloatParam (apvts, ParamIDs::voiceDelay (i));
        v.vibratoDepth = readFloatParam (apvts, ParamIDs::voiceVibrato (i));
    }

    c.enabled = readBoolParam (apvts, ParamIDs::compEnabled);
    c.thresholdDb = readFloatParam (apvts, ParamIDs::compThreshold);
    c.ratio = readFloatParam (apvts, ParamIDs::compRatio);
    c.attackMs = readFloatParam (apvts, ParamIDs::compAttack);
    c.releaseMs = readFloatParam (apvts, ParamIDs::compRelease);
    c.makeupDb = readFloatParam (apvts, ParamIDs::compMakeup);
    c.highPassHz = readFloatParam (apvts, ParamIDs::compHighPass);
    c.mix = readFloatParam (apvts, ParamIDs::compMix);

    return s;
}

void TunePlusAudioProcessor::applyCompressorSettings (const CompressorSettings& c)
{
    apvts.getParameter (ParamIDs::compEnabled)->setValueNotifyingHost (c.enabled ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::compThreshold)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::compThreshold).convertTo0to1 (c.thresholdDb));
    apvts.getParameter (ParamIDs::compRatio)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::compRatio).convertTo0to1 (c.ratio));
    apvts.getParameter (ParamIDs::compAttack)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::compAttack).convertTo0to1 (c.attackMs));
    apvts.getParameter (ParamIDs::compRelease)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::compRelease).convertTo0to1 (c.releaseMs));
    apvts.getParameter (ParamIDs::compMakeup)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::compMakeup).convertTo0to1 (c.makeupDb));
    apvts.getParameter (ParamIDs::compHighPass)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::compHighPass).convertTo0to1 (c.highPassHz));
    apvts.getParameter (ParamIDs::compMix)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::compMix).convertTo0to1 (c.mix));
}

void TunePlusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (getSampleRate() <= 0.0 || buffer.getNumSamples() <= 0)
        return;

    engine.setSettings (readSettingsFromParameters());
    engine.process (buffer);

    if (stemExporter.isCapturing())
    {
        engine.getStemOutputs (stemDry, stemCorrected, stemHarmonies, stemBlend);
        std::vector<juce::AudioBuffer<float>> harmonies (stemHarmonies.begin(), stemHarmonies.end());
        stemExporter.captureBlock (stemDry, stemCorrected, harmonies, stemBlend);
    }

    vocalplus::sanitizeBuffer (buffer);
}

juce::AudioProcessorEditor* TunePlusAudioProcessor::createEditor()
{
    return new TunePlusAudioProcessorEditor (*this);
}

int TunePlusAudioProcessor::getNumPrograms()
{
    return presetManager.getNumGenrePresets();
}

int TunePlusAudioProcessor::getCurrentProgram()
{
    return currentGenreProgram;
}

void TunePlusAudioProcessor::setCurrentProgram (int index)
{
    applyGenrePreset (index);
}

const juce::String TunePlusAudioProcessor::getProgramName (int index)
{
    return presetManager.getGenrePreset (index).name;
}

void TunePlusAudioProcessor::applyGenrePreset (int index)
{
    index = juce::jlimit (0, presetManager.getNumGenrePresets() - 1, index);
    currentGenreProgram = index;
    const auto& preset = presetManager.getGenrePreset (index);
    const auto& settings = preset.settings;

    currentCompressorProgram = preset.linkedCompressorIndex;

    apvts.getParameter (ParamIDs::autoTuneEnabled)->setValueNotifyingHost (settings.harmony.autoTune.enabled ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::retuneSpeed)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::retuneSpeed).convertTo0to1 (settings.harmony.autoTune.retuneSpeed));
    apvts.getParameter (ParamIDs::tolerance)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::tolerance).convertTo0to1 (settings.harmony.autoTune.toleranceCents));
    apvts.getParameter (ParamIDs::formantPreserve)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::formantPreserve).convertTo0to1 (settings.harmony.autoTune.formantPreserve));
    apvts.getParameter (ParamIDs::rootNote)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::rootNote).convertTo0to1 (static_cast<float> (settings.harmony.autoTune.rootNote)));
    apvts.getParameter (ParamIDs::scaleType)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::scaleType).convertTo0to1 (static_cast<float> (settings.harmony.autoTune.scale)));
    apvts.getParameter (ParamIDs::detune)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::detune).convertTo0to1 (settings.harmony.autoTune.detuneCents));
    apvts.getParameter (ParamIDs::dryLevel)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::dryLevel).convertTo0to1 (settings.harmony.dryLevel));
    apvts.getParameter (ParamIDs::wetLevel)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::wetLevel).convertTo0to1 (settings.harmony.wetLevel));
    apvts.getParameter (ParamIDs::stereoWidth)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::stereoWidth).convertTo0to1 (settings.harmony.stereoWidth));
    apvts.getParameter (ParamIDs::blendToMono)->setValueNotifyingHost (settings.harmony.blendToMono ? 1.0f : 0.0f);

    applyCompressorSettings (settings.compressor);

    for (int i = 0; i < 3; ++i)
    {
        const auto& v = settings.harmony.voices[static_cast<size_t> (i)];
        apvts.getParameter (ParamIDs::voiceEnabled (i))->setValueNotifyingHost (v.enabled ? 1.0f : 0.0f);
        if (auto* interval = dynamic_cast<juce::AudioParameterInt*> (apvts.getParameter (ParamIDs::voiceInterval (i))))
            interval->setValueNotifyingHost (interval->convertTo0to1 (v.intervalSemitones));
        apvts.getParameter (ParamIDs::voiceLevel (i))->setValueNotifyingHost (
            apvts.getParameterRange (ParamIDs::voiceLevel (i)).convertTo0to1 (v.level));
        apvts.getParameter (ParamIDs::voicePan (i))->setValueNotifyingHost (
            apvts.getParameterRange (ParamIDs::voicePan (i)).convertTo0to1 (v.pan));
        apvts.getParameter (ParamIDs::voiceFormant (i))->setValueNotifyingHost (
            apvts.getParameterRange (ParamIDs::voiceFormant (i)).convertTo0to1 (v.formantShift));
        apvts.getParameter (ParamIDs::voiceDelay (i))->setValueNotifyingHost (
            apvts.getParameterRange (ParamIDs::voiceDelay (i)).convertTo0to1 (v.delayMs));
        apvts.getParameter (ParamIDs::voiceVibrato (i))->setValueNotifyingHost (
            apvts.getParameterRange (ParamIDs::voiceVibrato (i)).convertTo0to1 (v.vibratoDepth));
    }
}

void TunePlusAudioProcessor::applyCompressorPreset (int index)
{
    index = juce::jlimit (0, presetManager.getNumCompressorPresets() - 1, index);
    currentCompressorProgram = index;
    applyCompressorSettings (presetManager.getCompressorPreset (index).settings);
}

void TunePlusAudioProcessor::beginStemCapture()
{
    stemExporter.beginCapture();
}

void TunePlusAudioProcessor::exportStems (std::function<void (bool, juce::String)> callback)
{
    stemExporter.exportStemsAsync (std::move (callback));
}

void TunePlusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("currentGenreProgram", currentGenreProgram);
        xml->setAttribute ("currentCompressorProgram", currentCompressorProgram);
        copyXmlToBinary (*xml, destData);
    }
}

void TunePlusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        currentGenreProgram = xml->getIntAttribute ("currentGenreProgram", 0);
        currentCompressorProgram = xml->getIntAttribute ("currentCompressorProgram", 0);
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

} // namespace tuneplus

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new tuneplus::TunePlusAudioProcessor();
}
