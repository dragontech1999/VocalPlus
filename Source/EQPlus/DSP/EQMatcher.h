#pragma once

#include "EQTypes.h"
#include "SpectrumAnalyzer.h"
#include <vector>

namespace eqplus
{

class EQMatcher
{
public:
    std::vector<BandSettings> computeMatchBands (const SpectrumAnalyzer& analyzer,
                                                 int maxBands = 8) const;

private:
    float getSpectrumAtFreq (const std::array<float, SpectrumAnalyzer::fftSize / 2>& spectrum,
                             float freq, double sampleRate) const;
};

} // namespace eqplus
