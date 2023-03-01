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
    

    
    float delayInSamples {0};
    double sampleRate = {44100.f};
    int samplesPerBlock = {512};
    float delayBufferLength = 192000;
    int numChannels = {2};
    
//    std::array<juce::SmoothedValue<float, juce::Interpolators::Linear>, 2> smoothDelayTime;
    juce::SmoothedValue<float, juce::Interpolators::WindowedSinc> smoothDelayTime {0};

    std::array<juce::SmoothedValue<float>, 2> smoothMix;
    std::array<juce::SmoothedValue<float>, 2> smoothFeedback;
    std::array<float, 2> lastDelayOutput;
    std::array<juce::dsp::DelayLine<float>, 2> delayLines {{juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayBufferLength), juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> (delayBufferLength)}};
    juce::dsp::DryWetMixer<float> mixer;
    
};
