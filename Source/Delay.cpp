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

template <typename Func, typename... Items>
constexpr void forEach (Func&& func, Items&&... items)
{
    (func (std::forward<Items> (items)), ...);
}

template <typename... Processors>
void prepareAll (const juce::dsp::ProcessSpec& spec, Processors&... processors)
{
    forEach([&] (auto& proc) { proc.prepare (spec); }, processors...);
}

template <typename... Processors>
void resetAll (Processors&... processors)
{
    forEach([] (auto& proc) { proc.reset(); }, processors...);
}

template <typename... DelayLines>
void setMaxDelay (int maxDelayInSamples, DelayLines&... delayLines)
{
    forEach([&] (auto& delayLine) { delayLine.setMaximumDelayInSamples (maxDelayInSamples); }, delayLines...);
}

void Delay::prepare(const juce::dsp::ProcessSpec spec)
{
    sampleRate = spec.sampleRate;
    samplesPerBlock = spec.maximumBlockSize;
    numChannels = spec.numChannels;
        
    prepareAll(spec, delayL, delayR, delayStereo, delayFilter, delayHighpass, delayMixer, wowOsc);
    
    setMaxDelay((3 * sampleRate), delayL, delayR, delayStereo);
        
    delayMixer.setMixingRule(juce::dsp::DryWetMixingRule::balanced);
    smoothFilter.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    
    setDelayFilter();
    setWowOsc();

}

void Delay::reset()
{
    resetAll(delayL, delayR, delayStereo, delayFilter, delayHighpass, delayMixer, wowOsc);
        
    std::fill (lastDelayOutputStereo.begin(), lastDelayOutputStereo.end(), 0.0f);
    std::fill (lastDelayOutputL.begin(), lastDelayOutputL.end(), 0.0f);
    std::fill (lastDelayOutputR.begin(), lastDelayOutputR.end(), 0.0f);
}

void Delay::processCircular(juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
    
    float feedback = smoothFeedback.getNextValue();
    
    delayMixer.pushDrySamples(input);
    
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
        delayedSampleL = delayHighpass.processSample(0, delayedSampleL);
        
        auto delayedSampleR = delayR.popSample(1, delayPingPong * 2, true);
        delayedSampleR = delayFilter.processSample(1, delayedSampleR);
        delayedSampleR = delayHighpass.processSample(1, delayedSampleR);
        
        samplesOutL[sample] = delayedSampleL;
        lastDelayOutputL[0] = samplesOutL[sample] * feedback;
        samplesOutR[sample] = delayedSampleR;
        lastDelayOutputR[1] = samplesOutR[sample] * feedback;
    }
    wowOsc.process(context);
    delayMixer.mixWetSamples(output);
}

void Delay::processStereo(juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
    
    float feedback = smoothFeedback.getNextValue();
    
    delayMixer.pushDrySamples(input);
    
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
    wowOsc.process(context);
    delayMixer.mixWetSamples(output);
}

void Delay::updateParameters()
{
    float feedback = getFeedback();
    float mix = getMix();
    float modDepth = getModAmount();
    
    smoothFeedback.setCurrentAndTargetValue(feedback);
    delayMixer.setWetMixProportion(mix);
    wowOsc.setDepth(modDepth);
    smoothFilter.setCutoffFrequency(1.8);
}

float Delay::updateTimeInSamples(double bpm)
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
    delayFilter.setCutoffFrequency(5000);
    delayFilter.setResonance(0.7);
    delayHighpass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    delayHighpass.setCutoffFrequency(400);
}

constexpr void Delay::setWowOsc()
{
    wowOsc.setCentreDelay(1);
    wowOsc.setMix(1);
    wowOsc.setRate(0.5);
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

float Delay::getModAmount()
{
    return *treeState.getRawParameterValue("MOD");
}

float Delay::getMix()
{
    return *treeState.getRawParameterValue("MIX");
}
