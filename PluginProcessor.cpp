#include "PluginProcessor.h"
#include "PluginEditor.h"

const std::array<NoteValue, 6> TempoConvertAudioProcessor::noteValues { {
    { "1/1",  "whole",      4.0    },
    { "1/2",  "half",       2.0    },
    { "1/4",  "quarter",    1.0    },
    { "1/8",  "eighth",     0.5    },
    { "1/16", "sixteenth",  0.25   },
    { "1/32", "thirty-2nd", 0.125  }
} };

TempoConvertAudioProcessor::TempoConvertAudioProcessor()
    : juce::AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout TempoConvertAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { SYNC_PARAM_ID, 1 }, "Sync to Host", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { MANUAL_BPM_PARAM_ID, 1 }, "Manual BPM",
        juce::NormalisableRange<float> (20.0f, 300.0f, 0.01f), 120.0f));

    return { params.begin(), params.end() };
}

void TempoConvertAudioProcessor::prepareToPlay (double, int) {}

bool TempoConvertAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
        && ! layouts.getMainOutputChannelSet().isDisabled();
}

void TempoConvertAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    // Plugin di utilita': l'audio passa invariato, non viene processato.
    juce::ignoreUnused (buffer);

    const bool sync = apvts.getRawParameterValue (SYNC_PARAM_ID)->load() > 0.5f;

    if (sync)
    {
        bool gotHostTempo = false;

        if (auto* playHead = getPlayHead())
        {
            if (auto position = playHead->getPosition())
            {
                if (auto bpm = position->getBpm())
                {
                    if (*bpm > 0.0)
                    {
                        displayBpm.store (*bpm);
                        gotHostTempo = true;
                    }
                }
            }
        }
        usingHostTempo.store (gotHostTempo);
    }
    else
    {
        displayBpm.store ((double) apvts.getRawParameterValue (MANUAL_BPM_PARAM_ID)->load());
        usingHostTempo.store (false);
    }
}

juce::AudioProcessorEditor* TempoConvertAudioProcessor::createEditor()
{
    return new TempoConvertAudioProcessorEditor (*this);
}

void TempoConvertAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (std::unique_ptr<juce::XmlElement> xml (state.createXml()); xml != nullptr)
        copyXmlToBinary (*xml, destData);
}

void TempoConvertAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// Punto di ingresso richiesto da JUCE per creare il plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TempoConvertAudioProcessor();
}
