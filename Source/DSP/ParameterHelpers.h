#pragma once

#include <JuceHeader.h>

namespace vocalplus
{

inline juce::NormalisableRange<float> linearRange (float min, float max, float step)
{
    return { min, max, step };
}

inline juce::NormalisableRange<float> bipolarRange (float min, float max, float step)
{
    juce::NormalisableRange<float> range (min, max, step);
    range.setSkewForCentre (0.0f);
    return range;
}

inline juce::NormalisableRange<float> rangedRange (float min, float max, float step)
{
    juce::NormalisableRange<float> range (min, max, step);
    range.setSkewForCentre (min + (max - min) * 0.5f);
    return range;
}

inline float readFloatParam (const juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    auto* param = apvts.getParameter (id);
    if (param == nullptr)
        return 0.0f;

    return apvts.getParameterRange (id).convertFrom0to1 (param->getValue());
}

inline int readIntParam (const juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    if (auto* param = dynamic_cast<juce::AudioParameterInt*> (apvts.getParameter (id)))
        return param->get();

    return static_cast<int> (std::round (readFloatParam (apvts, id)));
}

inline int readChoiceParam (const juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
        return choice->getIndex();

    return static_cast<int> (std::round (readFloatParam (apvts, id)));
}

inline bool readBoolParam (const juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    return apvts.getRawParameterValue (id)->load() > 0.5f;
}

} // namespace vocalplus
