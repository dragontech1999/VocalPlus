#include "EQEngine.h"

namespace eqplus
{

void EQEngine::prepare (double sr, int maxBlockSize, int channels)
{
    sampleRate = sr;
    numChannels = channels;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels = static_cast<juce::uint32> (channels);

    for (auto& band : bandProcessors)
        band.prepare (spec);

    analyzer.prepare (sr);
    analyzer.setSpeed (globalSettings.spectrumSpeed);
    reset();
}

void EQEngine::reset()
{
    for (auto& band : bandProcessors)
        band.reset();
    analyzer.reset();
    autoGainCompensationDb = 0.0f;
    outputPeakDb = -100.0f;
}

void EQEngine::setBands (const std::array<BandSettings, kMaxBands>& newBands)
{
    bands = newBands;
    for (int i = 0; i < kMaxBands; ++i)
        bandProcessors[static_cast<size_t> (i)].updateCoefficients (bands[static_cast<size_t> (i)], sampleRate);
}

void EQEngine::setGlobalSettings (const EQGlobalSettings& settings)
{
    globalSettings = settings;
    analyzer.setSpeed (settings.spectrumSpeed);
}

bool EQEngine::anySoloActive() const
{
    for (const auto& b : bands)
        if (b.enabled && b.solo)
            return true;
    return false;
}

void EQEngine::applyBand (juce::AudioBuffer<float>& buffer, int bandIndex)
{
    const auto& settings = bands[static_cast<size_t> (bandIndex)];
    if (! settings.enabled)
        return;

    if (anySoloActive() && ! settings.solo)
        return;

    auto& processor = bandProcessors[static_cast<size_t> (bandIndex)];

    if (settings.channel == ChannelMode::Mid || settings.channel == ChannelMode::Side)
    {
        if (buffer.getNumChannels() < 2)
            return;

        juce::AudioBuffer<float> midSide (2, buffer.getNumSamples());
        const auto* left = buffer.getReadPointer (0);
        const auto* right = buffer.getReadPointer (1);
        auto* mid = midSide.getWritePointer (0);
        auto* side = midSide.getWritePointer (1);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            mid[i] = 0.5f * (left[i] + right[i]);
            side[i] = 0.5f * (left[i] - right[i]);
        }

        const int targetCh = settings.channel == ChannelMode::Mid ? 0 : 1;
        juce::AudioBuffer<float> target (1, buffer.getNumSamples());
        target.copyFrom (0, 0, midSide, targetCh, 0, buffer.getNumSamples());

        auto block = juce::dsp::AudioBlock<float> (target);
        processor.processBlock (block, settings);

        midSide.copyFrom (targetCh, 0, target, 0, 0, buffer.getNumSamples());

        auto* outLeft = buffer.getWritePointer (0);
        auto* outRight = buffer.getWritePointer (1);
        const auto* procMid = midSide.getReadPointer (0);
        const auto* procSide = midSide.getReadPointer (1);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            outLeft[i] = procMid[i] + procSide[i];
            outRight[i] = procMid[i] - procSide[i];
        }
        return;
    }

    if (settings.channel == ChannelMode::Left)
    {
        juce::AudioBuffer<float> ch (1, buffer.getNumSamples());
        ch.copyFrom (0, 0, buffer, 0, 0, buffer.getNumSamples());
        auto block = juce::dsp::AudioBlock<float> (ch);
        processor.processBlock (block, settings);
        buffer.copyFrom (0, 0, ch, 0, 0, buffer.getNumSamples());
        return;
    }

    if (settings.channel == ChannelMode::Right)
    {
        if (buffer.getNumChannels() < 2)
            return;
        juce::AudioBuffer<float> ch (1, buffer.getNumSamples());
        ch.copyFrom (0, 0, buffer, 1, 0, buffer.getNumSamples());
        auto block = juce::dsp::AudioBlock<float> (ch);
        processor.processBlock (block, settings);
        buffer.copyFrom (1, 0, ch, 0, 0, buffer.getNumSamples());
        return;
    }

    auto block = juce::dsp::AudioBlock<float> (buffer);
    processor.processBlock (block, settings);
}

void EQEngine::updateAutoGain()
{
    if (! globalSettings.autoGain)
    {
        autoGainCompensationDb = 0.0f;
        return;
    }

    if (postRms > 1.0e-8f && preRms > 1.0e-8f)
    {
        const float target = juce::Decibels::gainToDecibels (preRms);
        const float actual = juce::Decibels::gainToDecibels (postRms);
        autoGainCompensationDb = 0.92f * autoGainCompensationDb + 0.08f * (target - actual);
        autoGainCompensationDb = juce::jlimit (-12.0f, 12.0f, autoGainCompensationDb);
    }
}

