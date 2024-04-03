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
                        std::make_unique<juce::AudioParameterBool>("satOnOff", "Saturator On/Off", true),
                        std::make_unique<juce::AudioParameterFloat>("rate", "Rate", 0.1f, 10.0f, 1.0f),
                        std::make_unique<juce::AudioParameterFloat>("depth", "Depth", 0.0f, 0.50f, 0.1f),
                        std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f),
                        std::make_unique<juce::AudioParameterBool>("chorusOnOff", "Chorus On/Off", true)
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
    const int numInputChannels = getTotalNumInputChannels();
    const int delayBufferSize = 2 * (sampleRate + samplesPerBlock);
    delayBuffer.setSize(numInputChannels, delayBufferSize);
    delayBuffer.clear();
    delayBuffer2.setSize(numInputChannels, delayBufferSize);
    delayBuffer2.clear();
    delayBufferSamples = delayBufferSize;
    delayBufferChannels = numInputChannels;
    delayWritePosition = 0;
    delayWritePosition2 = 0;
    lowPassFilter1.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 4000.0));
    lowPassFilter2.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 4000.0));
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
    bool chorusOnOff = *parameters.getRawParameterValue("chorusOnOff");
    bool satOnOff = *parameters.getRawParameterValue("satOnOff");
    float driveParam = *parameters.getRawParameterValue("drive");
    float dryWetParam = *parameters.getRawParameterValue("dryWet");
    float saturationModeParam = *parameters.getRawParameterValue("saturationMode");
    float outputGainValue = *parameters.getRawParameterValue("outputGain");
    
    auto* rateParam = parameters.getRawParameterValue("rate");
    auto* depthParam = parameters.getRawParameterValue("depth");
    auto* mixParam = parameters.getRawParameterValue("mix");

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
        
        auto inL = buffer.getReadPointer(0)[sample];
        auto inR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1)[sample] : inL;

        
        lfoPhase += *rateParam * 0.01f; // LFO rate
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;
        
        auto delayTime = (1.0f + std::sin(lfoPhase)) * 0.5f * *depthParam * 0.02f; // 20ms max delay

        int delaySamples = static_cast<int>(delayTime * getSampleRate());

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            float cleanSignal = channelData[sample];
            float saturatedSignal = cleanSignal;
            
            // apply chorus if toggled
            if (chorusOnOff)
            {
                auto* delayData1 = delayBuffer.getWritePointer(channel);
                auto* delayData2 = delayBuffer2.getWritePointer(channel);
                
                // Write the input signal into the delay buffers
                delayData1[delayWritePosition] = cleanSignal;
                delayData2[delayWritePosition2] = cleanSignal;

                // Calculate the read positions for the delay buffers
                int readPosition1 = delayWritePosition - static_cast<int>((1.0f + std::sin(lfoPhase)) * 0.5f * *depthParam * 0.02f * getSampleRate());
                int readPosition2 = delayWritePosition2 - static_cast<int>((1.0f + std::sin(lfoPhase2)) * 0.5f * *depthParam * 0.02f * getSampleRate());

                // Wrap the read positions if they go beyond the buffer bounds
                if (readPosition1 < 0) readPosition1 += delayBufferSamples;
                if (readPosition2 < 0) readPosition2 += delayBufferSamples;

                // Read the delayed samples from the delay buffers
                auto delaySample1 = delayData1[readPosition1];
                auto delaySample2 = delayData2[readPosition2];

                // Process the delayed samples through the low-pass filters
                delaySample1 = lowPassFilter1.processSingleSampleRaw(delaySample1 + feedbackAmount * delaySample1);
                delaySample2 = lowPassFilter2.processSingleSampleRaw(delaySample2 + feedbackAmount * delaySample2);

                // Mix the delayed samples with the original signal
                channelData[sample] = cleanSignal + (*mixParam * ((delaySample1 + delaySample2) - cleanSignal));
            }
                
            
            
            float postChorusSignal = channelData[sample];
            
            // Apply the saturation effect based on the selected mode
            if (satOnOff){
                switch (static_cast<int>(saturationModeParam))
                {
                    case 0: // Soft Sine
                        saturatedSignal = std::sin(driveParam * postChorusSignal);
                        break;
                    case 1: // Hard Curve
                        saturatedSignal = postChorusSignal - std::pow(postChorusSignal, 3) * driveParam;
                        break;
                    case 2: // Analog Clip
                        saturatedSignal = std::max(static_cast<float>(-driveParam), std::min(static_cast<float>(driveParam), postChorusSignal));
                        break;
                    case 3: // Sinoid Fold
                        saturatedSignal = std::asin(std::sin(driveParam * postChorusSignal));
                        break;
                    default:
                        break;
                }
            }
            // Blend the processed (wet) signal with the original (dry) signal using smoothedDryWet
            float mixedSignal = smoothedDryWet * saturatedSignal + (1 - smoothedDryWet) * postChorusSignal;

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
        lfoPhase += *rateParam * 0.01f;
        lfoPhase2 += *rateParam * 0.012f; // Slightly different rate for the second LFO

        if (lfoPhase >= juce::MathConstants<float>::twoPi) lfoPhase -= juce::MathConstants<float>::twoPi;
        if (lfoPhase2 >= juce::MathConstants<float>::twoPi) lfoPhase2 -= juce::MathConstants<float>::twoPi;

        if (++delayWritePosition >= delayBufferSamples) delayWritePosition = 0;
        if (++delayWritePosition2 >= delayBufferSamples) delayWritePosition2 = 0;
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

