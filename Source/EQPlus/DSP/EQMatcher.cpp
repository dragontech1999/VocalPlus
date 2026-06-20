#include "EQMatcher.h"

namespace eqplus
{

float EQMatcher::getSpectrumAtFreq (const std::array<float, SpectrumAnalyzer::fftSize / 2>& spectrum,
                                    float freq, double sampleRate) const
{
    const float binF = freq * static_cast<float> (SpectrumAnalyzer::fftSize) / static_cast<float> (sampleRate);
    const int bin = juce::jlimit (1, SpectrumAnalyzer::fftSize / 2 - 2, static_cast<int> (binF));
    const float frac = binF - static_cast<float> (bin);
    const float a = spectrum[static_cast<size_t> (bin)];
    const float b = spectrum[static_cast<size_t> (bin + 1)];
    return a + frac * (b - a);
}

std::vector<BandSettings> EQMatcher::computeMatchBands (const SpectrumAnalyzer& analyzer,
                                                        int maxBands) const
{
    std::vector<BandSettings> bands;
    if (! analyzer.hasReference())
        return bands;

    const auto& input = analyzer.getSpectrum (false);
    const auto& reference = analyzer.getReferenceSpectrum();
    const double sr = 44100.0;

    const std::array<float, 8> matchFreqs = { 60.0f, 150.0f, 400.0f, 800.0f,
                                              1600.0f, 3200.0f, 6400.0f, 12000.0f };

    for (const float freq : matchFreqs)
    {
        if (static_cast<int> (bands.size()) >= maxBands)
            break;

        const float inputDb = getSpectrumAtFreq (input, freq, sr);
        const float refDb = getSpectrumAtFreq (reference, freq, sr);
        const float diff = refDb - inputDb;

        if (std::abs (diff) < 1.5f)
            continue;

        BandSettings band;
        band.enabled = true;
        band.type = FilterType::Bell;
        band.frequency = freq;
        band.gainDb = juce::jlimit (-12.0f, 12.0f, diff);
        band.q = freq < 200.0f ? 0.8f : (freq > 5000.0f ? 1.5f : 1.2f);
        bands.push_back (band);
    }

    return bands;
}

} // namespace eqplus
