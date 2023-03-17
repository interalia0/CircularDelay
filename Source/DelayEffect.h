/*
  ==============================================================================

    DelayLine.h
    Created: 1 Mar 2023 10:55:04am
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DelayStyleProcessor.h"
#include "TapeDelay.h"
#include "DigitalDelay.h"

class DelayEffect
{
public:
    DelayEffect(juce::AudioProcessorValueTreeState& treeState);
    ~DelayEffect();
    
    void prepare(const juce::dsp::ProcessSpec spec);
    void reset();
    void processCircular(juce::AudioBuffer<float>& buffer);
    void processStereo(juce::AudioBuffer<float>& buffer);
    
    void updateParameters();
    void updateTimeInSamples(double bpm);
    int getMode() const;
            
private:
    
    void setTimeAndMode(int channel);
    constexpr void setWowOsc();
    float setStyle(int channel, float sample);
    
    bool isSync() const;
    float getSyncTime() const;
    float getTime() const;
    int getStyle() const;
    float getFeedback() const;
    float getModAmount() const;
    float getMix() const;
        
    juce::AudioProcessorValueTreeState& treeState;
    juce::dsp::ProcessSpec spec;
    
    float delayInSamples = 0;
    double sampleRate = 44100;
    int samplesPerBlock = 512;
    int delayBufferLength = 44100;
    int numChannels = 2;
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL {delayBufferLength};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR {delayBufferLength};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayStereo {delayBufferLength};
    
    juce::dsp::DryWetMixer<float> delayMixer;
    juce::dsp::Chorus<float> wowOsc;
    
    TapeDelay tapeDelay;
    DigitalDelay digitalDelay;
    
    constexpr static const std::array<float, 13> subdivisions{0.25f, (0.5f/3.0f), 0.375f, 0.5f, (1.0f/3.0f), 0.75f, 1.0f, (2.0f/3.0f), 1.5f, 2.0f, (4.0f/3.0f),3.0f, 4.0f};
    
    juce::dsp::FirstOrderTPTFilter<double> smoothFilter;
    juce::SmoothedValue<float> smoothFeedback;
    
    std::array<float, 2> lastDelayOutputStereo;
    std::array<float, 2> lastDelayOutputL;
    std::array<float, 2> lastDelayOutputR;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayEffect)
};