void EQEngine::processBlock (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    juce::AudioBuffer<float> mono (1, numSamples);
    if (buffer.getNumChannels() >= 2)
    {
        mono.copyFrom (0, 0, buffer, 0, 0, numSamples);
        mono.addFrom (0, 0, buffer, 1, 0, numSamples, 0.5f);
    }
    else
    {
        mono.copyFrom (0, 0, buffer, 0, 0, numSamples);
    }

    if (globalSettings.showPreSpectrum)
        analyzer.pushSamples (mono.getReadPointer (0), numSamples, false);

    float sumSq = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sumSq += mono.getSample (0, i) * mono.getSample (0, i);
    preRms = std::sqrt (sumSq / static_cast<float> (numSamples));

    juce::AudioBuffer<float> work (buffer);
    for (int b = 0; b < kMaxBands; ++b)
        applyBand (work, b);

    if (globalSettings.phaseInvert)
    {
        for (int ch = 0; ch < work.getNumChannels(); ++ch)
            work.applyGain (ch, 0, numSamples, -1.0f);
    }

    updateAutoGain();
    const float totalGain = juce::Decibels::decibelsToGain (globalSettings.outputGainDb + autoGainCompensationDb);
    work.applyGain (totalGain);

    float peak = 0.0f;
    sumSq = 0.0f;
    for (int ch = 0; ch < work.getNumChannels(); ++ch)
    {
        const auto* data = work.getReadPointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            peak = juce::jmax (peak, std::abs (data[i]));
            sumSq += data[i] * data[i];
        }
    }
    postRms = std::sqrt (sumSq / static_cast<float> (numSamples * work.getNumChannels()));
    outputPeakDb = juce::Decibels::gainToDecibels (peak + 1.0e-8f);

    if (globalSettings.showPostSpectrum)
    {
        juce::AudioBuffer<float> postMono (1, numSamples);
        if (work.getNumChannels() >= 2)
        {
            postMono.copyFrom (0, 0, work, 0, 0, numSamples);
            postMono.addFrom (0, 0, work, 1, 0, numSamples, 0.5f);
        }
        else
        {
            postMono.copyFrom (0, 0, work, 0, 0, numSamples);
        }
        analyzer.pushSamples (postMono.getReadPointer (0), numSamples, true);
    }

    buffer.makeCopyOf (work);
}

float EQEngine::getMagnitudeAtFrequency (float freq) const
{
    float mag = 1.0f;
    for (int i = 0; i < kMaxBands; ++i)
    {
        if (! bands[static_cast<size_t> (i)].enabled)
            continue;
        mag *= bandProcessors[static_cast<size_t> (i)].getMagnitudeAtFrequency (freq, sampleRate);
    }
    return mag;
}

float EQEngine::getMagnitudeDbAtFrequency (float freq) const
{
    return juce::Decibels::gainToDecibels (getMagnitudeAtFrequency (freq));
}

float EQEngine::getBandDynamicGainDb (int bandIndex) const
{
    if (bandIndex < 0 || bandIndex >= kMaxBands)
        return 0.0f;

    return bandProcessors[static_cast<size_t> (bandIndex)].getLastDynamicGainDb();
}

void EQEngine::sketchCurve (const std::vector<std::pair<float, float>>& points)
{
    if (points.size() < 2)
        return;

    int bandIndex = 0;
    for (size_t i = 1; i < points.size() && bandIndex < kMaxBands; ++i, ++bandIndex)
    {
        const auto& p0 = points[i - 1];
        const auto& p1 = points[i];
        const float midFreq = std::sqrt (p0.first * p1.first);
        const float midGain = 0.5f * (p0.second + p1.second);

        auto& band = bands[static_cast<size_t> (bandIndex)];
        band.enabled = true;
        band.type = FilterType::Bell;
        band.frequency = juce::jlimit (kMinFreq, kMaxFreq, midFreq);
        band.gainDb = juce::jlimit (kMinGain, kMaxGain, midGain);
        band.q = 1.5f;
        bandProcessors[static_cast<size_t> (bandIndex)].updateCoefficients (band, sampleRate);
    }
}

} // namespace eqplus
