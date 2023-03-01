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
    
    
    mixer.reset();
    leftDelay.reset();
    rightDelay.reset();
    
    mixer.prepare(spec);
    leftDelay.prepare(spec);
    rightDelay.prepare(spec);
    
    float feedbackParameter = getFeedback();
    float mixParameter = getMix();
    
    float feedback = feedbackParameter / 100.0f;
    float mix = mixParameter / 100.0f;
    
    for (int channel = 0; channel < 2; ++channel)
    {
        smoothFeedback[channel].reset(samplesPerBlock);
        smoothFeedback[channel].setCurrentAndTargetValue(feedback);
        smoothMix[channel].reset(samplesPerBlock);
        smoothMix[channel].setCurrentAndTargetValue(mix);
    }
    
    std::fill (lastDelayOutputL.begin(), lastDelayOutputL.end(), 0.0f);
    std::fill (lastDelayOutputR.begin(), lastDelayOutputR.end(), 0.0f);
}

void Delay::process(juce::AudioBuffer<float>& buffer)
{
    updateParameters();
    
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
        
    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto *samplesIn = input.getChannelPointer(channel);
        auto *samplesOut = output.getChannelPointer(channel);
        
        float FB = smoothFeedback[channel].getNextValue();
        float MIX = smoothMix[channel].getNextValue();
        
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (channel == 0)
            {
                auto input = samplesIn[sample] - lastDelayOutputL[channel];
                leftDelay.pushSample((int) channel, input);
                
                auto delayedSampleL = leftDelay.popSample((int)channel);
                
                samplesOut[sample] = delayedSampleL + lastDelayOutputL[channel];
                lastDelayOutputL[channel] = samplesOut[sample] * FB * MIX * 0.5f;
            }
            else if (channel == 1)
            {
                auto input = samplesIn[sample] - lastDelayOutputR[channel];
                rightDelay.pushSample((int) channel, input);
            
                auto delayedSampleR = rightDelay.popSample((int)channel);
            
                samplesOut[sample] = delayedSampleR + lastDelayOutputR[channel];
                lastDelayOutputR[channel] = samplesOut[sample] * FB * MIX * 0.5f;
            }
        }
    }
}

void Delay::updateParameters()
{
    float delayTimeInSamples = getTime() / 1000 * sampleRate;
    float feedback = getFeedback();
    float mix = getMix();
    
    for (int channel = 0; channel < 2; ++channel)
    {
        smoothDelayTime[channel].setCurrentAndTargetValue(delayTimeInSamples);
        smoothFeedback[channel].setCurrentAndTargetValue(feedback);
        mixer.setWetMixProportion(mix);
    }
    
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
