#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Simple teal LookAndFeel
class HarmoniqLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HarmoniqLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff141414));
        setColour (juce::Slider::textBoxTextColourId, juce::Colours::white.withAlpha (0.85f));
        setColour (juce::Slider::thumbColourId, juce::Colour::fromString ("#FF33D0C0"));
        setColour (juce::Slider::trackColourId, juce::Colour::fromString ("#FF2CBFAE"));
        setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.75f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider& s) override
    {
        auto bounds = juce::Rectangle<float> (x, y, w, h).reduced (6.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto cx = bounds.getCentreX();
        auto cy = bounds.getCentreY();
        auto r  = radius;

        // Base ring
        g.setColour (juce::Colour (0xff1e1e1e));
        g.fillEllipse (bounds);

        // Teal ring
        juce::ColourGradient grad (juce::Colour::fromString ("#FF1E1E1E"), cx, cy,
                                   juce::Colour::fromString ("#FF33D0C0"), cx, y, true);
        g.setGradientFill (grad);
        g.fillEllipse (bounds.reduced (2.0f));

        // Arc
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path arc;
        arc.addCentredArc (cx, cy, r - 4.0f, r - 4.0f, 0.0f,
                           rotaryStartAngle, angle, true);
        g.setColour (juce::Colour::fromString ("#FF33FFCC"));
        g.strokePath (arc, juce::PathStrokeType (3.0f));

        // Pointer
        juce::Path p;
        p.addRectangle (-2.0f, -r + 8.0f, 4.0f, r * 0.4f);
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.fillPath (p, juce::AffineTransform::rotation (angle).translated (cx, cy));
    }
};

class AutoLevel2AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AutoLevel2AudioProcessorEditor (AutoLevel2AudioProcessor&);
    ~AutoLevel2AudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AutoLevel2AudioProcessor& processor;
    HarmoniqLookAndFeel lnf;

    // Controls
    juce::Slider inSlider, outSlider, gammaSlider, thetaSlider;
    juce::Label  inLabel, outLabel, gammaLabel, thetaLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> inAttach, outAttach, gammaAttach, thetaAttach;

    // Meters
    juce::Rectangle<float> meterInL, meterInR, meterOutL, meterOutR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoLevel2AudioProcessorEditor)
};
