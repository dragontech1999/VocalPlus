#pragma once

#include <JuceHeader.h>
#include "../DSP/EQEngine.h"
#include "../DSP/EQTypes.h"
#include "../DSP/SpectrumAnalyzer.h"
#include "EQPlusLookAndFeel.h"

namespace eqplus
{

class EQCurveDisplay : public juce::Component,
                       private juce::Timer
{
public:
    EQCurveDisplay (EQEngine& engine, juce::AudioProcessorValueTreeState& apvts);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

    void setDisplayRange (DisplayRange range);
    int getSelectedBand() const noexcept { return selectedBand; }

    std::function<void (int bandIndex)> onBandSelected;
    std::function<void (int bandIndex, float freq, float gain)> onBandChanged;
    std::function<void (const std::vector<std::pair<float, float>>&)> onSketchComplete;
    std::function<void (int bandIndex, float freq, float gainDb, bool dynamic)> onSpectrumGrab;

private:
    struct PlotLayout
    {
        juce::Rectangle<float> bounds;
        juce::Rectangle<float> graph;
        juce::Rectangle<float> meterStrip;
    };

    void timerCallback() override;
    void rebuildPaths();
    PlotLayout layout() const;
    float freqToX (float freq, const juce::Rectangle<float>& graph) const;
    float xToFreq (float x, const juce::Rectangle<float>& graph) const;
    float gainToY (float gainDb, const juce::Rectangle<float>& graph) const;
    float yToGain (float y, const juce::Rectangle<float>& graph) const;
    float spectrumDbToY (float db, const juce::Rectangle<float>& graph) const;
    BandSettings readBand (int index) const;
    int findBandAt (juce::Point<float> pos, const PlotLayout& plot) const;
    int findFreeBand() const;
    juce::Colour bandColour (int index) const;
    float bandShapeDb (const BandSettings& band, float freq) const;

    void drawBackground (juce::Graphics& g, const PlotLayout& plot);
    void drawGrid (juce::Graphics& g, const PlotLayout& plot);
    void drawSpectrum (juce::Graphics& g, const PlotLayout& plot);
    void drawBandRegions (juce::Graphics& g, const PlotLayout& plot);
    void drawCurve (juce::Graphics& g, const PlotLayout& plot);
    void drawNodes (juce::Graphics& g, const PlotLayout& plot);
    void drawMeters (juce::Graphics& g, const PlotLayout& plot);

    EQEngine& engine;
    juce::AudioProcessorValueTreeState& apvts;

    DisplayRange displayRange = DisplayRange::Range12dB;
    int selectedBand = 0;
    int draggedBand = -1;
    int lastSpectrumFrame = -1;

    static constexpr float kSpectrumMinDb = -54.0f;
    static constexpr float kSpectrumMaxDb = 6.0f;

    juce::Path spectrumFillPath;
    juce::Path spectrumLinePath;
    juce::Path curvePath;
    std::array<juce::Path, kMaxBands> bandFillPaths;
    std::array<float, SpectrumAnalyzer::fftSize / 2> spectrumSmooth {};
};

} // namespace eqplus
