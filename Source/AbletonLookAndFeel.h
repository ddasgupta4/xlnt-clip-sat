/*
  ==============================================================================

    AbletonLookAndFeel.h
    Created: 26 Feb 2024 10:55:10pm
    Author:  dylan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class AbletonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackWidth = 4.0f;

        juce::Point<float> startPoint(x + width * 0.5f, y + height);
        juce::Point<float> endPoint(x + width * 0.5f, y);

        // Draw the track
        g.setColour(juce::Colours::darkgrey);
        g.drawLine({ startPoint, endPoint }, trackWidth);

        // Draw the thumb
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillEllipse(juce::Rectangle<float>(static_cast<float>(width) * 0.5f - 10.0f, sliderPos - 10.0f, 20.0f, 20.0f));
    }
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto buttonArea = button.getLocalBounds();
        auto edge = 4;

        buttonArea.reduce(edge, edge);

        auto cornerSize = 6.0f;
        g.setColour(backgroundColour);
        g.fillRoundedRectangle(buttonArea.toFloat(), cornerSize);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        {
            g.setColour(backgroundColour.brighter(shouldDrawButtonAsDown ? 0.2f : 0.1f));
            g.fillRoundedRectangle(buttonArea.toFloat(), cornerSize);
        }
    }
    
    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll(label.findColour(juce::Label::backgroundColourId));

        if (!label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;
            const juce::Font font(getLabelFont(label));

            g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
            g.setFont(font);

            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                             juce::jmax(1, (int)(textArea.getHeight() / font.getHeight())),
                             label.getMinimumHorizontalScale());

            g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (label.isEnabled())
        {
            g.setColour(label.findColour(juce::Label::outlineColourId));
        }

        g.drawRect(label.getLocalBounds());
    }

    juce::Font getLabelFont(juce::Label& label) override
    {
        return juce::Font(15.0f, juce::Font::bold);
    }
    // Override other methods as needed...
};

