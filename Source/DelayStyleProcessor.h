/*
  ==============================================================================

    DelayStyleProcessor.h
    Created: 16 Mar 2023 10:04:41am
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class DelayStyleProcessor
{
public:
    virtual ~DelayStyleProcessor() = default;
    virtual void prepare(const juce::dsp::ProcessSpec spec) = 0;
    virtual float processSample(int channel, float inputValue) = 0;
};
