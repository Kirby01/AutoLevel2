#include "PluginEditor.h"
#include "PluginProcessor.h"

AutoLevel2AudioProcessorEditor::AutoLevel2AudioProcessorEditor (AutoLevel2AudioProcessor& p)
: AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);
    setResizable (false, false);
    setSize (680, 360);

    auto styleKnob = [] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    };

    styleKnob (inSlider);   addAndMakeVisible (inSlider);
    styleKnob (outSlider);  addAndMakeVisible (outSlider);
    styleKnob (gammaSlider);addAndMakeVisible (gammaSlider);
    styleKnob (thetaSlider);addAndMakeVisible (thetaSlider);

    inLabel.setText ("Input", juce::dontSendNotification);
    outLabel.setText ("Output", juce::dontSendNotification);
    gammaLabel.setText ("Gamma", juce::dontSendNotification);
    thetaLabel.setText ("Theta", juce::dontSendNotification);

    for (auto* l : { &inLabel, &outLabel, &gammaLabel, &thetaLabel })
    { l->setJustificationType (juce::Justification::centred); addAndMakeVisible (*l); }

    auto& apvts = processor.apvts;
    inAttach    = std::make_unique<Attachment>   (apvts, "INPUT",  inSlider);
    outAttach   = std::make_unique<Attachment>   (apvts, "OUTPUT", outSlider);
    gammaAttach = std::make_unique<Attachment>   (apvts, "GAMMA",  gammaSlider);
    thetaAttach = std::make_unique<Attachment>   (apvts, "THETA",  thetaSlider);
}

void AutoLevel2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // Studio-dark background
    auto bg = juce::Colour (0xff141414);
    g.fillAll (bg);
    juce::Colour top = juce::Colour (0xff0e0e0e);
    juce::Colour bot = juce::Colour (0xff141414);
    g.setGradientFill (juce::ColourGradient (top, 0.f, 0.f, bot, 0.f, (float) getHeight(), false));
    g.fillRect (getLocalBounds());

    // Header
    g.setColour (juce::Colour::fromString ("#FF33D0C0"));
    g.setFont (juce::Font ("Roboto Condensed", 28.0f, juce::Font::bold));
    g.drawFittedText ("AUTOLEVEL2", juce::Rectangle<int> (20, 8, getWidth()-40, 36),
                      juce::Justification::centredTop, 1);

    g.setColour (juce::Colours::white.withAlpha (0.65f));
    g.setFont (juce::Font ("Open Sans", 14.0f, juce::Font::italic));
    g.drawFittedText ("by Harmonious Records", juce::Rectangle<int> (20, 42, getWidth()-40, 24),
                      juce::Justification::centredTop, 1);

    // Meters (simple teal bars)
    auto area = getLocalBounds().reduced (18);
    auto footer = area.removeFromBottom (70);
    auto meterW = (footer.getWidth() - 60) / 4;
    meterInL  = footer.removeFromLeft (meterW).toFloat();
    footer.removeFromLeft (15);
    meterInR  = footer.removeFromLeft (meterW).toFloat();
    footer.removeFromLeft (15);
    meterOutL = footer.removeFromLeft (meterW).toFloat();
    footer.removeFromLeft (15);
    meterOutR = footer.removeFromLeft (meterW).toFloat();

    auto drawMeter = [&] (juce::Rectangle<float> r, float peak)
    {
        g.setColour (juce::Colours::white.withAlpha (0.15f));
        g.fillRoundedRectangle (r, 6.0f);
        auto filled = r.withBottom (r.getBottom() - r.getHeight() * (1.0f - juce::jlimit (0.0f, 1.0f, peak)));
        g.setColour (juce::Colour::fromString ("#FF2CBFAE"));
        g.fillRoundedRectangle (filled, 6.0f);
    };

    drawMeter (meterInL,  processor.inputPeakL.load());
    drawMeter (meterInR,  processor.inputPeakR.load());
    drawMeter (meterOutL, processor.outputPeakL.load());
    drawMeter (meterOutR, processor.outputPeakR.load());

    g.setColour (juce::Colours::white.withAlpha (0.65f));
    g.setFont (juce::Font (12.0f));
    g.drawFittedText ("In L",  meterInL.toNearestInt(),  juce::Justification::centredBottom, 1);
    g.drawFittedText ("In R",  meterInR.toNearestInt(),  juce::Justification::centredBottom, 1);
    g.drawFittedText ("Out L", meterOutL.toNearestInt(), juce::Justification::centredBottom, 1);
    g.drawFittedText ("Out R", meterOutR.toNearestInt(), juce::Justification::centredBottom, 1);
}

void AutoLevel2AudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (20);
    r.removeFromTop (80); // header

    auto knobs = r.removeFromTop (200);
    auto w = knobs.getWidth() / 4;

    auto place = [] (juce::Slider& s, juce::Label& l, juce::Rectangle<int> area)
    {
        s.setBounds (area.reduced (14));
        l.setBounds (s.getX(), s.getBottom() + 2, s.getWidth(), 18);
    };

    place (inSlider,    inLabel,    knobs.removeFromLeft (w));
    place (gammaSlider, gammaLabel, knobs.removeFromLeft (w));
    place (thetaSlider, thetaLabel, knobs.removeFromLeft (w));
    place (outSlider,   outLabel,   knobs.removeFromLeft (w));
}
