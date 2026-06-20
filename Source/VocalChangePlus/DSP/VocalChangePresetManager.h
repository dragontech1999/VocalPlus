#pragma once

#include "VocalChangeEngine.h"
#include <vector>

namespace vocalchangeplus
{

struct VoicePreset
{
    juce::String name;
    juce::String emoji;
    juce::String description;
    VocalChangeSettings settings;
};

class VocalChangePresetManager
{
public:
    VocalChangePresetManager();

    int getNumPresets() const noexcept { return static_cast<int> (presets.size()); }
    const VoicePreset& getPreset (int index) const;

private:
    std::vector<VoicePreset> presets;
    void addPreset (VoicePreset preset);

    static VocalChangeSettings make (float pitch, float formant, float speed,
                                     float distortion, float reverb, float robot, float chorus,
                                     int filter, float correction = 0.0f,
                                     float mix = 1.0f, float aiMorph = 0.0f);
};

} // namespace vocalchangeplus
