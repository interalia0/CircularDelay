

#include "EditorContent.h"
#include "GUI/EditorLnf.h"
#include "GUI/MyColours.h"

EditorContent::EditorContent (juce::AudioProcessorValueTreeState& treeState,
                              juce::UndoManager& um)
    : timeDial     (*treeState.getParameter ("TIME"), &um),
      syncTimeDial (*treeState.getParameter ("SYNC_TIME"), &um),
      syncChoice   (*treeState.getParameter ("SYNC"), &um),
      modeChoice   (*treeState.getParameter ("MODE"), &um),
      feedbackDial (*treeState.getParameter ("FEEDBACK"), &um),
      modDial      (*treeState.getParameter ("MOD"), &um),
      styleChoice  (*treeState.getParameter ("STYLE"), &um),
      mixDial      (*treeState.getParameter ("MIX"), &um)
{

    setWantsKeyboardFocus (true);

    timeDial.setInterval (5.0f);
    timeDial.setFineInterval (1.0f);
    syncTimeDial.setInterval (1.0f);
    syncTimeDial.setFineInterval (0.1f);
    syncChoice.setInterval(1);    
    styleChoice.setInterval(1);

    addAndMakeVisible (syncChoice);
    addAndMakeVisible (modeChoice);
    addAndMakeVisible (timeDial);
    addAndMakeVisible (feedbackDial);
    addAndMakeVisible (syncTimeDial);
    addAndMakeVisible (modDial);
    addAndMakeVisible (styleChoice);
    addAndMakeVisible (mixDial);
    
    syncChoice.addListener(this);
    
    syncChoice.setColour(Dial::foregroundArcColourId, juce::Colours::red);
    modeChoice.setColour(Dial::foregroundArcColourId, juce::Colours::red);
}

EditorContent::~EditorContent()
{
    syncChoice.removeListener(this);
}

void EditorContent::resized()
{
    timeDial.setBounds     (30,  30,  80, 95);
    syncTimeDial.setBounds (130, 20,  100, 105);
    syncChoice.setBounds   (230, 20,  100, 105);
    modeChoice.setBounds   (330, 30,  80, 95);
    

    modDial.setBounds      (30, 155, 80, 95);
    feedbackDial.setBounds (130,  145, 100, 105);
    mixDial.setBounds      (230, 145, 100, 105);
    styleChoice.setBounds  (330, 155, 80, 95);

}

void EditorContent::dialValueChanged(Dial* dial)
{
    if (dial == &syncChoice)
    {
        if (syncChoice.getValue() == 0)
        {
            syncTimeDial.setColour(Dial::foregroundArcColourId, juce::Colours::lightgrey);
            syncTimeDial.setAlpha(0.4);
            syncTimeDial.setLabelText(juce::String ("Sync Off"));
            syncTimeDial.repaint();
        }
        else if (syncChoice.getValue() == 1)
        {
            syncTimeDial.setColour(Dial::foregroundArcColourId, juce::Colours::aquamarine);
            syncTimeDial.setAlpha(1);
            syncTimeDial.setLabelText(juce::String ("Note"));
            syncTimeDial.repaint();
        }
    }
}
