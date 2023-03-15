

#pragma once

#include <JuceHeader.h>
#include "../Resources/FuturaMedium.h"
#include "MyColours.h"

class EditorLnf : public juce::LookAndFeel_V4
{
public:
    EditorLnf();

    void drawCornerResizer (juce::Graphics& g, int w, int h, bool isMouseOver, bool isMouseDragging) override;
};
