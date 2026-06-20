#include "VocalCompressor.h"

namespace tuneplus
{

void VocalCompressor::prepare (double sampleRate, int maxBlockSize, int /*numChannels*/)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels = 1;

    compressor.prepare (spec);
    highPass.prepare (spec);
    reset();
    prepared = true;
    setSettings (settings);
}

void VocalCompressor::reset()
{
    compressor.reset();
    highPass.reset();
    gainReductionDb = 0.0f;
}

void VocalCompressor::setSettings (const CompressorSettings& newSettings)
{
    settings = newSettings;

    compressor.setThreshold (settings.thresholdDb);
    compressor.setRatio (settings.ratio);
    compressor.setAttack (settings.attackMs / 1000.0f);
    compressor.setRelease (settings.releaseMs / 1000.0f);

    if (spec.sampleRate > 0.0)
    {
        highPass.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (
            spec.sampleRate, juce::jmax (20.0f, settings.highPassHz));
    }
}

void VocalCompressor::process (juce::AudioBuffer<float>& buffer)
{
    if (! prepared || ! settings.enabled || settings.mix <= 0.001f)
    {
        gainReductionDb = 0.0f;
        return;
    }

    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    if (numSamples <= 0 || channels <= 0 || highPass.coefficients == nullptr)
        return;

    const float inPeak = buffer.getMagnitude (0, numSamples);

    juce::AudioBuffer<float> dry;
    if (settings.mix < 0.999f)
        dry.makeCopyOf (buffer, true);

    juce::dsp::AudioBlock<float> block (buffer);

    for (int ch = 0; ch < channels; ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock (static_cast<size_t> (ch));
        juce::dsp::ProcessContextReplacing<float> context (channelBlock);
        highPass.process (context);
        compressor.process (context);
    }

    const float makeupGain = juce::Decibels::decibelsToGain (settings.makeupDb);
    buffer.applyGain (makeupGain);

    if (settings.mix < 0.999f)
    {
        for (int ch = 0; ch < channels; ++ch)
        {
            auto* out = buffer.getWritePointer (ch);
            const auto* in = dry.getReadPointer (ch);

            for (int i = 0; i < numSamples; ++i)
                out[i] = in[i] + (out[i] - in[i]) * settings.mix;
        }
    }

    const float outPeak = buffer.getMagnitude (0, numSamples);
    if (inPeak > 1.0e-6f && outPeak > 1.0e-6f)
        gainReductionDb = juce::jlimit (-60.0f, 0.0f, 20.0f * std::log10 (outPeak / inPeak));
    else
        gainReductionDb = 0.0f;
}

} // namespace tuneplus
