/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

//==============================================================================
ClipSatAudioProcessor::ClipSatAudioProcessor()
    : parameters (*this, &undoManager, "Parameters",
                    {
                        std::make_unique<juce::AudioParameterFloat>("inputGain", "Input Gain", 0.0f, 2.0f, 1.0f),
                        std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", juce::NormalisableRange<float>(-24.0f, 0.0f, 0.1f), -6.0f),
                        std::make_unique<juce::AudioParameterBool>("softClipping", "Soft Clipping", false),
                        std::make_unique<juce::AudioParameterFloat>("outputGain", "Output Gain", 0.0f, 2.0f, 1.0f),
                        std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 1.0f, 10.0f, 0.5f),
                        std::make_unique<juce::AudioParameterFloat>("dryWet", "Dry/Wet", 0.0f, 1.0f, 0.5f),
                        std::make_unique<juce::AudioParameterChoice>("saturationMode", "Saturation Mode", juce::StringArray{"Soft Sine", "Hard Curve", "Analog Clip", "Sinoid Fold"}, 0),
                        std::make_unique<juce::AudioParameterBool>("clipperOnOff", "Clipper On/Off", true),
                        std::make_unique<juce::AudioParameterBool>("satOnOff", "Saturator On/Off", true)
                   })
{
}

ClipSatAudioProcessor::~ClipSatAudioProcessor()
{
}

//==============================================================================


//==============================================================================
void ClipSatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ClipSatAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

//Might not need this
#ifndef JucePlugin_PreferredChannelConfigurations
bool ClipSatAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ClipSatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear any channels that are not being used
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Retrieve parameter values
    float inputGain = *parameters.getRawParameterValue("inputGain");
    float threshold = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("threshold")->load());
    bool softClipping = *parameters.getRawParameterValue("softClipping");
    bool clipperOnOff = *parameters.getRawParameterValue("clipperOnOff");
    bool satOnOff = *parameters.getRawParameterValue("satOnOff");
    float driveParam = *parameters.getRawParameterValue("drive");
    float dryWetParam = *parameters.getRawParameterValue("dryWet");
    float saturationModeParam = *parameters.getRawParameterValue("saturationMode");
    float outputGainValue = *parameters.getRawParameterValue("outputGain");

    // Apply the input gain to the buffer
    buffer.applyGain(inputGain);
    
    
    // Push the input buffer to the visualizer before any processing
    if (auto* editor = dynamic_cast<ClipSatAudioProcessorEditor*>(getActiveEditor()))
        {
            editor->getAudioVisualiser().pushInputBuffer(buffer);
        }
    
    
    auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Smoothly update the parameter values
        smoothedDrive += smoothingFactor * (driveParam - smoothedDrive);
        smoothedDryWet += smoothingFactor * (dryWetParam - smoothedDryWet);

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            float cleanSignal = channelData[sample];
            float saturatedSignal = cleanSignal;

            // Apply the saturation effect based on the selected mode
            if (satOnOff){
                switch (static_cast<int>(saturationModeParam))
                {
                    case 0: // Soft Sine
                        saturatedSignal = std::sin(driveParam * cleanSignal);
                        break;
                    case 1: // Hard Curve
                        saturatedSignal = cleanSignal - std::pow(cleanSignal, 3) * driveParam;
                        break;
                    case 2: // Analog Clip
                        saturatedSignal = std::max(static_cast<float>(-driveParam), std::min(static_cast<float>(driveParam), cleanSignal));
                        break;
                    case 3: // Sinoid Fold
                        saturatedSignal = std::asin(std::sin(driveParam * cleanSignal));
                        break;
                    default:
                        break;
                }
            }
            

            // Blend the processed (wet) signal with the original (dry) signal using smoothedDryWet
            float mixedSignal = smoothedDryWet * saturatedSignal + (1 - smoothedDryWet) * cleanSignal;

            // Apply the output level control using smoothedOutput
            channelData[sample] = mixedSignal;
            
            float processedSample = mixedSignal;
            
            if (clipperOnOff){
                if (softClipping)
                {
                    // Soft clipping
                    if (processedSample > threshold)
                        processedSample = threshold + (1 - expf(-processedSample + threshold));
                    else if (processedSample < -threshold)
                        processedSample = -threshold - (1 - expf(-processedSample - threshold));
                }
                else
                {
                    // Hard clipping
                    if (processedSample > threshold)
                        processedSample = threshold;
                    else if (processedSample < -threshold)
                        processedSample = -threshold;
                }
            }
            channelData[sample] = processedSample;
        }
    }
    

    // Apply the output gain to the buffer
    buffer.applyGain(outputGainValue);
    
    if (auto* editor = dynamic_cast<ClipSatAudioProcessorEditor*>(getActiveEditor()))
        {
            // Process your audio and store the result in the outputBuffer
            juce::AudioBuffer<float> outputBuffer = buffer; // Replace this with your actual output buffer
            editor->getAudioVisualiser().pushOutputBuffer(outputBuffer);

            // Set the threshold value for the visualiser
            float thresholdValue = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("threshold")->load());
            editor->getAudioVisualiser().setThreshold(thresholdValue);
        }
}

//==============================================================================
bool ClipSatAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ClipSatAudioProcessor::createEditor()
{
    return new ClipSatAudioProcessorEditor (*this);
}

//==============================================================================
const juce::String ClipSatAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ClipSatAudioProcessor::acceptsMidi() const
{
    return false;
}

bool ClipSatAudioProcessor::producesMidi() const
{
    return false;
}

bool ClipSatAudioProcessor::isMidiEffect() const
{
    return false;
}

double ClipSatAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ClipSatAudioProcessor::getNumPrograms()
{
    return 1;
}

int ClipSatAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ClipSatAudioProcessor::setCurrentProgram(int index) {}

const juce::String ClipSatAudioProcessor::getProgramName(int index)
{
    return {};
}

void ClipSatAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

void ClipSatAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ClipSatAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClipSatAudioProcessor();
}

