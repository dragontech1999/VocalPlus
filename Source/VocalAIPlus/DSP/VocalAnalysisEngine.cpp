#include "VocalAnalysisEngine.h"

namespace vocalaiplus
{

void VocalAnalysisEngine::prepare (double sampleRate)
{
    sampleRateHz = sampleRate;
    fftData.assign (static_cast<size_t> (fftSize * 2), 0.0f);
    sampleRing.assign (static_cast<size_t> (fftSize), 0.0f);
    reset();
    prepared = true;
}

void VocalAnalysisEngine::reset()
{
    sumSquares = 0.0f;
    sampleCount = 0;
    peak = 0.0f;
    ringWrite = 0;
    std::fill (sampleRing.begin(), sampleRing.end(), 0.0f);
}

void VocalAnalysisEngine::pushSamples (const juce::AudioBuffer<float>& buffer)
{
    if (! prepared || buffer.getNumSamples() <= 0)
        return;

    const auto* mono = buffer.getReadPointer (0);
    const int n = buffer.getNumSamples();

    for (int i = 0; i < n; ++i)
    {
        const float s = mono[i];
        sumSquares += s * s;
        ++sampleCount;
        peak = juce::jmax (peak, std::abs (s));
        sampleRing[static_cast<size_t> (ringWrite)] = s;
        ringWrite = (ringWrite + 1) % fftSize;
    }
}

float VocalAnalysisEngine::bandEnergyRatio (float lowHz, float highHz) const
{
    if (sampleRateHz <= 0.0 || fftData.empty())
        return 0.0f;

    std::vector<float> data (static_cast<size_t> (fftSize * 2), 0.0f);
    for (int i = 0; i < fftSize; ++i)
    {
        const int idx = (ringWrite + i) % fftSize;
        data[static_cast<size_t> (i)] = sampleRing[static_cast<size_t> (idx)];
    }

    window.multiplyWithWindowingTable (data.data(), static_cast<size_t> (fftSize));
    fft.performFrequencyOnlyForwardTransform (data.data());

    const float binHz = static_cast<float> (sampleRateHz) / static_cast<float> (fftSize);
    float total = 0.0f;
    float band = 0.0f;

    for (int bin = 1; bin < fftSize / 2; ++bin)
    {
        const float freq = static_cast<float> (bin) * binHz;
        const float mag = data[static_cast<size_t> (bin)];
        const float energy = mag * mag;
        total += energy;
        if (freq >= lowHz && freq <= highHz)
            band += energy;
    }

    return total > 1.0e-12f ? band / total : 0.0f;
}

VocalAnalysisSnapshot VocalAnalysisEngine::analyze() const
{
    VocalAnalysisSnapshot snap;

    if (sampleCount <= 0)
        return snap;

    const float rms = std::sqrt (sumSquares / static_cast<float> (sampleCount));
    snap.integratedLUFS = -0.691f + 10.0f * std::log10 (juce::jmax (1.0e-12f, rms * rms));
    snap.truePeakDBTP = peak > 1.0e-8f ? 20.0f * std::log10 (peak) : -60.0f;
    snap.crestFactorDB = snap.truePeakDBTP - snap.integratedLUFS;

    std::vector<float> data (static_cast<size_t> (fftSize * 2), 0.0f);
    for (int i = 0; i < fftSize; ++i)
    {
        const int idx = (ringWrite + i) % fftSize;
        data[static_cast<size_t> (i)] = sampleRing[static_cast<size_t> (idx)];
    }

    window.multiplyWithWindowingTable (data.data(), static_cast<size_t> (fftSize));
    juce::dsp::FFT localFft (fftOrder);
    localFft.performFrequencyOnlyForwardTransform (data.data());

    const float binHz = static_cast<float> (sampleRateHz) / static_cast<float> (fftSize);
    float weighted = 0.0f;
    float total = 0.0f;

    for (int bin = 1; bin < fftSize / 2; ++bin)
    {
        const float freq = static_cast<float> (bin) * binHz;
        const float mag = data[static_cast<size_t> (bin)];
        const float energy = mag * mag;
        weighted += freq * energy;
        total += energy;
    }

    snap.spectralCentroidHz = total > 1.0e-12f ? weighted / total : 2500.0f;
    snap.sibilanceIndex = bandEnergyRatio (5000.0f, 12000.0f);
    snap.lowMidMudIndex = bandEnergyRatio (180.0f, 450.0f);
    snap.presenceIndex = bandEnergyRatio (2500.0f, 6000.0f);
    snap.valid = true;
    return snap;
}

} // namespace vocalaiplus
