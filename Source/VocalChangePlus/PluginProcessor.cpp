#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "../DSP/ParameterHelpers.h"
#include "../DSP/AudioSafety.h"

namespace vocalchangeplus
{

using namespace ParamIDs;
using namespace vocalplus;

VocalChangePlusAudioProcessor::VocalChangePlusAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "VocalChangePlus", createParameterLayout())
{
    applyPreset (1);
}

VocalChangePlusAudioProcessor::~VocalChangePlusAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout VocalChangePlusAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (inputGain, "Input",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (outputGain, "Output",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (mix, "Mix",
        linearRange (0.0f, 1.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (pitchShift, "Pitch",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (formant, "Formant",
        rangedRange (0.5f, 1.5f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (speed, "Speed",
        rangedRange (0.5f, 2.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (distortion, "Distort",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (reverb, "Reverb",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (robot, "Robot",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (chorus, "Chorus",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        filterMode, "Filter",
        juce::StringArray { "Off", "Phone", "Radio", "Space", "Cave" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (correction, "Correction",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (aiMorph, "AI Morph",
        linearRange (0.0f, 1.0f, 0.01f), 0.0f));

    return { params.begin(), params.end() };
}

void VocalChangePlusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    engine.setSettings (readSettingsFromParameters());
}

void VocalChangePlusAudioProcessor::releaseResources() {}

bool VocalChangePlusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

VocalChangeSettings VocalChangePlusAudioProcessor::readSettingsFromParameters() const
{
    VocalChangeSettings s;
    s.inputGainDb = readFloatParam (apvts, inputGain);
    s.outputGainDb = readFloatParam (apvts, outputGain);
    s.mix = readFloatParam (apvts, mix);
    s.pitchShiftSemitones = readFloatParam (apvts, pitchShift);
    s.formant = readFloatParam (apvts, formant);
    s.speed = readFloatParam (apvts, speed);
    s.distortion = readFloatParam (apvts, distortion);
    s.reverb = readFloatParam (apvts, reverb);
    s.robot = readFloatParam (apvts, robot);
    s.chorus = readFloatParam (apvts, chorus);
    s.filterMode = readChoiceParam (apvts, filterMode);
    s.correction = readFloatParam (apvts, correction);
    s.aiMorph = readFloatParam (apvts, aiMorph);
    return s;
}

void VocalChangePlusAudioProcessor::applySettingsToParameters (const VocalChangeSettings& s)
{
    auto setFloat = [this] (const juce::String& id, float value)
    {
        apvts.getParameter (id)->setValueNotifyingHost (
            apvts.getParameterRange (id).convertTo0to1 (value));
    };

    setFloat (inputGain, s.inputGainDb);
    setFloat (outputGain, s.outputGainDb);
    setFloat (mix, s.mix);
    setFloat (pitchShift, s.pitchShiftSemitones);
    setFloat (formant, s.formant);
    setFloat (speed, s.speed);
    setFloat (distortion, s.distortion);
    setFloat (reverb, s.reverb);
    setFloat (robot, s.robot);
    setFloat (chorus, s.chorus);
    apvts.getParameter (filterMode)->setValueNotifyingHost (
        apvts.getParameterRange (filterMode).convertTo0to1 (static_cast<float> (s.filterMode)));
    setFloat (correction, s.correction);
    setFloat (aiMorph, s.aiMorph);
}

void VocalChangePlusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (getSampleRate() <= 0.0 || buffer.getNumSamples() <= 0)
        return;

    engine.setSettings (readSettingsFromParameters());
    engine.process (buffer);
    vocalplus::sanitizeBuffer (buffer);
}

juce::AudioProcessorEditor* VocalChangePlusAudioProcessor::createEditor()
{
    return new VocalChangePlusAudioProcessorEditor (*this);
}

int VocalChangePlusAudioProcessor::getNumPrograms()
{
    return presetManager.getNumPresets();
}

int VocalChangePlusAudioProcessor::getCurrentProgram()
{
    return currentPreset;
}

void VocalChangePlusAudioProcessor::setCurrentProgram (int index)
{
    applyPreset (index);
}

const juce::String VocalChangePlusAudioProcessor::getProgramName (int index)
{
    return presetManager.getPreset (index).name;
}

void VocalChangePlusAudioProcessor::applyPreset (int index)
{
    index = juce::jlimit (0, presetManager.getNumPresets() - 1, index);
    currentPreset = index;
    applySettingsToParameters (presetManager.getPreset (index).settings);
}

void VocalChangePlusAudioProcessor::randomizeSettings()
{
    VocalChangeSettings s;
    s.pitchShiftSemitones = random.nextFloat() * 20.0f - 10.0f;
    s.formant = 0.5f + random.nextFloat() * 1.0f;
    s.speed = 0.7f + random.nextFloat() * 0.8f;
    s.distortion = random.nextFloat() * 0.8f;
    s.reverb = random.nextFloat() * 0.7f;
    s.robot = random.nextFloat() * 0.9f;
    s.chorus = random.nextFloat() * 0.8f;
    s.filterMode = random.nextInt (5);
    s.correction = random.nextFloat() * 0.5f;
    s.mix = 0.7f + random.nextFloat() * 0.3f;
    s.aiMorph = random.nextFloat() * 0.85f;
    applySettingsToParameters (s);
    currentPreset = -1;
}

void VocalChangePlusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("currentPreset", currentPreset);
        copyXmlToBinary (*xml, destData);
    }
}

void VocalChangePlusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        currentPreset = xml->getIntAttribute ("currentPreset", 0);
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

} // namespace vocalchangeplus

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new vocalchangeplus::VocalChangePlusAudioProcessor();
}
