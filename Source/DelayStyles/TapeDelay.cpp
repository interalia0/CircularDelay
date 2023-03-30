/*
   ==============================================================================

     TapeDelay.cpp
     Created: 16 Mar 2023 9:41:38am
     Author:  Elja Markkanen

   ==============================================================================
 */

 #include "TapeDelay.h"
 TapeDelay::TapeDelay()
 {
 }

 TapeDelay::~TapeDelay()
 {
 }
 void TapeDelay::prepare(const juce::dsp::ProcessSpec& spec)
 {
     tapeLowpass.prepare(spec);
     tapeHighpass.prepare(spec);

     tapeLowpass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
     tapeLowpass.setCutoffFrequency(5000);
     tapeLowpass.setResonance(0.7);
     tapeHighpass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
     tapeHighpass.setCutoffFrequency(400);
 }

 float TapeDelay::processSample(int channel, float inputValue)
 {
     processedSample = tapeLowpass.processSample(channel, inputValue);
     processedSample = tapeHighpass.processSample(channel, processedSample);

     return processedSample;
 }

