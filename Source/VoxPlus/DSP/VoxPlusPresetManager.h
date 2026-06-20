#pragma once

#include "VoxPlusEngine.h"
#include <vector>

namespace voxplus
{

struct GenrePreset
{
    juce::String name;
    juce::String category;
    juce::String description;
    VoxPlusSettings settings;
};

class VoxPlusPresetManager
{
public:
    VoxPlusPresetManager();

    int getNumPresets() const noexcept { return static_cast<int> (presets.size()); }
    const GenrePreset& getPreset (int index) const;

private:
    std::vector<GenrePreset> presets;
    void addPreset (GenrePreset preset);

    static FxModuleSettings fx (bool on, int mode, float amount, float tone = 0.5f);
    static VoxPlusSettings make (const juce::String& /*tag*/,
                                 float pitch, float formant, float unison, float correction,
                                 int scale,
                                 std::array<FxModuleSettings, 6> modules,
                                 float input = 0.0f, float output = 0.0f,
                                 float glue = 0.0f, float doubling = 0.0f);
};

} // namespace voxplus
