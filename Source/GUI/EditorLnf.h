

#pragma once

#include <JuceHeader.h>
#include "/Users/eljamarkkanen/Programming/JUCE PROJECTS/CircularDelay/Resources/FuturaMedium.h"
#include "/Users/eljamarkkanen/Programming/JUCE PROJECTS/CircularDelay/Source/GUI/MyColours.h"

class EditorLnf : public juce::LookAndFeel_V4
{
public:
    EditorLnf();

    void drawCornerResizer (juce::Graphics& g, int w, int h, bool isMouseOver, bool isMouseDragging) override;
};
