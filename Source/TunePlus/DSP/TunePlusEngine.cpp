#include "TunePlusEngine.h"

namespace tuneplus
{

void TunePlusEngine::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    harmony.prepare (sampleRate, maxBlockSize, numChannels);
    compressor.prepare (sampleRate, maxBlockSize, numChannels);
    prepared = true;
    reset();
}

void TunePlusEngine::reset()
{
    harmony.reset();
    compressor.reset();
}

void TunePlusEngine::setSettings (const TunePlusSettings& newSettings)
{
    settings = newSettings;
    harmony.setSettings (settings.harmony);
    compressor.setSettings (settings.compressor);
}

void TunePlusEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (! prepared || buffer.getNumSamples() <= 0)
        return;

    harmony.process (buffer);
    compressor.process (buffer);
}

void TunePlusEngine::getStemOutputs (juce::AudioBuffer<float>& dry,
                                     juce::AudioBuffer<float>& corrected,
                                     std::array<juce::AudioBuffer<float>, numHarmonyVoices>& harmonies,
                                     juce::AudioBuffer<float>& blend) const
{
    harmony.getStemOutputs (dry, corrected, harmonies, blend);
}

} // namespace tuneplus
