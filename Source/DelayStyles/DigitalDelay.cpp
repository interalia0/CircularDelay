/*
  ==============================================================================

    DigitalDelay.cpp
    Created: 16 Mar 2023 9:41:48am
    Author:  Elja Markkanen

  ==============================================================================
*/

#include "DigitalDelay.h"

DigitalDelay::DigitalDelay()
{
}

DigitalDelay::~DigitalDelay()
{
}

void DigitalDelay::prepare(const juce::dsp::ProcessSpec& spec)
{
    digitalHighpass.prepare(spec);
    digitalHighpass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    digitalHighpass.setCutoffFrequency(250);
}

float DigitalDelay::processSample(int channel, float inputValue)
{
    processedSample = digitalHighpass.processSample(channel, inputValue);
    
    return processedSample;    
}
