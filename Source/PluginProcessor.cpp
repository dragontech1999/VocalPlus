#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "DSP/ParameterHelpers.h"
#include "DSP/AudioSafety.h"

namespace vocalplus
{

using namespace ParamIDs;

VocalPlusAudioProcessor::VocalPlusAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "VocalPlus", createParameterLayout())
{
    applyPreset (0);
}

VocalPlusAudioProcessor::~VocalPlusAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout VocalPlusAudioProcessor::createParameterLayout()
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
        ParamIDs::rootNote, "Key", PresetManager::getNoteNames(), 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        ParamIDs::scaleType, "Scale", PresetManager::getScaleNames(), 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::detune, "Detune",
        bipolarRange (-50.0f, 50.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::dryLevel, "Dry",
        linearRange (0.0f, 1.0f, 0.01f), 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::wetLevel, "Wet",
        linearRange (0.0f, 1.5f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::stereoWidth, "Width",
        linearRange (0.0f, 2.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (ParamIDs::blendToMono, "Blend Mono", false));

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

void VocalPlusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    stemExporter.prepare (sampleRate, getTotalNumOutputChannels());
    engine.setSettings (readSettingsFromParameters());
}

void VocalPlusAudioProcessor::releaseResources()
{
    stemExporter.reset();
}

bool VocalPlusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

HarmonyEngineSettings VocalPlusAudioProcessor::readSettingsFromParameters() const
{
    HarmonyEngineSettings s;

    s.autoTune.enabled = readBoolParam (apvts, ParamIDs::autoTuneEnabled);
    s.autoTune.retuneSpeed = readFloatParam (apvts, ParamIDs::retuneSpeed);
    s.autoTune.toleranceCents = readFloatParam (apvts, ParamIDs::tolerance);
    s.autoTune.formantPreserve = readFloatParam (apvts, ParamIDs::formantPreserve);
    s.autoTune.rootNote = readChoiceParam (apvts, ParamIDs::rootNote);
    s.autoTune.scale = static_cast<ScaleType> (readChoiceParam (apvts, ParamIDs::scaleType));
    s.autoTune.detuneCents = readFloatParam (apvts, ParamIDs::detune);
    s.dryLevel = readFloatParam (apvts, ParamIDs::dryLevel);
    s.wetLevel = readFloatParam (apvts, ParamIDs::wetLevel);
    s.stereoWidth = readFloatParam (apvts, ParamIDs::stereoWidth);
    s.blendToMono = readBoolParam (apvts, ParamIDs::blendToMono);

    for (int i = 0; i < 3; ++i)
    {
        auto& v = s.voices[static_cast<size_t> (i)];
        v.enabled = readBoolParam (apvts, ParamIDs::voiceEnabled (i));
        v.intervalSemitones = readIntParam (apvts, ParamIDs::voiceInterval (i));
        v.level = readFloatParam (apvts, ParamIDs::voiceLevel (i));
        v.pan = readFloatParam (apvts, ParamIDs::voicePan (i));
        v.formantShift = readFloatParam (apvts, ParamIDs::voiceFormant (i));
        v.delayMs = readFloatParam (apvts, ParamIDs::voiceDelay (i));
        v.vibratoDepth = readFloatParam (apvts, ParamIDs::voiceVibrato (i));
    }

    return s;
}

void VocalPlusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    engine.setSettings (readSettingsFromParameters());
    engine.process (buffer);

    if (stemExporter.isCapturing())
    {
        engine.getStemOutputs (stemDry, stemCorrected, stemHarmonies, stemBlend);
        std::vector<juce::AudioBuffer<float>> harmonies (stemHarmonies.begin(), stemHarmonies.end());
        stemExporter.captureBlock (stemDry, stemCorrected, harmonies, stemBlend);
    }

    sanitizeBuffer (buffer);
}

juce::AudioProcessorEditor* VocalPlusAudioProcessor::createEditor()
{
    return new VocalPlusAudioProcessorEditor (*this);
}

int VocalPlusAudioProcessor::getNumPrograms()
{
    return presetManager.getNumPresets();
}

int VocalPlusAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void VocalPlusAudioProcessor::setCurrentProgram (int index)
{
    applyPreset (index);
}

const juce::String VocalPlusAudioProcessor::getProgramName (int index)
{
    return presetManager.getPreset (index).name;
}

void VocalPlusAudioProcessor::applyPreset (int index)
{
    index = juce::jlimit (0, presetManager.getNumPresets() - 1, index);
    currentProgram = index;
    const auto& preset = presetManager.getPreset (index);
    const auto& s = preset.settings;

    apvts.getParameter (ParamIDs::autoTuneEnabled)->setValueNotifyingHost (s.autoTune.enabled ? 1.0f : 0.0f);
    apvts.getParameter (ParamIDs::retuneSpeed)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::retuneSpeed).convertTo0to1 (s.autoTune.retuneSpeed));
    apvts.getParameter (ParamIDs::tolerance)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::tolerance).convertTo0to1 (s.autoTune.toleranceCents));
    apvts.getParameter (ParamIDs::formantPreserve)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::formantPreserve).convertTo0to1 (s.autoTune.formantPreserve));
    apvts.getParameter (ParamIDs::rootNote)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::rootNote).convertTo0to1 (static_cast<float> (s.autoTune.rootNote)));
    apvts.getParameter (ParamIDs::scaleType)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::scaleType).convertTo0to1 (static_cast<float> (s.autoTune.scale)));
    apvts.getParameter (ParamIDs::detune)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::detune).convertTo0to1 (s.autoTune.detuneCents));
    apvts.getParameter (ParamIDs::dryLevel)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::dryLevel).convertTo0to1 (s.dryLevel));
    apvts.getParameter (ParamIDs::wetLevel)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::wetLevel).convertTo0to1 (s.wetLevel));
    apvts.getParameter (ParamIDs::stereoWidth)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::stereoWidth).convertTo0to1 (s.stereoWidth));
    apvts.getParameter (ParamIDs::blendToMono)->setValueNotifyingHost (s.blendToMono ? 1.0f : 0.0f);

    for (int i = 0; i < 3; ++i)
    {
        const auto& v = s.voices[static_cast<size_t> (i)];
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

void VocalPlusAudioProcessor::beginStemCapture()
{
    stemExporter.beginCapture();
}

void VocalPlusAudioProcessor::exportStems (std::function<void (bool, juce::String)> callback)
{
    stemExporter.exportStemsAsync (std::move (callback));
}

void VocalPlusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("currentProgram", currentProgram);
        copyXmlToBinary (*xml, destData);
    }
}

void VocalPlusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        currentProgram = xml->getIntAttribute ("currentProgram", 0);
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

} // namespace vocalplus

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new vocalplus::VocalPlusAudioProcessor();
}
