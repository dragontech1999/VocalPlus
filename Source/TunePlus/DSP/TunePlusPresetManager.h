#pragma once

#include "TunePlusEngine.h"
#include <vector>

namespace tuneplus
{

struct CompressorPreset
{
    juce::String name;
    juce::String category;
    juce::String description;
    CompressorSettings settings;
};

struct GenrePreset
{
    juce::String name;
    juce::String category;
    juce::String description;
    TunePlusSettings settings;
    int linkedCompressorIndex = 0;
};

class TunePlusPresetManager
{
public:
    TunePlusPresetManager();

    int getNumGenrePresets() const noexcept { return static_cast<int> (genrePresets.size()); }
    int getNumCompressorPresets() const noexcept { return static_cast<int> (compressorPresets.size()); }

    const GenrePreset& getGenrePreset (int index) const;
    const CompressorPreset& getCompressorPreset (int index) const;

    juce::StringArray getGenreCategories() const;
    juce::StringArray getCompressorCategories() const;

    static juce::StringArray getNoteNames();
    static juce::StringArray getScaleNames();

private:
    std::vector<GenrePreset> genrePresets;
    std::vector<CompressorPreset> compressorPresets;

    void addGenrePreset (GenrePreset preset);
    void addCompressorPreset (CompressorPreset preset);

    static vocalplus::HarmonyVoiceSettings voice (bool enabled, int interval, float level, float pan,
                                                  float formant, float delayMs, float vibratoDepth);
    static CompressorSettings comp (bool enabled, float threshold, float ratio, float attack, float release,
                                    float makeup, float knee, float highPass, float mix = 1.0f);
};

} // namespace tuneplus
