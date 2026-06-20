#pragma once

#include <JuceHeader.h>

namespace vocalaiplus
{

struct VocalAnalysisSnapshot
{
    float integratedLUFS = -18.0f;
    float truePeakDBTP = -6.0f;
    float crestFactorDB = 12.0f;
    float spectralCentroidHz = 2500.0f;
    float sibilanceIndex = 0.25f;
    float lowMidMudIndex = 0.30f;
    float presenceIndex = 0.35f;
    bool valid = false;
};

class VocalAnalysisEngine
{
public:
    void prepare (double sampleRate);
    void reset();
    void pushSamples (const juce::AudioBuffer<float>& buffer);
    VocalAnalysisSnapshot analyze() const;

private:
    float bandEnergyRatio (float lowHz, float highHz) const;

    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { static_cast<size_t> (fftSize),
                                                  juce::dsp::WindowingFunction<float>::hann };

    std::vector<float> fftData;
    std::vector<float> sampleRing;
    double sampleRateHz = 44100.0;
    float sumSquares = 0.0f;
    int sampleCount = 0;
    float peak = 0.0f;
    int ringWrite = 0;
    bool prepared = false;
};

} // namespace vocalaiplus
