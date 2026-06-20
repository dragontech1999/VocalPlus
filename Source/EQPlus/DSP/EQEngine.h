#pragma once

#include "EQBand.h"
#include "EQTypes.h"
#include "SpectrumAnalyzer.h"
#include <array>

namespace eqplus
{

class EQEngine
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void setBands (const std::array<BandSettings, kMaxBands>& newBands);
    void setGlobalSettings (const EQGlobalSettings& settings);
    void processBlock (juce::AudioBuffer<float>& buffer);

    SpectrumAnalyzer& getSpectrumAnalyzer() noexcept { return analyzer; }
    const std::array<BandSettings, kMaxBands>& getBands() const noexcept { return bands; }

    float getMagnitudeAtFrequency (float freq) const;
    float getMagnitudeDbAtFrequency (float freq) const;
    float getBandDynamicGainDb (int bandIndex) const;
    float getAutoGainCompensationDb() const noexcept { return autoGainCompensationDb; }
    float getOutputPeakDb() const noexcept { return outputPeakDb; }

    void sketchCurve (const std::vector<std::pair<float, float>>& points);

private:
    void processMidSide (juce::AudioBuffer<float>& buffer);
    void applyBand (juce::AudioBuffer<float>& buffer, int bandIndex);
    void updateAutoGain();
    bool anySoloActive() const;

    std::array<EQBand, kMaxBands> bandProcessors;
    std::array<BandSettings, kMaxBands> bands {};
    EQGlobalSettings globalSettings {};
    SpectrumAnalyzer analyzer;

    double sampleRate = 44100.0;
    int numChannels = 2;
    float autoGainCompensationDb = 0.0f;
    float outputPeakDb = -100.0f;
    float preRms = 0.0f;
    float postRms = 0.0f;
};

} // namespace eqplus
