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
    float setTimeInSamples(double bpm);
            
private:
    void setDelayFilter();
    void setTimeAndMode(int channel);
    
    bool isSync();
    bool isPingPong();
    float getSyncTime();
    float getTime();
    float getFeedback();
    float getWidth();
    float getMix();
    
    
    juce::AudioProcessorValueTreeState& treeState;
    
    float delayInSamples = {0};
    double sampleRate = {44100.f};
    int samplesPerBlock = {512};
    int delayBufferLength = {44100};
    int numChannels = {2};
    
    constexpr static const std::array<float, 13> subdivisions{ 0.25f, (0.5f/3.0f), 0.375f, 0.5f, (1.0f/3.0f), 0.75f, 1.0f, (2.0f/3.0f), 1.5f, 2.0f, (4.0f/3.0f),3.0f, 4.0f};    
    juce::dsp::FirstOrderTPTFilter<double> smoothFilter;
    juce::SmoothedValue<float> smoothFeedback;
    std::array<float, 2> lastDelayOutput;
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine {delayBufferLength};

    juce::dsp::DryWetMixer<float> delayMixer;
    juce::dsp::StateVariableTPTFilter<float> delayFilter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Delay)
};
