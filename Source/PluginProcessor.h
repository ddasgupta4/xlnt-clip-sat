/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class ClipSatAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ClipSatAudioProcessor();
    ~ClipSatAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Constructor, destructor, and other necessary overrides here...

    
    // Might not need these lines if the code doesn't work
    float inputGain = 1.0f; // Default input gain
    float threshold = 0.0f; // Clipping threshold
    bool softClipping = true; // Clipping mode: true for soft, false for hard
    
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    
    juce::UndoManager undoManager;
    
    float outputGain = 1.0f; // Default output gain
    float smoothedDrive = 0.0f;
    float smoothedOutput = 0.0f;
    float smoothedDryWet = 0.0f;
    const float smoothingFactor = 0.01f; // Adjust this value to control the smoothing speed
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipSatAudioProcessor)
};

