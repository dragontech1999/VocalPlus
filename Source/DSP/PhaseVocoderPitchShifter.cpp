#include "PhaseVocoderPitchShifter.h"

namespace vocalplus
{

void PhaseVocoderPitchShifter::prepare (double sr, int maxBlockSize)
{
    sampleRate = sr;
    inputFifo.assign (static_cast<size_t> (fftSize * 2), 0.0f);
    outputFifo.assign (static_cast<size_t> (fftSize * 4), 0.0f);
    frame.assign (static_cast<size_t> (fftSize), 0.0f);
    lastPhase.assign (static_cast<size_t> (fftSize / 2 + 1), 0.0f);
    sumPhase.assign (static_cast<size_t> (fftSize / 2 + 1), 0.0f);
    magnitudes.assign (static_cast<size_t> (fftSize / 2 + 1), 0.0f);
    frequencies.assign (static_cast<size_t> (fftSize / 2 + 1), 0.0f);
    spectrum.assign (static_cast<size_t> (fftSize), {});
    ifftBuffer.assign (static_cast<size_t> (fftSize), {});
    processScratch.assign (static_cast<size_t> (maxBlockSize), 0.0f);
    reset();
}

void PhaseVocoderPitchShifter::reset()
{
    std::fill (inputFifo.begin(), inputFifo.end(), 0.0f);
    std::fill (outputFifo.begin(), outputFifo.end(), 0.0f);
    std::fill (lastPhase.begin(), lastPhase.end(), 0.0f);
    std::fill (sumPhase.begin(), sumPhase.end(), 0.0f);
    inputWritePos = 0;
    outputReadPos = 0;
    outputWritePos = fftSize;
    samplesSinceLastFrame = 0;
}

void PhaseVocoderPitchShifter::setPitchRatio (float ratio)
{
    pitchRatio = juce::jlimit (0.5f, 2.0f, ratio);
}

void PhaseVocoderPitchShifter::setFormantRatio (float ratio)
{
    formantRatio = juce::jlimit (0.5f, 2.0f, ratio);
}

void PhaseVocoderPitchShifter::processFrame()
{
    const int fifoSize = static_cast<int> (inputFifo.size());
    int readStart = inputWritePos - fftSize;
    while (readStart < 0)
        readStart += fifoSize;

    for (int i = 0; i < fftSize; ++i)
        frame[static_cast<size_t> (i)] = inputFifo[static_cast<size_t> ((readStart + i) % fifoSize)];

    window.multiplyWithWindowingTable (frame.data(), static_cast<size_t> (fftSize));

    for (int i = 0; i < fftSize; ++i)
        spectrum[static_cast<size_t> (i)] = { frame[static_cast<size_t> (i)], 0.0f };

    fft.perform (spectrum.data(), ifftBuffer.data(), false);

    const float freqPerBin = static_cast<float> (sampleRate / fftSize);
    const float expectedPhaseAdvance = juce::MathConstants<float>::twoPi * static_cast<float> (hopSize) / static_cast<float> (fftSize);

    for (int bin = 0; bin <= fftSize / 2; ++bin)
    {
        const auto c = ifftBuffer[static_cast<size_t> (bin)];
        magnitudes[static_cast<size_t> (bin)] = std::abs (c);
        const float phase = std::arg (c);
        const float delta = phase - lastPhase[static_cast<size_t> (bin)];
        lastPhase[static_cast<size_t> (bin)] = phase;

        float deltaUnwrapped = delta - static_cast<float> (bin) * expectedPhaseAdvance;
        while (deltaUnwrapped > juce::MathConstants<float>::pi) deltaUnwrapped -= juce::MathConstants<float>::twoPi;
        while (deltaUnwrapped < -juce::MathConstants<float>::pi) deltaUnwrapped += juce::MathConstants<float>::twoPi;

        sumPhase[static_cast<size_t> (bin)] += deltaUnwrapped + static_cast<float> (bin) * expectedPhaseAdvance;
        frequencies[static_cast<size_t> (bin)] = static_cast<float> (bin) * freqPerBin
            + deltaUnwrapped * static_cast<float> (sampleRate) / (juce::MathConstants<float>::twoPi * static_cast<float> (hopSize));
    }

    std::fill (ifftBuffer.begin(), ifftBuffer.end(), std::complex<float> {});

    for (int bin = 0; bin <= fftSize / 2; ++bin)
    {
        const int pitchBin = juce::jlimit (0, fftSize / 2,
                                           static_cast<int> (std::round (static_cast<float> (bin) / pitchRatio)));
        const int formantBin = juce::jlimit (0, fftSize / 2,
                                             static_cast<int> (std::round (static_cast<float> (bin) / formantRatio)));
        const float mag = magnitudes[static_cast<size_t> (formantBin)];
        const float phase = sumPhase[static_cast<size_t> (pitchBin)];

        ifftBuffer[static_cast<size_t> (bin)] = std::polar (mag, phase);

        if (bin > 0 && bin < fftSize / 2)
            ifftBuffer[static_cast<size_t> (fftSize - bin)] = std::conj (ifftBuffer[static_cast<size_t> (bin)]);
    }

    fft.perform (ifftBuffer.data(), spectrum.data(), true);

    for (int i = 0; i < fftSize; ++i)
        frame[static_cast<size_t> (i)] = spectrum[static_cast<size_t> (i)].real() / static_cast<float> (fftSize);

    for (int i = 0; i < hopSize; ++i)
    {
        const int outIndex = (outputWritePos + i) % static_cast<int> (outputFifo.size());
        outputFifo[static_cast<size_t> (outIndex)] += frame[static_cast<size_t> (i)];
    }

    outputWritePos = (outputWritePos + hopSize) % static_cast<int> (outputFifo.size());
}

void PhaseVocoderPitchShifter::processBlock (const float* input, float* output, int numSamples)
{
    const float* src = input;

    if (input == output)
    {
        if (static_cast<int> (processScratch.size()) < numSamples)
            processScratch.assign (static_cast<size_t> (numSamples), 0.0f);

        std::copy (input, input + numSamples, processScratch.data());
        src = processScratch.data();
    }

    for (int i = 0; i < numSamples; ++i)
    {
        inputFifo[static_cast<size_t> (inputWritePos)] = src[i];
        inputWritePos = (inputWritePos + 1) % static_cast<int> (inputFifo.size());

        ++samplesSinceLastFrame;

        if (samplesSinceLastFrame >= hopSize)
        {
            processFrame();
            samplesSinceLastFrame = 0;
        }

        output[i] = outputFifo[static_cast<size_t> (outputReadPos)];
        outputFifo[static_cast<size_t> (outputReadPos)] = 0.0f;
        outputReadPos = (outputReadPos + 1) % static_cast<int> (outputFifo.size());
    }
}

} // namespace vocalplus
