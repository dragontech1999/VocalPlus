#include "SpectrumAnalyzer.h"

namespace eqplus
{

void SpectrumAnalyzer::prepare (double sr)
{
    sampleRate = sr;
    reset();
}

void SpectrumAnalyzer::reset()
{
    preRingBuffer.fill (0.0f);
    postRingBuffer.fill (0.0f);
    fftData.fill (0.0f);
    preMagnitudes.fill (-100.0f);
    postMagnitudes.fill (-100.0f);
    accumulatedPre.fill (-100.0f);
    smoothedPre.fill (-100.0f);
    smoothedPost.fill (-100.0f);
    preRingWritePos = 0;
    postRingWritePos = 0;
    preSamplesUntilFFT = 0;
    postSamplesUntilFFT = 0;
    prePeakDb = -100.0f;
    postPeakDb = -100.0f;
}

float SpectrumAnalyzer::getBinFrequency (int bin) const noexcept
{
    return static_cast<float> (bin) * static_cast<float> (sampleRate) / static_cast<float> (fftSize);
}

void SpectrumAnalyzer::pushSamples (const float* samples, int numSamples, bool isPost)
{
    if (frozen)
        return;

    auto& ringBuffer = isPost ? postRingBuffer : preRingBuffer;
    int& ringWritePos = isPost ? postRingWritePos : preRingWritePos;
    int& samplesUntilFFT = isPost ? postSamplesUntilFFT : preSamplesUntilFFT;

    for (int i = 0; i < numSamples; ++i)
    {
        ringBuffer[static_cast<size_t> (ringWritePos)] = samples[i];
        ringWritePos = (ringWritePos + 1) % fftSize;
        ++samplesUntilFFT;

        if (samplesUntilFFT >= hopSize)
        {
            samplesUntilFFT = 0;
            for (int j = 0; j < fftSize; ++j)
            {
                const int idx = (ringWritePos + j) % fftSize;
                fftData[static_cast<size_t> (j)] = ringBuffer[static_cast<size_t> (idx)];
            }
            processFFT (isPost, ringBuffer);
        }
    }
}

void SpectrumAnalyzer::processFFT (bool post, const std::array<float, fftSize>& /*ringBuffer*/)
{
    window.multiplyWithWindowingTable (fftData.data(), static_cast<size_t> (fftSize));
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    auto& target = post ? postMagnitudes : preMagnitudes;
    auto& smoothed = post ? smoothedPost : smoothedPre;
    float framePeak = -100.0f;

    for (int i = 1; i < fftSize / 2; ++i)
    {
        const float mag = fftData[static_cast<size_t> (i)];
        const float db = juce::Decibels::gainToDecibels (mag / static_cast<float> (fftSize) + 1.0e-12f, -100.0f);
        smoothed[static_cast<size_t> (i)] = smoothing * smoothed[static_cast<size_t> (i)]
                                          + (1.0f - smoothing) * db;
        target[static_cast<size_t> (i)] = smoothed[static_cast<size_t> (i)];
        framePeak = juce::jmax (framePeak, db);
    }

    if (post)
        postPeakDb = 0.85f * postPeakDb + 0.15f * framePeak;
    else
    {
        prePeakDb = 0.85f * prePeakDb + 0.15f * framePeak;
        for (int i = 1; i < fftSize / 2; ++i)
        {
            accumulatedPre[static_cast<size_t> (i)] = juce::jmax (accumulatedPre[static_cast<size_t> (i)] * 0.92f,
                                                                   target[static_cast<size_t> (i)]);
        }
    }

    frameCounter.fetch_add (1, std::memory_order_relaxed);
}

void SpectrumAnalyzer::captureReference()
{
    referenceMagnitudes = accumulatedPre;
    referenceCaptured = true;
}

} // namespace eqplus
