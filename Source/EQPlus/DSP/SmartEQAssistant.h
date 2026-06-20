#pragma once

#include "EQTypes.h"
#include "SpectrumAnalyzer.h"
#include <vector>

namespace eqplus
{

struct EQSuggestion
{
    FilterType type = FilterType::Bell;
    float frequency = 1000.0f;
    float gainDb = 0.0f;
    float q = 2.0f;
    juce::String reason;
};

class SmartEQAssistant
{
public:
    std::vector<EQSuggestion> analyze (const SpectrumAnalyzer& analyzer, double sampleRate);

private:
    std::vector<int> findPeaks (const std::array<float, SpectrumAnalyzer::fftSize / 2>& spectrum,
                                float thresholdDb) const;
    float estimateLowEndBuildup (const std::array<float, SpectrumAnalyzer::fftSize / 2>& spectrum,
                                 double sampleRate) const;
};

} // namespace eqplus
