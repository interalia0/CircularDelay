/*
   ==============================================================================

     TapeDelay.h
     Created: 16 Mar 2023 9:41:38am
     Author:  Elja Markkanen

   ==============================================================================
 */

 #pragma once
 #include <JuceHeader.h>
 #include "DelayStyleProcessor.h"


 class TapeDelay : public DelayStyleProcessor
 {

 public:
     TapeDelay();
     ~TapeDelay() override;

     void prepare(const juce::dsp::ProcessSpec& spec) override;
     float processSample(int channel, float inputValue) override;

 private:

     juce::dsp::StateVariableTPTFilter<float> tapeLowpass;
     juce::dsp::StateVariableTPTFilter<float> tapeHighpass;

     float processedSample = 0;

 };
