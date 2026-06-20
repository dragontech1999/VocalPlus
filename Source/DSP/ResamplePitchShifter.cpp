#include "ResamplePitchShifter.h"

namespace vocalplus
{

void ResamplePitchShifter::prepare (double sr, int maxBlockSize)
{
    juce::ignoreUnused (maxBlockSize);
    sampleRate = sr;
    const int bufLen = juce::jmax (16384, static_cast<int> (sr * 0.35));
    ringBuffer.assign (static_cast<size_t> (bufLen), 0.0f);
    reset();
}

void ResamplePitchShifter::reset()
{
    std::fill (ringBuffer.begin(), ringBuffer.end(), 0.0f);

    if (ringBuffer.empty())
        return;

    writeIndex = 0;
    readPos = static_cast<double> (ringBuffer.size()) * 0.4;
}

void ResamplePitchShifter::setPitchRatio (float ratio)
{
    pitchRatio = juce::jlimit (0.25f, 4.0f, ratio);
}

void ResamplePitchShifter::setPitchSemitones (float semitones)
{
    setPitchRatio (std::pow (2.0f, semitones / 12.0f));
}

float ResamplePitchShifter::readInterpolated (double pos) const
{
    const int bufLen = static_cast<int> (ringBuffer.size());
    if (bufLen <= 1)
        return 0.0f;

    while (pos < 0.0)
        pos += bufLen;
    while (pos >= bufLen)
        pos -= bufLen;

    const int idx0 = static_cast<int> (pos);
    const int idx1 = (idx0 + 1) % bufLen;
    const float frac = static_cast<float> (pos - static_cast<double> (idx0));
    return ringBuffer[static_cast<size_t> (idx0)] * (1.0f - frac)
         + ringBuffer[static_cast<size_t> (idx1)] * frac;
}

void ResamplePitchShifter::processBlock (const float* input, float* output, int numSamples)
{
    if (ringBuffer.empty() || input == nullptr || output == nullptr || numSamples <= 0)
        return;

    const int bufLen = static_cast<int> (ringBuffer.size());
    const int minGap = 512;
    const int maxGap = bufLen - 512;
    const float makeup = 1.0f / std::sqrt (juce::jlimit (0.25f, 4.0f, pitchRatio));

    for (int i = 0; i < numSamples; ++i)
    {
        ringBuffer[static_cast<size_t> (writeIndex)] = input[i];
        writeIndex = (writeIndex + 1) % bufLen;

        output[i] = readInterpolated (readPos) * makeup;

        readPos += static_cast<double> (pitchRatio);
        while (readPos >= bufLen)
            readPos -= bufLen;

        int gap = writeIndex - static_cast<int> (readPos);
        if (gap < 0)
            gap += bufLen;

        if (gap < minGap || gap > maxGap)
            readPos = static_cast<double> ((writeIndex + bufLen - minGap * 2) % bufLen);
    }
}

} // namespace vocalplus
