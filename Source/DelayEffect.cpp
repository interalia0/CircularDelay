/*
  ==============================================================================

    DelayLine.cpp
    Created: 1 Mar 2023 10:55:04am
    Author:  Elja Markkanen

  ==============================================================================
*/

#include "DelayEffect.h"

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

DelayEffect::DelayEffect(juce::AudioProcessorValueTreeState& state) : treeState(state)
{
}

DelayEffect::~DelayEffect()
{
}

void DelayEffect::prepare(const juce::dsp::ProcessSpec spec)
{
    sampleRate = spec.sampleRate;
    samplesPerBlock = spec.maximumBlockSize;
    numChannels = spec.numChannels;
        
    prepareAll(spec, delayL, delayR, delayStereo, delayMixer, wowOsc, tapeDelay, digitalDelay);
    
    setMaxDelay((3 * sampleRate), delayL, delayR, delayStereo);
        
    delayMixer.setMixingRule(juce::dsp::DryWetMixingRule::balanced);
    smoothFilter.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    
//    setDelayFilter();
    setWowOsc();

}

void DelayEffect::reset()
{
    resetAll(delayL, delayR, delayStereo, delayMixer, wowOsc);
        
    std::fill (lastDelayOutputStereo.begin(), lastDelayOutputStereo.end(), 0.0f);
    std::fill (lastDelayOutputL.begin(), lastDelayOutputL.end(), 0.0f);
    std::fill (lastDelayOutputR.begin(), lastDelayOutputR.end(), 0.0f);
}

void DelayEffect::processCircular(juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();
    
    float feedback = smoothFeedback.getNextValue();
    
    delayMixer.pushDrySamples(input);
    
    auto *samplesIn = input.getChannelPointer(0);
    auto *samplesOutL = output.getChannelPointer(0);
    auto *samplesOutR = output.getChannelPointer(1);
    
    for (size_t sample = 0; sample < input.getNumSamples(); ++sample)
    {
        auto delayPingPong = smoothFilter.processSample(0, delayInSamples);
        
        auto input = samplesIn[sample];
        
        delayL.pushSample(0, input + lastDelayOutputR[1] * feedback);
        auto delayedSampleL = delayL.popSample(0, delayPingPong, true);
        delayedSampleL = setStyle(0, delayedSampleL);
                
        delayR.pushSample(1, delayedSampleL * feedback);
        auto delayedSampleR = delayR.popSample(1, delayPingPong, true);
        delayedSampleR = setStyle(1, delayedSampleR);

        samplesOutL[sample] = delayedSampleL;
        lastDelayOutputL[0] = samplesOutL[sample] * feedback;
        
        samplesOutR[sample] = delayedSampleR;
        lastDelayOutputR[1] = samplesOutR[sample] * feedback;
    }
    wowOsc.process(context);
    delayMixer.mixWetSamples(output);
}

void DelayEffect::processStereo(juce::AudioBuffer<float>& buffer)
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
            
            auto inputStereo = samplesInStereo[sample] - lastDelayOutputStereo[channel];
            
            delayStereo.pushSample(int(channel), inputStereo);
            
            auto delayedSampleStereo = delayStereo.popSample(int(channel), delay, true);
            delayedSampleStereo = setStyle(int(channel), delayedSampleStereo);
            
            samplesOutStereo[sample] = delayedSampleStereo;
            lastDelayOutputStereo[channel] = samplesOutStereo[sample] * feedback;
        }
    }
    wowOsc.process(context);
    delayMixer.mixWetSamples(output);
}

void DelayEffect::updateParameters()
{
    float feedback = getFeedback();
    float mix = getMix();
    float modDepth = getModAmount();
    
    smoothFeedback.setCurrentAndTargetValue(feedback);
    delayMixer.setWetMixProportion(mix);
    wowOsc.setDepth(modDepth);
    smoothFilter.setCutoffFrequency(1.8);    
}

float DelayEffect::setStyle(int channel, float sample)
{

    int selectedDelayStyle = getStyle();
    
    if (selectedDelayStyle == 0)
    {
        sample = tapeDelay.processSample(channel, sample);
    }
    
    if (selectedDelayStyle == 1)
    {
        sample = digitalDelay.processSample(channel, sample);
    }
    
    return sample;
}

void DelayEffect::updateTimeInSamples(double bpm)
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
}

constexpr void DelayEffect::setWowOsc()
{
    wowOsc.setCentreDelay(1);
    wowOsc.setMix(1);
    wowOsc.setRate(0.5);
}


bool DelayEffect::isSync() const
{
    return *treeState.getRawParameterValue("SYNC");
}

int DelayEffect::getMode() const
{
    return *treeState.getRawParameterValue("MODE");
}

int DelayEffect::getStyle() const
{
    return *treeState.getRawParameterValue("STYLE");
}

float DelayEffect::getSyncTime() const
{
    return *treeState.getRawParameterValue("SYNC_TIME");
}

float DelayEffect::getTime() const
{
    return *treeState.getRawParameterValue("TIME");
}

float DelayEffect::getFeedback() const
{
    return *treeState.getRawParameterValue("FEEDBACK");
}

float DelayEffect::getModAmount() const
{
    return *treeState.getRawParameterValue("MOD");
}

float DelayEffect::getMix() const
{
    return *treeState.getRawParameterValue("MIX");
}


