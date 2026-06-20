#include "EQBand.h"

namespace eqplus
{

void EQBand::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    for (auto& channel : stages)
        for (auto& stage : channel)
            stage.prepare (spec);
    envelope.prepare (sampleRate);
    reset();
}

void EQBand::reset()
{
    for (auto& channel : stages)
        for (auto& stage : channel)
            stage.reset();
    envelope.reset();
    lastDynGainDb = 0.0f;
}

int EQBand::numActiveStages (const BandSettings& settings) const
{
    if (settings.type == FilterType::LowCut || settings.type == FilterType::HighCut)
        return juce::jlimit (1, kMaxStages, static_cast<int> (settings.slope / 12.0f));
    return 1;
}

juce::dsp::IIR::Coefficients<float>::Ptr EQBand::makeCoefficients (const BandSettings& settings,
                                                                   double sr, float gainOffsetDb) const
{
    const float freq = juce::jlimit (kMinFreq, static_cast<float> (sr * 0.49), settings.frequency);
    const float q = juce::jlimit (kMinQ, kMaxQ, settings.q);
    const float gain = juce::jlimit (kMinGain, kMaxGain, settings.gainDb + gainOffsetDb);

    switch (settings.type)
    {
        case FilterType::Bell:
            return Coeffs::makePeakFilter (sr, freq, q, juce::Decibels::decibelsToGain (gain));
        case FilterType::LowShelf:
            return Coeffs::makeLowShelf (sr, freq, q, juce::Decibels::decibelsToGain (gain));
        case FilterType::HighShelf:
            return Coeffs::makeHighShelf (sr, freq, q, juce::Decibels::decibelsToGain (gain));
        case FilterType::LowCut:
            return Coeffs::makeHighPass (sr, freq, 0.707f);
        case FilterType::HighCut:
            return Coeffs::makeLowPass (sr, freq, 0.707f);
        case FilterType::Notch:
            return Coeffs::makeNotch (sr, freq, q);
        case FilterType::BandPass:
            return Coeffs::makeBandPass (sr, freq, q);
        case FilterType::TiltShelf:
        {
            const float tiltGain = gain * 0.5f;
            return Coeffs::makeLowShelf (sr, 1000.0f, 0.707f, juce::Decibels::decibelsToGain (tiltGain));
        }
        case FilterType::AllPass:
            return Coeffs::makeAllPass (sr, freq, q);
    }
    return Coeffs::makeAllPass (sr, 1000.0f, 0.707f);
}

void EQBand::updateCoefficients (const BandSettings& settings, double sr)
{
    sampleRate = sr;
    activeStages = numActiveStages (settings);
    const auto coeffs = makeCoefficients (settings, sr, lastDynGainDb);

    if (coeffs == nullptr)
        return;

    for (auto& channel : stages)
        for (int s = 0; s < activeStages; ++s)
            *channel[static_cast<size_t> (s)].coefficients = *coeffs;
}

float EQBand::getMagnitudeAtFrequency (float freq, double sr) const
{
    float mag = 1.0f;
    for (int s = 0; s < activeStages; ++s)
    {
        if (stages[0][static_cast<size_t> (s)].coefficients == nullptr)
            continue;
        mag *= stages[0][static_cast<size_t> (s)].coefficients->getMagnitudeForFrequency (freq, sr);
    }
    return mag;
}

void EQBand::processBlock (juce::dsp::AudioBlock<float>& block, const BandSettings& settings)
{
    if (! settings.enabled || block.getNumSamples() == 0)
        return;

    if (settings.dynamicEnabled)
    {
        float blockPeak = 0.0f;
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            const auto* data = block.getChannelPointer (ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
                blockPeak = juce::jmax (blockPeak, std::abs (data[i]));
        }

        const float level = juce::Decibels::gainToDecibels (blockPeak + 1.0e-8f);
        envelope.setAttackMs (settings.dynAttackMs);
        envelope.setReleaseMs (settings.dynReleaseMs);
        lastDynGainDb = envelope.process (level, settings.dynThresholdDb, settings.dynRatio);
        updateCoefficients (settings, sampleRate);
    }

    activeStages = numActiveStages (settings);

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock (ch);
        const size_t stageCh = juce::jmin (ch, size_t { 1 });

        for (int s = 0; s < activeStages; ++s)
            stages[stageCh][static_cast<size_t> (s)].process (juce::dsp::ProcessContextReplacing<float> (channelBlock));
    }
}

} // namespace eqplus
