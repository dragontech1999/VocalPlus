#pragma once

#include "HarmonyEngine.h"
#include <vector>

namespace vocalplus
{

struct FactoryPreset
{
    juce::String name;
    juce::String category;
    juce::String description;
    HarmonyEngineSettings settings;
};

class PresetManager
{
public:
    PresetManager();

    const std::vector<FactoryPreset>& getPresets() const noexcept { return presets; }
    int getNumPresets() const noexcept { return static_cast<int> (presets.size()); }

    const FactoryPreset& getPreset (int index) const;
    int findPresetIndexByName (const juce::String& name) const;

    static juce::StringArray getNoteNames();
    static juce::StringArray getScaleNames();

private:
    std::vector<FactoryPreset> presets;
    void addPreset (FactoryPreset preset);
    static HarmonyVoiceSettings voice (bool enabled, int interval, float level, float pan,
                                       float formant, float delayMs, float vibratoDepth);
};

} // namespace vocalplus
