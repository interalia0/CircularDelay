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
    void processCircular(juce::AudioBuffer<float>& buffer);
    void processStereo(juce::AudioBuffer<float>& buffer);
    void setParameters();
    float setTimeInSamples(double bpm);
    int getMode();
            
private:
    void setDelayFilter();
    void setTimeAndMode(int channel);    
    
    bool isSync();
    float getSyncTime();
    float getTime();
    float getFeedback();
    float getModAmount();
    float getMix();
        
    juce::AudioProcessorValueTreeState& treeState;
    
    float delayInSamples = 0;
    float modulatedDelayInSamples;
    double sampleRate = 44100;
    int samplesPerBlock = 512;
    int delayBufferLength = 44100;
    int numChannels = 2;
    

    float delayedSampleStereo = 0;

    constexpr static const std::array<float, 13> subdivisions{0.25f, (0.5f/3.0f), 0.375f, 0.5f, (1.0f/3.0f), 0.75f, 1.0f, (2.0f/3.0f), 1.5f, 2.0f, (4.0f/3.0f),3.0f, 4.0f};
    
    juce::dsp::FirstOrderTPTFilter<double> smoothFilter;
    juce::SmoothedValue<float> smoothFeedback;
    
    std::array<float, 2> lastDelayOutputStereo;
    std::array<float, 2> lastDelayOutputL;
    std::array<float, 2> lastDelayOutputR;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL {delayBufferLength};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR {delayBufferLength};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayStereo {delayBufferLength};

    juce::dsp::DryWetMixer<float> delayMixer;
    juce::dsp::StateVariableTPTFilter<float> delayFilter;
    juce::dsp::StateVariableTPTFilter<float> delayHighpass;

    
    
    juce::dsp::Chorus<float> wowOsc;   

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Delay)
};
