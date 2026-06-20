#pragma once

#include <JuceHeader.h>
#include <complex>
#include <vector>

namespace vocalplus
{

/** STFT phase-vocoder pitch shifter with optional formant preservation. */
class PhaseVocoderPitchShifter
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setPitchRatio (float ratio);
    void setFormantRatio (float ratio);

    void processBlock (const float* input, float* output, int numSamples);

private:
    void processFrame();

    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { static_cast<size_t> (fftSize),
                                                 juce::dsp::WindowingFunction<float>::hann,
                                                 true };

    std::vector<float> inputFifo;
    std::vector<float> outputFifo;
    std::vector<float> frame;
    std::vector<float> lastPhase;
    std::vector<float> sumPhase;
    std::vector<float> magnitudes;
    std::vector<float> frequencies;
    std::vector<std::complex<float>> spectrum;
    std::vector<std::complex<float>> ifftBuffer;
    std::vector<float> processScratch;

    int inputWritePos = 0;
    int outputReadPos = 0;
    int outputWritePos = 0;
    int samplesSinceLastFrame = 0;

    float pitchRatio = 1.0f;
    float formantRatio = 1.0f;
    double sampleRate = 44100.0;
};

} // namespace vocalplus
