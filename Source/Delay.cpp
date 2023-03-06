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
    setFilter();

    delayLine.reset();
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(3 * sampleRate);
    
    smoothDelayTime.reset(300 * sampleRate * 1000);
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
        setTimeAndMode(channel);
        
        auto *samplesIn = input.getChannelPointer(channel);
        auto *samplesOut = output.getChannelPointer(channel);
                
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto input = samplesIn[sample] - lastDelayOutput[channel];
            
            delayLine.pushSample((int)channel, input);
            
            auto delayedSample = delayLine.popSample((int)channel);
            delayedSample = delayFilter.processSample((int)channel, delayedSample);
            
            samplesOut[sample] = delayedSample + lastDelayOutput[channel];
            
            lastDelayOutput[channel] = samplesOut[sample] * FB * 0.5f;
        }
    }
    delayMixer.mixWetSamples(output);
}

void Delay::setParameters()
{
    float feedback = getFeedback();
    float width = getWidth();
    float mix = getMix();
    
    smoothFeedback.setCurrentAndTargetValue(feedback);
    smoothWidth.setCurrentAndTargetValue(width);
    delayMixer.setWetMixProportion(mix);
}

void Delay::setTimeInSamples(double bpm)
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
        smoothDelayTime.setTargetValue(delayInSamples);
}

void Delay::setTimeAndMode(int channel)
{
    if (isPingPong())
    {
        if (channel == 0)
        {
            delayLine.setDelay(smoothDelayTime.getNextValue() * 2);
        }
        if (channel == 1)
        {
            delayLine.setDelay(smoothDelayTime.getNextValue());
        }
    }
    
    else
    {
        if (channel == 0)
        {
            delayLine.setDelay(smoothDelayTime.getNextValue());
        }
        if (channel == 1)
        {
            delayLine.setDelay(smoothDelayTime.getNextValue() + getWidth());
        }
    }
}

void Delay::setFilter()
{
    delayFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    delayFilter.setCutoffFrequency(2500);
}


bool Delay::isSync()
{
    return *treeState.getRawParameterValue("SYNC");
}

bool Delay::isPingPong()
{
    return *treeState.getRawParameterValue("MODE");
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

float Delay::getWidth()
{
    return *treeState.getRawParameterValue("WIDTH");
}

float Delay::getMix()
{
    return *treeState.getRawParameterValue("MIX");
}


