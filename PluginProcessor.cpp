#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

static float dBPeak (const float* x, int n)
{
    float p = 0.f;
    for (int i = 0; i < n; ++i)
        p = std::max (p, std::abs (x[i]));
    return p;
}

// =====================================================
// Constructor — works on JUCE 6 & 7
// =====================================================
AutoLevel2AudioProcessor::AutoLevel2AudioProcessor()
: AudioProcessor (BusesProperties()
                    .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
  apvts (*this, nullptr, "PARAMS", createParameterLayout())
{}

// =====================================================
bool AutoLevel2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AutoLevel2AudioProcessor::prepareToPlay (double, int)
{
    b = b2 = zm = zm2 = 1.0;
}

// =====================================================
// DSP Core
// =====================================================
void AutoLevel2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int N = buffer.getNumSamples();
    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getWritePointer (1);

    const double input  = apvts.getRawParameterValue ("INPUT")->load();
    const double output = apvts.getRawParameterValue ("OUTPUT")->load();
    const double gamma  = gammaMin * std::pow (gammaMax / gammaMin,
                           (double) apvts.getRawParameterValue ("GAMMA")->load());
    const double theta  = thetaMin * std::pow (thetaMax / thetaMin,
                           (double) apvts.getRawParameterValue ("THETA")->load());

    for (int n = 0; n < N; ++n)
    {
        const double ioL = L[n];
        const double ioR = R[n];

        // --- Left channel ---
        double a  = std::abs (ioL * 2.0) * input;
        b  = (1.0 - gamma) * (b + std::abs (zm - b))
           +  gamma * (std::pow (a * 20.0, 2.0)) / (zm + 1e-10);
        zm = (1.0 - theta) * (b + std::abs (zm - b))
           +  theta * ( std::pow (std::abs (zm),
                        std::abs (std::abs (b) / (zm + 1e-7)) * 0.7)
                      + std::abs (zm) * 0.3 );

        // --- Right channel ---
        double a2 = std::abs (ioR * 2.0) * input;
        b2 = (1.0 - gamma) * (b2 + std::abs (zm2 - b2))
           +  gamma * (std::pow (a2 * 20.0, 2.0)) / (zm2 + 1e-10);
        zm2 = (1.0 - theta) * (b2 + std::abs (zm2 - b2))
             + theta * ( std::pow (std::abs (zm2),
                        std::abs (std::abs (b2) / (zm2 + 1e-7)) * 0.7)
                      + std::abs (zm2) * 0.3 );

        const double denom = 0.5 * (zm + zm2) + eps;
        L[n] = (float) juce::jlimit (-2.0, 2.0, (ioL / denom) * output);
        R[n] = (float) juce::jlimit (-2.0, 2.0, (ioR / denom) * output);
    }

    inputPeakL.store  (dBPeak (L, N));
    inputPeakR.store  (dBPeak (R, N));
    outputPeakL.store (inputPeakL.load());
    outputPeakR.store (inputPeakR.load());
}

// =====================================================
// State Management
// =====================================================
void AutoLevel2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto s = apvts.copyState().createXml())
        copyXmlToBinary (*s, destData);
}

void AutoLevel2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto s = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*s));
}

// =====================================================
// Parameter Layout (modern JUCE pattern)
// =====================================================
juce::AudioProcessorValueTreeState::ParameterLayout
AutoLevel2AudioProcessor::createParameterLayout()
{
    using R = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    const float gammaDefault = 0.725f;
    const float thetaDefault = 0.5f;

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "INPUT", "Input", R { 0.00025f, 3.0f, 0.00001f }, 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "OUTPUT", "Output", R { 1.0f, 10.0f, 0.001f }, 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "GAMMA", "Gamma", R { 0.0f, 1.0f, 0.0001f }, gammaDefault));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "THETA", "Theta", R { 0.0f, 0.5f, 0.0001f }, thetaDefault));

    return { params.begin(), params.end() };
}

// =====================================================
// JUCE plug-in entry point
// =====================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutoLevel2AudioProcessor();
}
// =====================================================
// Editor Factory
// =====================================================
juce::AudioProcessorEditor* AutoLevel2AudioProcessor::createEditor()
{
    // Minimal generic UI showing all parameters
    return new juce::GenericAudioProcessorEditor (*this);
}

// =====================================================
// Required JUCE Entry Point
// =====================================================
