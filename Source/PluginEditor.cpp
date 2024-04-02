/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AbletonLookAndFeel.h"

//==============================================================================
ClipSatAudioProcessorEditor::ClipSatAudioProcessorEditor (ClipSatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), audioVisualiser(2)
{
    //setSize(400, 300);
    setLookAndFeel(&abletonLookAndFeel);
    
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "drive", driveSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "output", outputSlider);
    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dryWet", dryWetSlider);
    saturationModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "saturationMode", saturationModeBox);

    // Set up the sliders
    addAndMakeVisible(driveSlider);
    driveSlider.setRange(0.0, 1.0);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 40);
    driveAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "drive", driveSlider));

    addAndMakeVisible(outputSlider);
    outputSlider.setRange(0.0, 1.0);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 40);
    outputAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "output", outputSlider));

    addAndMakeVisible(dryWetSlider);
    dryWetSlider.setRange(0.0, 1.0);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 40);
    dryWetAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "dryWet", dryWetSlider));

    // Set up the combo box for saturation mode selection
    addAndMakeVisible(saturationModeBox);
    saturationModeBox.addItem("Soft Sine", 1);
    saturationModeBox.addItem("Hard Curve", 2);
    saturationModeBox.addItem("Analog Clip", 3);
    saturationModeBox.addItem("Sinoid Fold", 4);
    saturationModeBox.setSelectedId(1);
    saturationModeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(audioProcessor.parameters, "saturationMode", saturationModeBox));

    // Input Gain Slider and Label
    inputGainSlider.setLookAndFeel(&abletonLookAndFeel);
    inputGainSlider.setSliderStyle(juce::Slider::Rotary);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(inputGainSlider);
    inputGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "inputGain", inputGainSlider));

    inputGainLabel.setText("Input Gain", juce::dontSendNotification);
    inputGainLabel.attachToComponent(&inputGainSlider, false);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(inputGainLabel);

    // Threshold Slider and Label
    thresholdSlider.setLookAndFeel(&abletonLookAndFeel);
    thresholdSlider.setSliderStyle(juce::Slider::Rotary);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(thresholdSlider);
    thresholdAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "threshold", thresholdSlider));

    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.attachToComponent(&thresholdSlider, false);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);

    // Output Gain Slider and Label
    outputGainSlider.setLookAndFeel(&abletonLookAndFeel);
    outputGainSlider.setSliderStyle(juce::Slider::Rotary);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(outputGainSlider);
    outputGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "outputGain", outputGainSlider));

    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.attachToComponent(&outputGainSlider, false);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputGainLabel);
    
    //Soft Clipping option
    softClippingButton.setButtonText("Soft Clipping");
    addAndMakeVisible(softClippingButton);
    softClippingAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.parameters, "softClipping", softClippingButton));

    //softClippingLabel.setText("Soft Clipping", juce::dontSendNotification);
    softClippingLabel.attachToComponent(&softClippingButton, true);
    softClippingLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(softClippingLabel);

    
    audioVisualiser.setBufferSize(512); // Set the buffer size for the visualiser
    audioVisualiser.setSamplesPerBlock(256); // Set the number of samples per block
    audioVisualiser.setColours(juce::Colours::black, juce::Colours::white); // Set the background and waveform colours
    addAndMakeVisible(audioVisualiser);
    
    setSize(400, 400);
}

ClipSatAudioProcessorEditor::~ClipSatAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void ClipSatAudioProcessorEditor::paint (juce::Graphics& g)
{
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colour::fromRGB(45, 45, 45)); // Dark gray background
}

void ClipSatAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    int totalComponents = 3; // Input, Output, and Threshold
    int spacing = 10; // Spacing between components
    int totalSpacing = (totalComponents - 1) * spacing;
    int componentWidth = (area.getWidth() - totalSpacing) / totalComponents;
    int sliderHeight = 100;
    int labelHeight = 20;
    int verticalOffset = 20;
    int visualizerHeight = 125;
    int buttonHeight = 30;  // Adjust the button height as desired

    int xPosition = (area.getWidth() - (componentWidth * totalComponents + totalSpacing)) / 2;

    inputGainSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;

    thresholdSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;

    outputGainSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);

    // Position the labels above the sliders
    inputGainLabel.setBounds(inputGainSlider.getX(), inputGainSlider.getY() - labelHeight, inputGainSlider.getWidth(), labelHeight);
    thresholdLabel.setBounds(thresholdSlider.getX(), thresholdSlider.getY() - labelHeight, thresholdSlider.getWidth(), labelHeight);
    outputGainLabel.setBounds(outputGainSlider.getX(), outputGainSlider.getY() - labelHeight, outputGainSlider.getWidth(), labelHeight);

    // Position the audio visualizer below the sliders
    audioVisualiser.setBounds(10, inputGainSlider.getBottom() + verticalOffset, area.getWidth() - 20, visualizerHeight);

    // Position the soft clipping button below the audio visualizer
    softClippingButton.setBounds(thresholdSlider.getX(), audioVisualiser.getBottom() + verticalOffset, componentWidth, buttonHeight);
    
    driveSlider.setBounds(200, 40, 100, 30);
    outputSlider.setBounds(200, 80, 100, 30);
    dryWetSlider.setBounds(200, 120, 100, 30);
    saturationModeBox.setBounds(200, 160, 100, 30);
}

CustomAudioVisualiserComponent& ClipSatAudioProcessorEditor::getAudioVisualiser()
{
    return audioVisualiser;
}

