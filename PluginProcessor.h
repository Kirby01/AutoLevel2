#pragma once
#include <JuceHeader.h>

class AutoLevel2AudioProcessor : public juce::AudioProcessor
{
public:
    AutoLevel2AudioProcessor();
    ~AutoLevel2AudioProcessor() override = default;

    // ===== JUCE Core Overrides =====
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AutoLevel2 by Harmonious Records"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // ===== Program Handling (unused) =====
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // ===== State Handling =====
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ===== Parameters & Metering =====
    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS apvts;

    std::atomic<float> inputPeakL { 0.f }, inputPeakR { 0.f };
    std::atomic<float> outputPeakL { 0.f }, outputPeakR { 0.f };

private:
    // DSP state variables
    double b = 1.0, b2 = 1.0, zm = 1.0, zm2 = 1.0;

    // Range constants
    static constexpr double gammaMin = 1.0e-5;
    static constexpr double gammaMax = 1.0e-1;
    static constexpr double thetaMin = 1.0e-5;
    static constexpr double thetaMax = 1.0e-1;
    static constexpr double eps      = 1.0e-9;

    // Modern JUCE parameter creation
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoLevel2AudioProcessor)
};
