#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "DSP/VoxPlusFxModes.h"
#include "../DSP/ParameterHelpers.h"
#include "../DSP/AudioSafety.h"

namespace voxplus
{

using namespace ParamIDs;
using namespace vocalplus;

VoxPlusAudioProcessor::VoxPlusAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "VoxPlus", createParameterLayout())
{
    applyPreset (0);
}

VoxPlusAudioProcessor::~VoxPlusAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout VoxPlusAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::inputGain, "Input",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::outputGain, "Output",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::globalMix, "Mix",
        linearRange (0.0f, 1.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::autoLevel, "Auto Level",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::lowCut, "Low Cut",
        rangedRange (20.0f, 400.0f, 1.0f), 80.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::highCut, "High Cut",
        rangedRange (2000.0f, 20000.0f, 10.0f), 16000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::doubling, "Doubling",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::focusEQ, "Focus",
        linearRange (0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::superGlue, "Super Glue",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (ParamIDs::voiceBypass, "Voice Bypass", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::voiceMix, "Voice Mix",
        linearRange (0.0f, 1.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::pitchShift, "Pitch",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::formant, "Formant",
        rangedRange (0.5f, 1.5f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::unison, "Unison",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::correction, "Correction",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        ParamIDs::rootNote, "Key", getNoteNames(), 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        ParamIDs::tuneScale, "Scale", getTuneScaleNames(), 1));

    for (int i = 0; i < numFxModules; ++i)
    {
        const auto idx = juce::String (i);
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            fxEnabled (i), kFxModules[static_cast<size_t> (i)].title, true));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            fxMode (i), kFxModules[static_cast<size_t> (i)].title + " Mode",
            kFxModules[static_cast<size_t> (i)].modes, 0));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            fxAmount (i), kFxModules[static_cast<size_t> (i)].title + " Amount",
            linearRange (0.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            fxTone (i), kFxModules[static_cast<size_t> (i)].title + " Tone",
            linearRange (0.0f, 1.0f, 0.01f), 0.5f));
    }

    return { params.begin(), params.end() };
}

void VoxPlusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    engine.setSettings (readSettingsFromParameters());
}

void VoxPlusAudioProcessor::releaseResources() {}

bool VoxPlusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

VoxPlusSettings VoxPlusAudioProcessor::readSettingsFromParameters() const
{
    VoxPlusSettings s;

    s.inputGainDb = readFloatParam (apvts, ParamIDs::inputGain);
    s.outputGainDb = readFloatParam (apvts, ParamIDs::outputGain);
    s.globalMix = readFloatParam (apvts, ParamIDs::globalMix);
    s.autoLevel = readFloatParam (apvts, ParamIDs::autoLevel);
    s.lowCutHz = readFloatParam (apvts, ParamIDs::lowCut);
    s.highCutHz = readFloatParam (apvts, ParamIDs::highCut);
    s.doubling = readFloatParam (apvts, ParamIDs::doubling);
    s.focusEQ = readFloatParam (apvts, ParamIDs::focusEQ);
    s.superGlue = readFloatParam (apvts, ParamIDs::superGlue);

    s.voiceBypass = readBoolParam (apvts, ParamIDs::voiceBypass);
    s.voiceMix = readFloatParam (apvts, ParamIDs::voiceMix);
    s.pitchShiftSemitones = readFloatParam (apvts, ParamIDs::pitchShift);
    s.formant = readFloatParam (apvts, ParamIDs::formant);
    s.unison = readFloatParam (apvts, ParamIDs::unison);
    s.correction = readFloatParam (apvts, ParamIDs::correction);
    s.rootNote = readChoiceParam (apvts, ParamIDs::rootNote);
    s.tuneScale = readChoiceParam (apvts, ParamIDs::tuneScale);

    for (int i = 0; i < numFxModules; ++i)
    {
        auto& fx = s.fx[static_cast<size_t> (i)];
        fx.enabled = readBoolParam (apvts, fxEnabled (i));
        fx.mode = readChoiceParam (apvts, fxMode (i));
        fx.amount = readFloatParam (apvts, fxAmount (i));
        fx.tone = readFloatParam (apvts, fxTone (i));
    }

    return s;
}

