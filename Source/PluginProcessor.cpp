/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
HarrymonsterIRAudioProcessor::HarrymonsterIRAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
{
    auto dir = juce::File::getCurrentWorkingDirectory();
    dir = dir.getParentDirectory();
    auto irFile = dir.getChildFile("Resources").getChildFile("BD_RH_DDR.wav");
    convolution.loadImpulseResponse(irFile, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 1024);

    // Wait for the impulse response to load
    while (convolution.getLatency() == 0)
    {
        // You could add a small sleep here to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}




HarrymonsterIRAudioProcessor::~HarrymonsterIRAudioProcessor()
{
}

//==============================================================================
const juce::String HarrymonsterIRAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HarrymonsterIRAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HarrymonsterIRAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HarrymonsterIRAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HarrymonsterIRAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HarrymonsterIRAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HarrymonsterIRAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HarrymonsterIRAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HarrymonsterIRAudioProcessor::getProgramName (int index)
{
    return {};
}

void HarrymonsterIRAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HarrymonsterIRAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    // Reset the processor chain
    processorChain.reset();
}

void HarrymonsterIRAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HarrymonsterIRAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void HarrymonsterIRAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Apply the convolution effect
    processorChain.process(juce::dsp::ProcessContextReplacing<float>(buffer));
}


//==============================================================================
bool HarrymonsterIRAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HarrymonsterIRAudioProcessor::createEditor()
{
    return new HarrymonsterIRAudioProcessorEditor (*this);
}

//==============================================================================
void HarrymonsterIRAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HarrymonsterIRAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarrymonsterIRAudioProcessor();
}
