

#pragma once

#include <JuceHeader.h>
#include "GUI/Dial.h"

class EditorContent  : public juce::Component,
                       public Dial::Listener

{
public:
    EditorContent (juce::AudioProcessorValueTreeState& treeState,
                   juce::UndoManager& um);
    
    ~EditorContent();

    void resized() override;
    void dialValueChanged(Dial* dial) override;
private:
    Dial timeDial,
         syncTimeDial,
         syncChoice,
         modeChoice,
         feedbackDial,
         mixDial;
    
//         gainDial;
    
//    juce::ToggleButton syncButton;
//    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncButtonAttachment;
//
//    juce::ComboBox modeChoice;
//    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeChoiceAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorContent)
};
