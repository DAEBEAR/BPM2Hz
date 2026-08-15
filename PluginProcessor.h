#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <array>

// Una divisione ritmica: nome mostrato, descrizione, moltiplicatore in beat (quarti)
struct NoteValue
{
    juce::String name;
    juce::String desc;
    double beatMultiplier;
};

class TempoConvertAudioProcessor : public juce::AudioProcessor
{
public:
    TempoConvertAudioProcessor();
    ~TempoConvertAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Letto dalla GUI (message thread): il valore vero viene scritto in processBlock
    // (audio thread) dentro un std::atomic, quindi la lettura e' thread-safe.
    double getDisplayBpm() const noexcept { return displayBpm.load(); }
    bool isUsingHostTempo() const noexcept { return usingHostTempo.load(); }

    juce::AudioProcessorValueTreeState apvts;

    static constexpr const char* SYNC_PARAM_ID       = "syncToHost";
    static constexpr const char* MANUAL_BPM_PARAM_ID = "manualBpm";

    static const std::array<NoteValue, 6> noteValues;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<double> displayBpm   { 120.0 };
    std::atomic<bool>   usingHostTempo { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TempoConvertAudioProcessor)
};
