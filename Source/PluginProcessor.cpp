/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularDelayAudioProcessor::CircularDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor(BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), delayEffect(treeState)

#endif
{
}

CircularDelayAudioProcessor::~CircularDelayAudioProcessor()
{
}
                                    
//==============================================================================
const juce::String CircularDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CircularDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CircularDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CircularDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CircularDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CircularDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CircularDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CircularDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CircularDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void CircularDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CircularDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    delayEffect.prepare(spec);
    delayEffect.reset();
}

void CircularDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CircularDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void CircularDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const auto numChannels = juce::jmax (totalNumInputChannels, totalNumOutputChannels);
    int delayMode = delayEffect.getMode();

    delayEffect.updateParameters();
    delayEffect.updateTimeInSamples(getHostBpm());
        
    if (numChannels == 1 && delayMode == modePingPong)
    {
        treeState.getParameter("MODE")->setValueNotifyingHost(0);
    }
    
    if (delayMode == modePingPong && numChannels > 1)
    {
        delayEffect.processCircular(buffer);
    }
    
    if (delayMode == modeStereo || numChannels == 1)
    {
        delayEffect.processStereo(buffer);
    }
    
}

//==============================================================================
bool CircularDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CircularDelayAudioProcessor::createEditor()
{
    return new CircularDelayAudioProcessorEditor (*this, treeState, undoManager);
//    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void CircularDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    treeState.state.writeToStream (mos);
}

void CircularDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, static_cast<size_t> (sizeInBytes));

    if (tree.isValid())
        treeState.replaceState (tree);
}

double CircularDelayAudioProcessor::getHostBpm()
{
    if (auto bpmFromHost = *getPlayHead()->getPosition()->getBpm())
    {
        bpm = bpmFromHost;
        return bpm;
    }
    else
    {
        treeState.getParameter("SYNC")->setValueNotifyingHost(0);
        return 0;
    }

}

juce::AudioProcessorValueTreeState::ParameterLayout
CircularDelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    using pID = juce::ParameterID;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>  (pID{"TIME", 1}, "Time", juce::NormalisableRange<float> {1.0f, 3000.0f, 1.0f}, 250.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice> (pID{"MODE", 1}, "Mode", juce::StringArray{"Stereo", "Ping-Pong"}, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice> (pID{"STYLE", 1}, "Style", juce::StringArray{"Tape", "Digital"}, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>   (pID{"SYNC", 1}, "Sync", true));
    layout.add(std::make_unique<juce::AudioParameterChoice> (pID{"SYNC_TIME", 1}, "Note", juce::StringArray{"1/16", "1/16 Triplet", "16th Dotted",
                                                             "1/8", "1/8 Triplet", "1/8 Dotted", "1/4", "1/4 Triplet", "1/4 Dotted",
                                                             "1/2", "1/2 Triplet", "1/2 Dotted", "1"}, 3, "Sync Time"));
    layout.add(std::make_unique<juce::AudioParameterFloat>  (pID{"FEEDBACK", 1}, "Feedback", juce::NormalisableRange<float> {0.0f, 1.0f, 0.01f}, 0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>  (pID{"MOD", 1}, "Mod", juce::NormalisableRange<float>{0.0f, 1.0f, 0.01f, 0.7}, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>  (pID{"MIX", 1}, "Mix", juce::NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 0.5f));
    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CircularDelayAudioProcessor();
}