void VoxPlusAudioProcessor::applySettingsToParameters (const VoxPlusSettings& s)
{
    auto setFloat = [this] (const juce::String& id, float value)
    {
        apvts.getParameter (id)->setValueNotifyingHost (
            apvts.getParameterRange (id).convertTo0to1 (value));
    };

    setFloat (ParamIDs::inputGain, s.inputGainDb);
    setFloat (ParamIDs::outputGain, s.outputGainDb);
    setFloat (ParamIDs::globalMix, s.globalMix);
    setFloat (ParamIDs::autoLevel, s.autoLevel);
    setFloat (ParamIDs::lowCut, s.lowCutHz);
    setFloat (ParamIDs::highCut, s.highCutHz);
    setFloat (ParamIDs::doubling, s.doubling);
    setFloat (ParamIDs::focusEQ, s.focusEQ);
    setFloat (ParamIDs::superGlue, s.superGlue);

    apvts.getParameter (ParamIDs::voiceBypass)->setValueNotifyingHost (s.voiceBypass ? 1.0f : 0.0f);
    setFloat (ParamIDs::voiceMix, s.voiceMix);
    setFloat (ParamIDs::pitchShift, s.pitchShiftSemitones);
    setFloat (ParamIDs::formant, s.formant);
    setFloat (ParamIDs::unison, s.unison);
    setFloat (ParamIDs::correction, s.correction);
    apvts.getParameter (ParamIDs::rootNote)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::rootNote).convertTo0to1 (static_cast<float> (s.rootNote)));
    apvts.getParameter (ParamIDs::tuneScale)->setValueNotifyingHost (
        apvts.getParameterRange (ParamIDs::tuneScale).convertTo0to1 (static_cast<float> (s.tuneScale)));

    for (int i = 0; i < numFxModules; ++i)
    {
        apvts.getParameter (fxEnabled (i))->setValueNotifyingHost (s.fx[static_cast<size_t> (i)].enabled ? 1.0f : 0.0f);
        apvts.getParameter (fxMode (i))->setValueNotifyingHost (
            apvts.getParameterRange (fxMode (i)).convertTo0to1 (static_cast<float> (s.fx[static_cast<size_t> (i)].mode)));
        setFloat (fxAmount (i), s.fx[static_cast<size_t> (i)].amount);
        setFloat (fxTone (i), s.fx[static_cast<size_t> (i)].tone);
    }
}

void VoxPlusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (getSampleRate() <= 0.0 || buffer.getNumSamples() <= 0)
        return;

    engine.setSettings (readSettingsFromParameters());
    engine.process (buffer);
    vocalplus::sanitizeBuffer (buffer);
}

juce::AudioProcessorEditor* VoxPlusAudioProcessor::createEditor()
{
    return new VoxPlusAudioProcessorEditor (*this);
}

int VoxPlusAudioProcessor::getNumPrograms()
{
    return presetManager.getNumPresets();
}

int VoxPlusAudioProcessor::getCurrentProgram()
{
    return currentPreset;
}

void VoxPlusAudioProcessor::setCurrentProgram (int index)
{
    applyPreset (index);
}

const juce::String VoxPlusAudioProcessor::getProgramName (int index)
{
    return presetManager.getPreset (index).name;
}

void VoxPlusAudioProcessor::applyPreset (int index)
{
    index = juce::jlimit (0, presetManager.getNumPresets() - 1, index);
    currentPreset = index;
    applySettingsToParameters (presetManager.getPreset (index).settings);
}

void VoxPlusAudioProcessor::randomizeSettings()
{
    VoxPlusSettings s;

    s.inputGainDb = random.nextFloat() * 6.0f - 2.0f;
    s.outputGainDb = random.nextFloat() * 4.0f - 1.0f;
    s.globalMix = 0.6f + random.nextFloat() * 0.4f;
    s.autoLevel = random.nextFloat() * 0.5f;
    s.lowCutHz = 40.0f + random.nextFloat() * 120.0f;
    s.highCutHz = 8000.0f + random.nextFloat() * 10000.0f;
    s.doubling = random.nextFloat() * 0.6f;
    s.focusEQ = random.nextFloat();
    s.superGlue = random.nextFloat() * 0.7f;

    s.voiceBypass = random.nextFloat() > 0.85f;
    s.voiceMix = 0.5f + random.nextFloat() * 0.5f;
    s.pitchShiftSemitones = random.nextFloat() * 16.0f - 8.0f;
    s.formant = 0.7f + random.nextFloat() * 0.6f;
    s.unison = random.nextFloat() * 0.8f;
    s.correction = random.nextFloat();
    s.rootNote = random.nextInt (12);
    s.tuneScale = random.nextInt (5);

    for (auto& fx : s.fx)
    {
        fx.enabled = random.nextFloat() > 0.15f;
        fx.mode = random.nextInt (6);
        fx.amount = random.nextFloat();
        fx.tone = random.nextFloat();
    }

    applySettingsToParameters (s);
    currentPreset = -1;
}

void VoxPlusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("currentPreset", currentPreset);
        copyXmlToBinary (*xml, destData);
    }
}

void VoxPlusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        currentPreset = xml->getIntAttribute ("currentPreset", 0);
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

} // namespace voxplus

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new voxplus::VoxPlusAudioProcessor();
}
