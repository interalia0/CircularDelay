/*
  ==============================================================================

    DigitalDelay.h
    Created: 16 Mar 2023 9:41:48am
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include "DelayStyleProcessor.h"


class DigitalDelay : public DelayStyleProcessor
{

public:
    DigitalDelay();
    ~DigitalDelay() override;

    void prepare(const juce::dsp::ProcessSpec spec) override;
    float processSample(int channel, float inputValue) override;
    
private:
    
    juce::dsp::StateVariableTPTFilter<float> digitalHighpass;
    
    double sampleRate = 44100;
    int samplesPerBlock = 512;
    int numChannels = 2;
    
    float processedSample = 0;
    
};
