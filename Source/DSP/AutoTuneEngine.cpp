#include "AutoTuneEngine.h"

namespace vocalplus
{

void AutoTuneEngine::prepare (double sampleRate, int maxBlockSize)
{
    detector.prepare (sampleRate, maxBlockSize);
    shifter.prepare (sampleRate, maxBlockSize);
    analysisBuffer.assign (4096, 0.0f);
    orderedAnalysis.assign (4096, 0.0f);
    scratch.assign (static_cast<size_t> (maxBlockSize), 0.0f);
    reset();
}

void AutoTuneEngine::reset()
{
    detector.reset();
    shifter.reset();
    std::fill (analysisBuffer.begin(), analysisBuffer.end(), 0.0f);
    std::fill (orderedAnalysis.begin(), orderedAnalysis.end(), 0.0f);
    smoothedTargetMidi = 69.0f;
    currentPitchHz = 0.0f;
    targetMidiNote = 0.0f;
    correctionAmount = 0.0f;
    analysisWritePos = 0;
    currentPitchRatio = 1.0f;
}

void AutoTuneEngine::setSettings (const AutoTuneSettings& newSettings)
{
    settings = newSettings;
}

void AutoTuneEngine::process (const float* input, float* output, int numSamples)
{
    if (! settings.enabled)
    {
        if (input != output)
            std::copy (input, input + numSamples, output);
        correctionAmount = 0.0f;
        return;
    }

    const int bufSize = static_cast<int> (analysisBuffer.size());

    for (int i = 0; i < numSamples; ++i)
    {
        analysisBuffer[static_cast<size_t> (analysisWritePos)] = input[i];
        analysisWritePos = (analysisWritePos + 1) % bufSize;
    }

    for (int i = 0; i < bufSize; ++i)
    {
        const int idx = (analysisWritePos + i) % bufSize;
        orderedAnalysis[static_cast<size_t> (i)] = analysisBuffer[static_cast<size_t> (idx)];
    }

    const float detected = detector.process (orderedAnalysis.data(), bufSize);
    currentPitchHz = detected;

    float targetRatio = 1.0f;

    if (detected > 0.0f && detector.getConfidence() > 0.12f)
    {
        const float rawMidi = detector.getDetectedMidiNote() + settings.detuneCents / 100.0f;
        const float quantized = ScaleQuantizer::quantizeMidi (rawMidi,
                                                                settings.rootNote,
                                                                settings.scale,
                                                                settings.toleranceCents);
        const float speed = juce::jlimit (0.05f, 1.0f, settings.retuneSpeed);
        smoothedTargetMidi += (quantized - smoothedTargetMidi) * speed;
        targetMidiNote = smoothedTargetMidi;
        correctionAmount = std::abs (quantized - rawMidi) / 100.0f;

        targetRatio = std::pow (2.0f, (smoothedTargetMidi - rawMidi) / 12.0f);
        targetRatio = juce::jlimit (0.5f, 2.0f, targetRatio);
    }
    else
    {
        correctionAmount = 0.0f;
    }

    const float smooth = juce::jlimit (0.05f, 1.0f, settings.retuneSpeed * 0.35f + 0.15f);
    currentPitchRatio += (targetRatio - currentPitchRatio) * smooth;
    shifter.setPitchRatio (currentPitchRatio);
    shifter.processBlock (input, scratch.data(), numSamples);

    const float wetGain = juce::Decibels::decibelsToGain (4.0f);
    for (int i = 0; i < numSamples; ++i)
        output[i] = scratch[static_cast<size_t> (i)] * wetGain;
}

} // namespace vocalplus
