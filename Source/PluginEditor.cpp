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
    
    //driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "drive", driveSlider);
    //dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dryWet", dryWetSlider);
    //saturationModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "saturationMode", saturationModeBox);

    // Set up the sliders
    /*
    addAndMakeVisible(driveSlider);
    driveSlider.setRange(0.0, 1.0);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 40);
    driveAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "drive", driveSlider));
     */
    
    // Drive Slider and Label
    driveSlider.setLookAndFeel(&abletonLookAndFeel);
    driveSlider.setSliderStyle(juce::Slider::Rotary);
    driveSlider.setRange(0.0, 1.0);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(driveSlider);
    driveAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "drive", driveSlider));

    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.attachToComponent(&inputGainSlider, false);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);
    
    // Drive Slider and Label
    dryWetSlider.setLookAndFeel(&abletonLookAndFeel);
    dryWetSlider.setSliderStyle(juce::Slider::Rotary);
    dryWetSlider.setRange(0.0, 1.0);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(dryWetSlider);
    dryWetAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "dryWet", dryWetSlider));

    dryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent(&inputGainSlider, false);
    dryWetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dryWetLabel);

    /*
    addAndMakeVisible(dryWetSlider);
    dryWetSlider.setRange(0.0, 1.0);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 40);
    dryWetAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "dryWet", dryWetSlider));
     */
    
    // Set up the combo box for saturation mode selection
    addAndMakeVisible(saturationModeBox);
    saturationModeBox.addItem("Soft Sine", 1);
    saturationModeBox.addItem("Hard Curve", 2);
    saturationModeBox.addItem("Analog Clip", 3);
    saturationModeBox.addItem("Sinoid Fold", 4);
    saturationModeBox.setSelectedId(1);
    saturationModeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(audioProcessor.parameters, "saturationMode", saturationModeBox));
    
    saturationLabel.setText("Saturation Mode", juce::dontSendNotification);
    saturationLabel.attachToComponent(&inputGainSlider, false);
    saturationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationLabel);

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
    
    //Clipper On/Off option
    clipperButton.setButtonText("Clipper On/Off");
    addAndMakeVisible(clipperButton);
    clipperOnOffAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.parameters, "clipperOnOff", clipperButton));

    //softClippingLabel.setText("Soft Clipping", juce::dontSendNotification);
    clipperOnOffLabel.attachToComponent(&clipperButton, true);
    clipperOnOffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(clipperOnOffLabel);
    
    //saturator On/Off option
    satButton.setButtonText("Saturator On/Off");
    addAndMakeVisible(satButton);
    satOnOffAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.parameters, "satOnOff", satButton));

    //softClippingLabel.setText("Soft Clipping", juce::dontSendNotification);
    satOnOffLabel.attachToComponent(&satButton, true);
    satOnOffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(satOnOffLabel);

    
    audioVisualiser.setBufferSize(512); // Set the buffer size for the visualiser
    audioVisualiser.setSamplesPerBlock(256); // Set the number of samples per block
    audioVisualiser.setColours(juce::Colours::black, juce::Colours::white); // Set the background and waveform colours
    addAndMakeVisible(audioVisualiser);
    
    setSize(600, 400);
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
    int totalComponents = 7; // Input, Output, Threshold, Drive, Dry/Wet, Saturation Mode, and Soft Clipping Button
    int spacing = 10; // Spacing between components
    int totalSpacing = (totalComponents - 1) * spacing;
    int componentWidth = (area.getWidth() - totalSpacing) / totalComponents;
    int sliderHeight = 100;
    int labelHeight = 20;
    int verticalOffset = 20;
    int visualizerHeight = 125;
    int buttonHeight = 30;  // Adjust the button height as desired

    int xPosition = (area.getWidth() - (componentWidth * totalComponents + totalSpacing)) / 2;

    // Position the original sliders
    inputGainSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;
    
    // Position the new sliders and combo box
    driveSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;

    dryWetSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;

    saturationModeBox.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;
    
    thresholdSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;

    outputGainSlider.setBounds(xPosition, (area.getHeight() - sliderHeight) / 4, componentWidth, sliderHeight);
    xPosition += componentWidth + spacing;

    // Position the labels above the sliders
    inputGainLabel.setBounds(inputGainSlider.getX(), inputGainSlider.getY() - labelHeight, inputGainSlider.getWidth(), labelHeight);
    thresholdLabel.setBounds(thresholdSlider.getX(), thresholdSlider.getY() - labelHeight, thresholdSlider.getWidth(), labelHeight);
    outputGainLabel.setBounds(outputGainSlider.getX(), outputGainSlider.getY() - labelHeight, outputGainSlider.getWidth(), labelHeight);
    driveLabel.setBounds(driveSlider.getX(), driveSlider.getY() - labelHeight, driveSlider.getWidth(), labelHeight);
    dryWetLabel.setBounds(dryWetSlider.getX(), dryWetSlider.getY() - labelHeight, dryWetSlider.getWidth(), labelHeight);
    saturationLabel.setBounds(saturationModeBox.getX(), saturationModeBox.getY() - labelHeight, saturationModeBox.getWidth(), labelHeight);

    int totalComponents2 = 3; // Input, Output, Threshold, Drive, Dry/Wet, Saturation Mode, and Soft Clipping Button
    int spacing2 = 20; // Spacing between components
    int totalSpacing2 = (totalComponents2 - 1) * spacing2;
    int componentWidth2 = (area.getWidth() - totalSpacing2) / totalComponents2;
    int xPosition2 = (area.getWidth() - (componentWidth2 * totalComponents2 + totalSpacing2)) / 2;
    // Position the audio visualizer below the sliders
    audioVisualiser.setBounds(10, inputGainSlider.getBottom() + verticalOffset, area.getWidth() - 20, visualizerHeight);

    // Position the soft clipping button below the audio visualizer
    softClippingButton.setBounds(xPosition2, audioVisualiser.getBottom() + verticalOffset, componentWidth, buttonHeight);
    xPosition2 += componentWidth2 + spacing2;
    
    clipperButton.setBounds(xPosition2, audioVisualiser.getBottom() + verticalOffset, componentWidth, buttonHeight);
    xPosition2 += componentWidth2 + spacing2;
    
    satButton.setBounds(xPosition2, audioVisualiser.getBottom() + verticalOffset, componentWidth, buttonHeight);
    xPosition2 += componentWidth2 + spacing2;
}


CustomAudioVisualiserComponent& ClipSatAudioProcessorEditor::getAudioVisualiser()
{
    return audioVisualiser;
}

