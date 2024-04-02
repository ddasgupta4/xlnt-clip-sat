/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AbletonLookAndFeel.h"

//==============================================================================
/**
*/

class CustomAudioVisualiserComponent : public juce::AudioVisualiserComponent
{
    // Class definition...
    public:
        CustomAudioVisualiserComponent(int numChannels) : AudioVisualiserComponent(numChannels) {
            setBufferSize(1024); // Increase the buffer size for smoother waveforms
            setSamplesPerBlock(256);
        }

    void pushInputBuffer(const juce::AudioBuffer<float>& buffer)
        {
            inputBuffer.clear();
            inputBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
            inputBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
            repaint();
        }

        void pushOutputBuffer(const juce::AudioBuffer<float>& buffer)
        {
            outputBuffer.clear();
            outputBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
            outputBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
            repaint();
        }

        void setThreshold(float newThreshold)
        {
            threshold = newThreshold;
            repaint();
        }

    protected:
        void paint(juce::Graphics& g) override
        {
            // Calculate the height of each lane
            float laneHeight = getHeight() / 2.0f;

            // Draw the input waveform in the top lane (in white)
            juce::Rectangle<float> inputLane(0, 0, getWidth(), laneHeight);
            drawWaveform(g, inputBuffer, juce::Colours::white, inputLane);

            // Draw the output waveform in the bottom lane (in red)
            juce::Rectangle<float> outputLane(0, laneHeight, getWidth(), laneHeight);
            drawWaveform(g, outputBuffer, juce::Colours::red, outputLane); // Using 50% opacity for the output

            // Draw the threshold line in blue across both lanes
            float y = juce::jmap<float>(threshold, -1.0f, 1.0f, static_cast<float>(getHeight()), 0.0f);
            g.setColour(juce::Colours::blue.withAlpha(0.5f));
            g.drawLine(0, y, getWidth(), y, 2.0f);
        }

    private:
        void drawWaveform(juce::Graphics& g, const juce::AudioBuffer<float>& buffer, const juce::Colour& colour, const juce::Rectangle<float>& lane)
        {
            g.setColour(colour);
            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getReadPointer(channel);
                auto numSamples = buffer.getNumSamples();
                auto bounds = getLocalBounds().toFloat();
                auto width = bounds.getWidth();
                auto height = bounds.getHeight();

                juce::Path path;
                path.startNewSubPath(0, juce::jmap<float>(channelData[0], -1.0f, 1.0f, height, 0));

                for (int sample = 0; sample < numSamples; ++sample)
                {
                    /*
                    auto x = juce::jmap<int>(sample, 0, numSamples - 1, 0, static_cast<int>(lane.getWidth()));
                    auto y = juce::jmap<float>(channelData[sample], -1.0f, 1.0f, lane.getBottom(), lane.getY());
                    path.lineTo(x + lane.getX(), y);
                     */
                    
                    float x = juce::jmap<float>(sample, 0, numSamples - 1, 0, width);
                    float y = juce::jmap<float>(channelData[sample], -1.0f, 1.0f, height, 0);
                    path.lineTo(x, y);
                }

                g.strokePath(path, juce::PathStrokeType(1.0f));
            }
        }

        juce::AudioBuffer<float> inputBuffer;
        juce::AudioBuffer<float> outputBuffer;
        float threshold = 0.0f;
    };

class ClipSatAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ClipSatAudioProcessorEditor (ClipSatAudioProcessor&);
    ~ClipSatAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    CustomAudioVisualiserComponent& getAudioVisualiser(); // Add this line

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ClipSatAudioProcessor& audioProcessor;
    
    // GUI components
    juce::Slider inputGainSlider;
    juce::Slider thresholdSlider;
    juce::ToggleButton softClippingButton;
    juce::Slider outputGainSlider;

    // Attachments bridge the GUI components with the processor's parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> softClippingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    
    juce::Label inputGainLabel;
    juce::Label thresholdLabel;
    juce::Label outputGainLabel;
    juce::Label softClippingLabel;
    
    // UI components
    juce::Slider driveSlider;
    juce::Slider outputSlider;
    juce::Slider dryWetSlider;
    juce::ComboBox saturationModeBox;
    

    // Attachment for each parameter
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> saturationModeAttachment;
    
    //juce::AudioVisualiserComponent audioVisualiser; // Add this line
    CustomAudioVisualiserComponent audioVisualiser;
    AbletonLookAndFeel abletonLookAndFeel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipSatAudioProcessorEditor)
};

