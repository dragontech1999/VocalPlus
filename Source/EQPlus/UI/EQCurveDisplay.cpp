#include "EQCurveDisplay.h"
#include "../ParameterIDs.h"
#include "../../DSP/ParameterHelpers.h"

namespace eqplus
{

using namespace vocalplus;

EQCurveDisplay::EQCurveDisplay (EQEngine& eng, juce::AudioProcessorValueTreeState& state)
    : engine (eng), apvts (state)
{
    setOpaque (false);
    setMouseCursor (juce::MouseCursor::CrosshairCursor);
    spectrumSmooth.fill (-90.0f);
    startTimerHz (30);
}

void EQCurveDisplay::setDisplayRange (DisplayRange range)
{
    displayRange = range;
    rebuildPaths();
    repaint();
}

void EQCurveDisplay::timerCallback()
{
    const int frame = engine.getSpectrumAnalyzer().getFrameCounter();
    if (frame != lastSpectrumFrame)
    {
        lastSpectrumFrame = frame;
        rebuildPaths();
        repaint();
    }
}

EQCurveDisplay::PlotLayout EQCurveDisplay::layout() const
{
    PlotLayout p;
    p.bounds = getLocalBounds().toFloat().reduced (1.0f);
    p.meterStrip = p.bounds.removeFromRight (44.0f).reduced (6.0f, 10.0f);
    p.graph = p.bounds.reduced (8.0f, 10.0f);
    p.graph.removeFromLeft (42.0f);
    p.graph.removeFromBottom (22.0f);
    return p;
}

float EQCurveDisplay::freqToX (float freq, const juce::Rectangle<float>& graph) const
{
    const float minLog = std::log10 (20.0f);
    const float maxLog = std::log10 (20000.0f);
    const float logF = std::log10 (juce::jlimit (20.0f, 20000.0f, freq));
    return graph.getX() + (logF - minLog) / (maxLog - minLog) * graph.getWidth();
}

float EQCurveDisplay::xToFreq (float x, const juce::Rectangle<float>& graph) const
{
    const float norm = juce::jlimit (0.0f, 1.0f, (x - graph.getX()) / graph.getWidth());
    const float minLog = std::log10 (20.0f);
    const float maxLog = std::log10 (20000.0f);
    return std::pow (10.0f, minLog + norm * (maxLog - minLog));
}

float EQCurveDisplay::gainToY (float gainDb, const juce::Rectangle<float>& graph) const
{
    const float range = displayRangeToDb (displayRange);
    const float norm = juce::jlimit (-1.0f, 1.0f, gainDb / range);
    return graph.getCentreY() - norm * graph.getHeight() * 0.46f;
}

float EQCurveDisplay::yToGain (float y, const juce::Rectangle<float>& graph) const
{
    const float range = displayRangeToDb (displayRange);
    const float norm = (graph.getCentreY() - y) / (graph.getHeight() * 0.46f);
    return juce::jlimit (-range, range, norm * range);
}

float EQCurveDisplay::spectrumDbToY (float db, const juce::Rectangle<float>& graph) const
{
    const float norm = juce::jlimit (0.0f, 1.0f, (db - kSpectrumMinDb) / (kSpectrumMaxDb - kSpectrumMinDb));
    return graph.getBottom() - norm * graph.getHeight();
}

BandSettings EQCurveDisplay::readBand (int index) const
{
    BandSettings b;
    b.enabled = readBoolParam (apvts, ParamIDs::bandEnabled (index));
    b.solo = readBoolParam (apvts, ParamIDs::bandSolo (index));
    b.type = static_cast<FilterType> (readChoiceParam (apvts, ParamIDs::bandType (index)));
    b.channel = static_cast<ChannelMode> (readChoiceParam (apvts, ParamIDs::bandChannel (index)));
    b.frequency = readFloatParam (apvts, ParamIDs::bandFreq (index));
    b.gainDb = readFloatParam (apvts, ParamIDs::bandGain (index));
    b.q = readFloatParam (apvts, ParamIDs::bandQ (index));
    b.dynamicEnabled = readBoolParam (apvts, ParamIDs::bandDynEnabled (index));
    return b;
}

juce::Colour EQCurveDisplay::bandColour (int index) const
{
    static const juce::Colour palette[] = {
        juce::Colour (0xffe040a0),
        juce::Colour (0xff9060ff),
        juce::Colour (0xff30d0c0),
        juce::Colour (0xff4090ff),
        juce::Colour (0xffff6090),
        juce::Colour (0xff70e0ff),
        juce::Colour (0xffffa040),
        juce::Colour (0xff80ff80)
    };
    return palette[static_cast<size_t> (index) % (sizeof (palette) / sizeof (palette[0]))];
}

float EQCurveDisplay::bandShapeDb (const BandSettings& band, float freq) const
{
    if (freq <= 0.0f || band.frequency <= 0.0f)
        return 0.0f;

    const float logRatio = std::log2 (freq / band.frequency);
    const float bw = juce::jmax (0.25f, 1.2f / band.q);
    const float bell = std::exp (-0.5f * (logRatio / bw) * (logRatio / bw));

    switch (band.type)
    {
        case FilterType::LowShelf:  return freq <= band.frequency ? band.gainDb : band.gainDb * bell;
        case FilterType::HighShelf: return freq >= band.frequency ? band.gainDb : band.gainDb * bell;
        default:                    return band.gainDb * bell;
    }
}

int EQCurveDisplay::findBandAt (juce::Point<float> pos, const PlotLayout& plot) const
{
    int closest = -1;
    float closestDist = 14.0f;

    for (int i = 0; i < kMaxBands; ++i)
    {
        const auto b = readBand (i);
        if (! b.enabled)
            continue;

        const float dist = pos.getDistanceFrom ({ freqToX (b.frequency, plot.graph), gainToY (b.gainDb, plot.graph) });
        if (dist < closestDist)
        {
            closestDist = dist;
            closest = i;
        }
    }
    return closest;
}

int EQCurveDisplay::findFreeBand() const
{
    for (int i = 0; i < kMaxBands; ++i)
        if (apvts.getRawParameterValue (ParamIDs::bandEnabled (i))->load() < 0.5f)
            return i;
    return -1;
}

void EQCurveDisplay::rebuildPaths()
{
    const auto plot = layout();
    const auto& graph = plot.graph;

    spectrumFillPath.clear();
    spectrumLinePath.clear();
    curvePath.clear();

    if (apvts.getRawParameterValue (ParamIDs::showPreSpectrum)->load() > 0.5f
        && engine.getSpectrumAnalyzer().getPeakDb (false) > -80.0f)
    {
        const auto& pre = engine.getSpectrumAnalyzer().getSpectrum (false);
        bool started = false;

        for (int i = 3; i < SpectrumAnalyzer::fftSize / 2; i += 2)
        {
            const float freq = engine.getSpectrumAnalyzer().getBinFrequency (i);
            if (freq < 40.0f || freq > 16000.0f)
                continue;

            spectrumSmooth[static_cast<size_t> (i)] += (pre[static_cast<size_t> (i)] - spectrumSmooth[static_cast<size_t> (i)]) * 0.22f;

            const float x = freqToX (freq, graph);
            const float y = spectrumDbToY (spectrumSmooth[static_cast<size_t> (i)], graph);

            if (! started)
            {
                spectrumFillPath.startNewSubPath (x, graph.getBottom());
                spectrumFillPath.lineTo (x, y);
                spectrumLinePath.startNewSubPath (x, y);
                started = true;
            }
            else
            {
                spectrumFillPath.lineTo (x, y);
                spectrumLinePath.lineTo (x, y);
            }
        }

        if (started)
        {
            spectrumFillPath.lineTo (graph.getRight(), graph.getBottom());
            spectrumFillPath.closeSubPath();
        }
    }

    for (int band = 0; band < kMaxBands; ++band)
    {
        auto& path = bandFillPaths[static_cast<size_t> (band)];
        path.clear();

        const auto b = readBand (band);
        if (! b.enabled)
            continue;

        const float f0 = b.frequency;
        const float span = juce::jlimit (0.4f, 2.2f, 1.4f / juce::jmax (0.3f, b.q));
        const float fLow = juce::jlimit (20.0f, 20000.0f, f0 / std::pow (2.0f, span));
        const float fHigh = juce::jlimit (20.0f, 20000.0f, f0 * std::pow (2.0f, span));
        const float zeroY = gainToY (0.0f, graph);

        path.startNewSubPath (freqToX (fLow, graph), zeroY);
        for (float f = fLow; f <= fHigh; f *= 1.04f)
            path.lineTo (freqToX (f, graph), gainToY (bandShapeDb (b, f), graph));
        path.lineTo (freqToX (fHigh, graph), zeroY);
        path.closeSubPath();
    }

    bool started = false;
    for (float x = graph.getX(); x <= graph.getRight(); x += 1.5f)
    {
        const float y = gainToY (engine.getMagnitudeDbAtFrequency (xToFreq (x, graph)), graph);
        if (! started) { curvePath.startNewSubPath (x, y); started = true; }
        else curvePath.lineTo (x, y);
    }
}

void EQCurveDisplay::drawBackground (juce::Graphics& g, const PlotLayout& plot)
{
    g.setColour (juce::Colour (0xff0c0c14));
    g.fillRoundedRectangle (plot.bounds, 8.0f);

    g.setColour (juce::Colour (0xff1a1a28));
    g.fillRoundedRectangle (plot.graph.expanded (4.0f, 4.0f), 6.0f);

    g.setColour (juce::Colour (0xff252535));
    g.drawRoundedRectangle (plot.graph.expanded (4.0f, 4.0f), 6.0f, 1.0f);
}

void EQCurveDisplay::drawGrid (juce::Graphics& g, const PlotLayout& plot)
{
    const auto& graph = plot.graph;
    const float range = displayRangeToDb (displayRange);

    g.setColour (juce::Colour (0x18ffffff));
    for (float db = -range; db <= range + 0.01f; db += range / 4.0f)
        g.drawHorizontalLine (static_cast<int> (gainToY (db, graph)), graph.getX(), graph.getRight());

    const float freqs[] = { 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    for (float f : freqs)
    {
        const float x = freqToX (f, graph);
        g.drawVerticalLine (static_cast<int> (x), graph.getY(), graph.getBottom());
    }

    g.setColour (juce::Colour (0x55ffffff));
    g.drawHorizontalLine (static_cast<int> (gainToY (0.0f, graph)), graph.getX(), graph.getRight());

    g.setFont (juce::FontOptions (10.0f));
    g.setColour (juce::Colour (0xff8888a0));
    for (float db = -range; db <= range + 0.01f; db += range / 2.0f)
    {
        g.drawText (juce::String (static_cast<int> (db)),
                    plot.bounds.getX() + 4.0f, gainToY (db, graph) - 7.0f,
                    36.0f, 14.0f, juce::Justification::centredRight, false);
    }

    for (float f : freqs)
    {
        juce::String label = f >= 1000.0f ? juce::String (static_cast<int> (f / 1000.0f)) + "k"
                                           : juce::String (static_cast<int> (f));
        const float x = freqToX (f, graph);
        g.drawText (label, static_cast<int> (x - 18), static_cast<int> (graph.getBottom() + 6), 36, 14,
                    juce::Justification::centred, false);
    }
}

void EQCurveDisplay::drawSpectrum (juce::Graphics& g, const PlotLayout& plot)
{
    if (spectrumFillPath.isEmpty())
        return;

    juce::ColourGradient grad (juce::Colour (0x00ffffff), 0, plot.graph.getBottom(),
                               juce::Colour (0x35cccccc), 0, plot.graph.getCentreY(), false);
    g.setGradientFill (grad);
    g.fillPath (spectrumFillPath);

    g.setColour (juce::Colour (0x66dddddd));
    g.strokePath (spectrumLinePath, juce::PathStrokeType (1.0f));
}

void EQCurveDisplay::drawBandRegions (juce::Graphics& g, const PlotLayout& plot)
{
    juce::ignoreUnused (plot);

    for (int i = 0; i < kMaxBands; ++i)
    {
        const auto b = readBand (i);
        if (! b.enabled || bandFillPaths[static_cast<size_t> (i)].isEmpty())
            continue;

        const auto colour = bandColour (i);
        const float alpha = (i == selectedBand) ? 0.35f : 0.22f;
        g.setColour (colour.withAlpha (alpha));
        g.fillPath (bandFillPaths[static_cast<size_t> (i)]);
    }
}

void EQCurveDisplay::drawCurve (juce::Graphics& g, const PlotLayout& plot)
{
    juce::ignoreUnused (plot);
    if (curvePath.isEmpty())
        return;

    g.setColour (juce::Colours::white.withAlpha (0.15f));
    g.strokePath (curvePath, juce::PathStrokeType (4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (juce::Colours::white.withAlpha (0.95f));
    g.strokePath (curvePath, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void EQCurveDisplay::drawNodes (juce::Graphics& g, const PlotLayout& plot)
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        const auto b = readBand (i);
        if (! b.enabled)
            continue;

        const float x = freqToX (b.frequency, plot.graph);
        const float y = gainToY (b.gainDb, plot.graph);
        const auto colour = bandColour (i);
        const bool selected = i == selectedBand;

        if (b.dynamicEnabled)
        {
            juce::Path arrow;
            const float dir = b.gainDb >= 0.0f ? -1.0f : 1.0f;
            arrow.addTriangle (x, y + dir * 14.0f, x - 4.0f, y + dir * 6.0f, x + 4.0f, y + dir * 6.0f);
            g.setColour (colour.withAlpha (0.9f));
            g.fillPath (arrow);
        }

        g.setColour (colour.withAlpha (selected ? 0.55f : 0.35f));
        g.fillEllipse (x - (selected ? 8.0f : 6.5f), y - (selected ? 8.0f : 6.5f),
                       selected ? 16.0f : 13.0f, selected ? 16.0f : 13.0f);

        g.setColour (juce::Colours::white.withAlpha (0.95f));
        g.fillEllipse (x - 3.0f, y - 3.0f, 6.0f, 6.0f);
    }
}

void EQCurveDisplay::drawMeters (juce::Graphics& g, const PlotLayout& plot)
{
    const float peak = juce::jlimit (-60.0f, 6.0f, engine.getOutputPeakDb());
    const float norm = juce::jlimit (0.0f, 1.0f, (peak + 60.0f) / 66.0f);

    g.setColour (juce::Colour (0xff1a1a26));
    g.fillRoundedRectangle (plot.meterStrip, 4.0f);

    auto meter = plot.meterStrip.reduced (8.0f, 4.0f);
    g.setColour (juce::Colour (0xff2a2a38));
    g.fillRoundedRectangle (meter, 3.0f);

    auto fill = meter.removeFromBottom (meter.getHeight() * norm);
    juce::ColourGradient grad (juce::Colour (0xff30d0c0), 0, fill.getBottom(),
                               juce::Colour (0xffe040a0), 0, fill.getY(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (fill, 2.0f);

    g.setColour (juce::Colour (0xff8888a0));
    g.setFont (juce::FontOptions (9.0f));
    g.drawText ("OUT", plot.meterStrip.withHeight (14.0f).translated (0, 2.0f),
                juce::Justification::centred, false);
    g.drawText (juce::String (peak, 1), plot.meterStrip.withTrimmedTop (plot.meterStrip.getHeight() - 16.0f),
                juce::Justification::centred, false);
}

void EQCurveDisplay::paint (juce::Graphics& g)
{
    const auto plot = layout();

    drawBackground (g, plot);
    drawGrid (g, plot);

    if (apvts.getRawParameterValue (ParamIDs::showPreSpectrum)->load() > 0.5f)
        drawSpectrum (g, plot);

    drawBandRegions (g, plot);
    drawCurve (g, plot);
    drawNodes (g, plot);
    drawMeters (g, plot);
}

void EQCurveDisplay::resized()
{
    rebuildPaths();
}

void EQCurveDisplay::mouseDown (const juce::MouseEvent& e)
{
    const auto plot = layout();

    draggedBand = findBandAt (e.position, plot);
    if (draggedBand >= 0)
    {
        selectedBand = draggedBand;
        if (onBandSelected) onBandSelected (selectedBand);
        if (auto* p = apvts.getParameter (ParamIDs::selectedBand))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (static_cast<float> (selectedBand)));
        repaint();
        return;
    }

    if (e.mods.isCommandDown())
    {
        const int band = findFreeBand();
        if (band >= 0)
        {
            if (auto* p = apvts.getParameter (ParamIDs::bandEnabled (band))) p->setValueNotifyingHost (1.0f);
            if (auto* p = apvts.getParameter (ParamIDs::bandFreq (band)))
                p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (xToFreq (static_cast<float> (e.x), plot.graph)));
            if (auto* p = apvts.getParameter (ParamIDs::bandGain (band)))
                p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (yToGain (static_cast<float> (e.y), plot.graph)));
            selectedBand = band;
            draggedBand = band;
            if (onBandSelected) onBandSelected (band);
            rebuildPaths();
            repaint();
        }
    }
}

void EQCurveDisplay::mouseDrag (const juce::MouseEvent& e)
{
    if (draggedBand < 0)
        return;

    const auto plot = layout();
    const float freq = xToFreq (static_cast<float> (e.x), plot.graph);
    const float gain = yToGain (static_cast<float> (e.y), plot.graph);

    if (auto* p = apvts.getParameter (ParamIDs::bandFreq (draggedBand)))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (freq));
    if (auto* p = apvts.getParameter (ParamIDs::bandGain (draggedBand)))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (gain));

    if (onBandChanged) onBandChanged (draggedBand, freq, gain);
    rebuildPaths();
    repaint();
}

void EQCurveDisplay::mouseUp (const juce::MouseEvent&)
{
    draggedBand = -1;
}

void EQCurveDisplay::mouseDoubleClick (const juce::MouseEvent& e)
{
    const auto plot = layout();
    const int band = findBandAt (e.position, plot);
    if (band >= 0)
    {
        if (auto* p = apvts.getParameter (ParamIDs::bandEnabled (band)))
            p->setValueNotifyingHost (0.0f);
        rebuildPaths();
        repaint();
    }
}

} // namespace eqplus
