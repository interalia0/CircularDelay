/*
  ==============================================================================

    TapeDelay.h
    Created: 16 Mar 2023 9:41:38am
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DelayStyleProcessor.h"


class TapeDelay : public DelayStyleProcessor
{

public:
    TapeDelay();
    ~TapeDelay() override;

    void prepare(const juce::dsp::ProcessSpec spec) override;
//    void setParameters();
    float processSample(int channel, float inputValue) override;
    
private:
    
    juce::dsp::StateVariableTPTFilter<float> tapeLowpass;
    juce::dsp::StateVariableTPTFilter<float> tapeHighpass;
    
    double sampleRate = 44100;
    int samplesPerBlock = 512;
    int numChannels = 2;
    
    float processedSample = 0;
    
};
