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
    smoothDelayTime.reset(sampleRate, 0.05f);
    
    float feedbackParameter = getFeedback();
    float feedback = feedbackParameter / 100.0f;
    
    delayLine.reset();
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(3 * sampleRate);
    
    smoothFeedback.reset(samplesPerBlock);
    smoothFeedback.setCurrentAndTargetValue(feedback);
    
    std::fill (lastDelayOutput.begin(), lastDelayOutput.end(), 0.0f);
}

void Delay::process(juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
        
    delayMixer.pushDrySamples(input);
        
    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float FB = smoothFeedback.getNextValue();
        float TIME = smoothDelayTime.getNextValue();
        
        auto *samplesIn = input.getChannelPointer(channel);
        auto *samplesOut = output.getChannelPointer(channel);
                
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            delayLine.setDelay(TIME);
            
            auto input = samplesIn[sample] - lastDelayOutput[channel];
            
            delayLine.pushSample((int) channel, input);
            
            auto delayedSample = delayLine.popSample((int)channel);
            
            samplesOut[sample] = delayedSample + lastDelayOutput[channel];
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

}

void Delay::setTime(double bpm)
{
    const int subdivisionIndex = getSyncTime();
    const float selectedSubdivision = subdivisions[subdivisionIndex];
    
    if (getSync())
    {
        delayInSamples = (60 / bpm) * selectedSubdivision * sampleRate;
    }
    else
    {
        delayInSamples = getTime() / 1000 * sampleRate;
    }
        smoothDelayTime.setTargetValue(delayInSamples);
}


bool Delay::getSync()
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


