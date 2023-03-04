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
    
    void setParameters();
    float setTime(double bpm);



        
private:
    bool getSync();
    float getSyncTime();
    float getTime();
    float getFeedback();
    float getMix();
    
    
    juce::AudioProcessorValueTreeState& treeState;
    
    
    float delayInSamples {0};
    double sampleRate = {44100.f};
    int samplesPerBlock = {512};
    float delayBufferLength = {44100};
    int numChannels = {2};
    
    constexpr static const std::array<float, 13> subdivisions{ 0.25f, (0.5f/3.0f), 0.375f, 0.5f, (1.0f/3.0f), 0.75f, 1.0f, (2.0f/3.0f), 1.5f, 2.0f, (4.0f/3.0f),3.0f, 4.0f};
    juce::SmoothedValue<float, juce::Interpolators::WindowedSinc> smoothDelayTime {0};
    std::array<juce::SmoothedValue<float>, 2> smoothMix;
    std::array<juce::SmoothedValue<float>, 2> smoothFeedback;
    std::array<float, 2> lastDelayOutput;
    
    std::array<juce::dsp::DelayLine<float>, 2> delayLines {{juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayBufferLength), juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> (delayBufferLength)}};
    juce::dsp::DryWetMixer<float> mixer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Delay)
};
