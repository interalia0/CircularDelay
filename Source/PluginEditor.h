/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/Dial.h"
#include "GUI/EditorLnf.h"
#include "EditorContent.h"
#include "GUI/MyColours.h"

//==============================================================================
/**
*/
class CircularDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CircularDelayAudioProcessorEditor (CircularDelayAudioProcessor&,
                                       juce::AudioProcessorValueTreeState& treeState,
                                       juce::UndoManager& undoManager);
    ~CircularDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    bool keyPressed (const juce::KeyPress& key) override;
    
private:
    CircularDelayAudioProcessor& audioProcessor;
    
    juce::UndoManager& undoManager;
    
    EditorContent editorContent;

    static constexpr int defaultWidth  { 440 };
    static constexpr int defaultHeight { 280 };

    struct SharedLnf
    {
        SharedLnf()  { juce::LookAndFeel::setDefaultLookAndFeel (&editorLnf); }
        ~SharedLnf() { juce::LookAndFeel::setDefaultLookAndFeel (nullptr); }

        EditorLnf editorLnf;
    };

    juce::SharedResourcePointer<SharedLnf> lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularDelayAudioProcessorEditor)
};
