#pragma once

#include "EQTypes.h"
#include <JuceHeader.h>
#include <array>
#include <atomic>

namespace eqplus
{

class SpectrumAnalyzer
{
public:
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4;

    void prepare (double sampleRate);
    void reset();
    void pushSamples (const float* samples, int numSamples, bool isPost);
    void setSpeed (float speed) noexcept { smoothing = juce::jlimit (0.5f, 0.98f, speed); }

    const std::array<float, fftSize / 2>& getSpectrum (bool post) const noexcept
    {
        return post ? postMagnitudes : preMagnitudes;
    }

    const std::array<float, fftSize / 2>& getAccumulatedSpectrum() const noexcept { return accumulatedPre; }
    float getBinFrequency (int bin) const noexcept;
    float getPeakDb (bool post = false) const noexcept { return post ? postPeakDb : prePeakDb; }
    bool hasSignal() const noexcept { return prePeakDb > -55.0f; }
    int getFrameCounter() const noexcept { return frameCounter.load (std::memory_order_relaxed); }

    void freeze (bool shouldFreeze) noexcept { frozen = shouldFreeze; }
    bool isFrozen() const noexcept { return frozen; }

    void captureReference();
    bool hasReference() const noexcept { return referenceCaptured; }
    const std::array<float, fftSize / 2>& getReferenceSpectrum() const noexcept { return referenceMagnitudes; }

private:
    void processFFT (bool post, const std::array<float, fftSize>& ringBuffer);

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { static_cast<size_t> (fftSize),
                                                  juce::dsp::WindowingFunction<float>::hann };

    std::array<float, fftSize> preRingBuffer {};
    std::array<float, fftSize> postRingBuffer {};
    std::array<float, fftSize> fftData {};
    std::array<float, fftSize / 2> preMagnitudes {};
    std::array<float, fftSize / 2> postMagnitudes {};
    std::array<float, fftSize / 2> accumulatedPre {};
    std::array<float, fftSize / 2> referenceMagnitudes {};
    std::array<float, fftSize / 2> smoothedPre {};
    std::array<float, fftSize / 2> smoothedPost {};

    int preRingWritePos = 0;
    int postRingWritePos = 0;
    int preSamplesUntilFFT = 0;
    int postSamplesUntilFFT = 0;
    double sampleRate = 44100.0;
    float smoothing = 0.85f;
    float prePeakDb = -100.0f;
    float postPeakDb = -100.0f;
    bool frozen = false;
    bool referenceCaptured = false;
    std::atomic<int> frameCounter { 0 };
};

} // namespace eqplus
