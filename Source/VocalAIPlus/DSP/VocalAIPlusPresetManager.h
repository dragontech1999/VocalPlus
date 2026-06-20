#pragma once

#include "VocalAIPlusEngine.h"
#include "LocalMasteringAI.h"
#include <vector>

namespace vocalaiplus
{

struct MasterPreset
{
    juce::String name;
    juce::String description;
    VocalGenre genre;
    ChainSettings settings;
};

class VocalAIPlusPresetManager
{
public:
    VocalAIPlusPresetManager();

    int getNumPresets() const noexcept { return static_cast<int> (presets.size()); }
    const MasterPreset& getPreset (int index) const;

private:
    std::vector<MasterPreset> presets;
    void addPreset (MasterPreset preset);
};

} // namespace vocalaiplus
