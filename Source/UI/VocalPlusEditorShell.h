#pragma once

#include <JuceHeader.h>

namespace vocalplus::ui
{

/** Shared resize + scroll wrapper for all Vocal+ plugin editors. */
class VocalPlusEditorShell : public juce::Component
{
public:
    static constexpr int kEdgeBarThickness = 10;

    VocalPlusEditorShell (juce::AudioProcessorEditor& editor, juce::Colour accentColour)
        : editorRef (editor), accent (accentColour)
    {
        viewport.setScrollBarsShown (true, true);
        viewport.setScrollBarThickness (12);
        addAndMakeVisible (viewport);
        addAndMakeVisible (rightBar);
        addAndMakeVisible (bottomBar);

        rightBar.edge = EdgeBar::Edge::Right;
        bottomBar.edge = EdgeBar::Edge::Bottom;
        rightBar.shell = this;
        bottomBar.shell = this;
    }

    void setScrollContent (juce::Component* content, int width, int height)
    {
        contentWidth = width;
        contentHeight = height;
        viewport.setViewedComponent (content, false);

        if (content != nullptr)
            content->setSize (contentWidth, contentHeight);

        resized();
    }

    juce::Viewport& getViewport() noexcept { return viewport; }

    static void configureEditorWindow (juce::AudioProcessorEditor& editor,
                                      int contentWidth,
                                      int contentHeight,
                                      int minVisibleHeight = 400)
    {
        editor.setResizable (true, false);

        const int minW = juce::jmax (480, contentWidth / 2);
        const int maxW = contentWidth * 2;
        const int minH = juce::jmax (320, minVisibleHeight);
        const int maxH = contentHeight + 320;

        editor.setResizeLimits (minW, minH, maxW, maxH);
        editor.setSize (contentWidth + kEdgeBarThickness,
                        contentHeight + kEdgeBarThickness);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff080808));
    }

    void resized() override
    {
        auto area = getLocalBounds();
        rightBar.setBounds (area.removeFromRight (kEdgeBarThickness));
        bottomBar.setBounds (area.removeFromBottom (kEdgeBarThickness));
        viewport.setBounds (area);

        if (viewport.getViewedComponent() != nullptr)
            viewport.getViewedComponent()->setSize (contentWidth, contentHeight);
    }

private:
    struct EdgeBar : public juce::Component
    {
        enum class Edge { Right, Bottom };
        Edge edge = Edge::Right;
        VocalPlusEditorShell* shell = nullptr;

        void paint (juce::Graphics& g) override
        {
            if (shell == nullptr)
                return;

            const auto bounds = getLocalBounds().toFloat();
            g.setColour (juce::Colour (0xff141814));
            g.fillRect (bounds);

            g.setColour (shell->accent.withAlpha (hovered ? 0.85f : 0.55f));
            if (edge == Edge::Right)
                g.fillRect (bounds.getRight() - 2.0f, bounds.getY() + 8.0f, 2.0f, bounds.getHeight() - 16.0f);
            else
                g.fillRect (bounds.getX() + 8.0f, bounds.getBottom() - 2.0f, bounds.getWidth() - 16.0f, 2.0f);

            g.setColour (shell->accent.withAlpha (0.25f));
            if (edge == Edge::Right)
            {
                for (float y = 10.0f; y < bounds.getHeight() - 10.0f; y += 8.0f)
                    g.fillRect (bounds.getCentreX() - 1.0f, y, 2.0f, 3.0f);
            }
            else
            {
                for (float x = 10.0f; x < bounds.getWidth() - 10.0f; x += 8.0f)
                    g.fillRect (x, bounds.getCentreY() - 1.0f, 3.0f, 2.0f);
            }
        }

        void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit (const juce::MouseEvent&) override { hovered = false; repaint(); }

        void mouseDown (const juce::MouseEvent& e) override
        {
            dragStart = e.getScreenPosition();
            if (shell != nullptr)
            {
                startWidth = shell->editorRef.getWidth();
                startHeight = shell->editorRef.getHeight();
            }
        }

        void mouseDrag (const juce::MouseEvent& e) override
        {
            if (shell == nullptr)
                return;

            const auto delta = e.getScreenPosition() - dragStart;
            auto& ed = shell->editorRef;

            if (edge == Edge::Right)
                ed.setSize (startWidth + delta.getX(), ed.getHeight());
            else
                ed.setSize (ed.getWidth(), startHeight + delta.getY());
        }

        juce::Point<int> dragStart;
        int startWidth = 0;
        int startHeight = 0;
        bool hovered = false;
    };

    juce::AudioProcessorEditor& editorRef;
    juce::Colour accent;
    juce::Viewport viewport;
    EdgeBar rightBar, bottomBar;
    int contentWidth = 1000;
    int contentHeight = 700;
};

} // namespace vocalplus::ui
