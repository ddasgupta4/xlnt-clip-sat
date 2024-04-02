// AudioVisualiserComponent.h
#pragma once

#include <JuceHeader.h>

class CustomAudioVisualiserComponent : public juce::AudioVisualiserComponent
{
public:
    CustomAudioVisualiserComponent(int numChannels) : AudioVisualiserComponent(numChannels) {}

    void setThreshold(float newThreshold)
    {
        threshold = newThreshold;
        repaint();
    }

protected:
    void paint(juce::Graphics& g) override
    {
        AudioVisualiserComponent::paint(g);

        float y = getHeight() - (getHeight() * (threshold + 1.0f) / 2.0f);
        g.setColour(juce::Colours::red);
        g.drawLine(0, y, getWidth(), y, 2.0f);
    }

private:
    float threshold = 0.0f;
};

