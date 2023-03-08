/*
  ==============================================================================

    DelayLine.cpp
    Created: 1 Mar 2023 10:55:04am
    Author:  Elja Markkanen

  ==============================================================================
*/

#include "Delay.h"

Delay::Delay(juce::AudioProcessorValueTreeState& state) : treeState(state)
{
}

Delay::~Delay()
{
}

void Delay::prepare(juce::dsp::ProcessSpec spec)
{
    sampleRate = spec.sampleRate;
    samplesPerBlock = spec.maximumBlockSize;
    numChannels = spec.numChannels;
        
    delayMixer.prepare(spec);
    
    delayFilter.prepare(spec);
    delayFilter.reset();
    setDelayFilter();
    smoothFilter.prepare(spec);
    smoothFilter.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    
    delayLine.reset();
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(3 * sampleRate);
    
    std::fill (lastDelayOutput.begin(), lastDelayOutput.end(), 0.0f);
}

void Delay::process(juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
    

    float FB = smoothFeedback.getNextValue();
        
    delayMixer.pushDrySamples(input);
        
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto *samplesIn = input.getChannelPointer(channel);
        auto *samplesOut = output.getChannelPointer(channel);
                
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto input = samplesIn[sample] - lastDelayOutput[channel];

            auto smoothTime = smoothFilter.processSample (int (channel), delayInSamples);
            
            delayLine.pushSample(int(channel), input);
            
            auto delayedSample = delayLine.popSample (int (channel), (float) smoothTime, true);
            delayedSample = delayFilter.processSample (int(channel), delayedSample);
            
            samplesOut[sample] = delayedSample;
            
            lastDelayOutput[channel] = samplesOut[sample] * FB * 0.5f;
        }
    }
    delayMixer.mixWetSamples(output);
}

void Delay::setParameters()
{
    float feedback = getFeedback();
    float mix = getMix();
    
    smoothFeedback.setCurrentAndTargetValue(feedback);
    delayMixer.setWetMixProportion(mix);
    smoothFilter.setCutoffFrequency(2);
}

float Delay::setTimeInSamples(double bpm)
{
    const int subdivisionIndex = getSyncTime();
    const float selectedSubdivision = subdivisions[subdivisionIndex];
    
    if (isSync())
    {
        delayInSamples = (60 / bpm) * selectedSubdivision * sampleRate;
    }
    else
    {
        delayInSamples = getTime() / 1000 * sampleRate;
    }
    
    return delayInSamples;
}

void Delay::setDelayFilter()
{
    delayFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    delayFilter.setCutoffFrequency(3500);
}


bool Delay::isSync()
{
    return *treeState.getRawParameterValue("SYNC");
}

float Delay::getSyncTime()
{
    return *treeState.getRawParameterValue("SYNC_TIME");
}

float Delay::getTime()
{
    return *treeState.getRawParameterValue("TIME");
}

float Delay::getFeedback()
{
    return *treeState.getRawParameterValue("FEEDBACK");
}

float Delay::getMix()
{
    return *treeState.getRawParameterValue("MIX");
}


