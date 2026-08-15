#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TempoConvertAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit TempoConvertAudioProcessorEditor (TempoConvertAudioProcessor&);
    ~TempoConvertAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildTable (double bpm);
    void copyToClipboard (const juce::String& text);

    static juce::String formatMs (double ms);
    static juce::String formatHz (double hz);

    TempoConvertAudioProcessor& processor;

    juce::Label bpmValueLabel;
    juce::Label bpmUnitLabel;
    juce::Slider manualBpmSlider;
    juce::ToggleButton syncButton { "Sync al tempo host" };
    juce::Label statusLabel;

    juce::Label headerNote, headerStraight, headerDotted, headerTriplet;
    juce::OwnedArray<juce::Label> nameCells;
    juce::OwnedArray<juce::TextButton> straightCells;
    juce::OwnedArray<juce::TextButton> dottedCells;
    juce::OwnedArray<juce::TextButton> tripletCells;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> manualBpmAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;

    double lastDisplayedBpm = -1.0;
    double lastBeatPhaseOrigin = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TempoConvertAudioProcessorEditor)
};
