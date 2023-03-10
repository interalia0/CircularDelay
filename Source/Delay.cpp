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
    delayMixer.setMixingRule(juce::dsp::DryWetMixingRule::balanced);
    
    smoothFilter.prepare(spec);
    smoothFilter.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    
    delayL.reset();
    delayL.prepare(spec);
    delayL.setMaximumDelayInSamples(3 * sampleRate);
    delayR.reset();
    delayR.prepare(spec);
    delayR.setMaximumDelayInSamples(3 * sampleRate);
    
    delayStereo.reset();
    delayStereo.prepare(spec);
    delayStereo.setMaximumDelayInSamples(3 * sampleRate);
        
    delayFilter.prepare(spec);
    delayFilter.reset();
    setDelayFilter();
    
    std::fill (lastDelayOutputStereo.begin(), lastDelayOutputStereo.end(), 0.0f);
    std::fill (lastDelayOutputL.begin(), lastDelayOutputL.end(), 0.0f);
    std::fill (lastDelayOutputR.begin(), lastDelayOutputR.end(), 0.0f);
}

void Delay::process(juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
    
    float feedback = smoothFeedback.getNextValue();
    
    delayMixer.pushDrySamples(input);
    
    if (numChannels == 1 && getMode() == modePingPong)
    {
//        treeState.getParameter("MODE")->beginChangeGesture();
        treeState.getParameter("MODE")->setValueNotifyingHost(0);
//        treeState.getParameter("MODE")->endChangeGesture();
    }
    
    if (getMode() == modePingPong && numChannels > 1)
    {
        auto *samplesInL = input.getChannelPointer(0);
        auto *samplesOutL = output.getChannelPointer(0);
        auto *samplesInR = input.getChannelPointer(1);
        auto *samplesOutR = output.getChannelPointer(1);
        
        for (size_t sample = 0; sample < input.getNumSamples(); ++sample)
        {
            auto delayPingPong = smoothFilter.processSample(0, delayInSamples);

            auto inputL = samplesInL[sample];
            auto inputR = samplesInR[sample];
            
            delayL.pushSample(0, inputR + lastDelayOutputR[1] * feedback);
            delayR.pushSample(1, inputL + lastDelayOutputL[0] * feedback);
            
            auto delayedSampleL = delayL.popSample(0, delayPingPong, true);
            delayedSampleL = delayFilter.processSample(0, delayedSampleL);
            
            auto delayedSampleR = delayR.popSample(1, delayPingPong * 2, true);
            delayedSampleR = delayFilter.processSample(1, delayedSampleR);
            
            samplesOutL[sample] = delayedSampleL;
            lastDelayOutputL[0] = samplesOutL[sample] * feedback;
            
            samplesOutR[sample] = delayedSampleR;
            lastDelayOutputR[1] = samplesOutR[sample] * feedback;
        }
    }
    
    if (getMode() == modeStereo || numChannels == 1)
    {
        for (size_t channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto *samplesInStereo = input.getChannelPointer(channel);
            auto *samplesOutStereo = output.getChannelPointer(channel);
            
            for (size_t sample = 0; sample < input.getNumSamples(); ++sample)
            {
                auto delay = smoothFilter.processSample(int(channel), delayInSamples);
                
                auto input = samplesInStereo[sample] - lastDelayOutputStereo[channel];
                
                delayStereo.pushSample(int(channel), input);
                
                auto delayedSampleStereo = delayStereo.popSample(int(channel), delay, true);
                delayedSampleStereo = delayFilter.processSample(int(channel), delayedSampleStereo);
                
                samplesOutStereo[sample] = delayedSampleStereo;
                lastDelayOutputStereo[channel] = samplesOutStereo[sample] * feedback;
            }
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
    smoothFilter.setCutoffFrequency(1.8);
}

float Delay::setTimeInSamples(double bpm)
{
    const int subdivisionIndex = getSyncTime();
    const float selectedSubdivision = subdivisions[subdivisionIndex];
    
    if (isSync())
    {
        delayInSamples = (60 / bpm) * selectedSubdivision * sampleRate;
        float delayInMillisec =  delayInSamples / sampleRate * 1000.0f;
        treeState.getParameter("TIME")->setValueNotifyingHost(juce::NormalisableRange<float>(1.0f, 3000.0f, 1.0f).convertTo0to1(delayInMillisec));
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
    delayFilter.setCutoffFrequency(5500);
}


bool Delay::isSync()
{
    return *treeState.getRawParameterValue("SYNC");
}

int Delay::getMode()
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

float Delay::getMix()
{
    return *treeState.getRawParameterValue("MIX");
}
