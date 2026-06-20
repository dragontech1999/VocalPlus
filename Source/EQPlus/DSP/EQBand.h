#pragma once

#include "EQTypes.h"
#include "DynamicEQ.h"
#include <JuceHeader.h>

namespace eqplus
{

class EQBand
{
public:
    static constexpr int kMaxStages = 4;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void updateCoefficients (const BandSettings& settings, double sampleRate);
    float getMagnitudeAtFrequency (float freq, double sampleRate) const;
    float getLastDynamicGainDb() const noexcept { return lastDynGainDb; }
    void processBlock (juce::dsp::AudioBlock<float>& block, const BandSettings& settings);

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;

    Coeffs::Ptr makeCoefficients (const BandSettings& settings, double sr, float gainOffsetDb) const;
    int numActiveStages (const BandSettings& settings) const;

    std::array<std::array<Filter, kMaxStages>, 2> stages {};
    DynamicEQEnvelope envelope;
    double sampleRate = 44100.0;
    float lastDynGainDb = 0.0f;
    int activeStages = 1;
};

} // namespace eqplus
