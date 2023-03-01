/*
  ==============================================================================

    DelayLine.h
    Created: 1 Mar 2023 10:55:04am
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Delay
{
public:
    Delay(juce::AudioProcessorValueTreeState& treeState);
    ~Delay();
    void prepare(juce::dsp::ProcessSpec spec);
    void process(juce::AudioBuffer<float>& buffer);
    void updateParameters();
        
private:
    
    float getTime();
    float getFeedback();
    float getMix();
    
    juce::AudioProcessorValueTreeState& treeState;
    



    std::array<juce::SmoothedValue<float, juce::Interpolators::Linear>, 2> smoothDelayTime;
//    std::array<juce::SmoothedValue<float>, 2> smoothGain;
    std::array<juce::SmoothedValue<float>, 2> smoothMix;
    std::array<juce::SmoothedValue<float>, 2> smoothFeedback;
    std::array<float, 2> lastDelayOutput;
    
    float delayTimeInSamples {0};
    double sampleRate = {44100.f};
    int samplesPerBlock = {512};
    static const int delayBufferLength = 192000;
    int numChannels = {2};
    
    
    
    std::array<float, 2> lastDelayOutputL;
    std::array<float, 2> lastDelayOutputR;
    std::array<float, delayBufferLength> leftDelayLine, rightDelayLine;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> leftDelay{delayBufferLength}, rightDelay{delayBufferLength};
};
