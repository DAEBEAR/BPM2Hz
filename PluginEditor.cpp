#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace Theme
{
    const juce::Colour bg        (0xff141317);
    const juce::Colour panel     (0xff1b1a20);
    const juce::Colour panel2    (0xff201f26);
    const juce::Colour line      (0xff302f38);
    const juce::Colour amber     (0xffffb238);
    const juce::Colour amberDim  (0xff8a611f);
    const juce::Colour cyan      (0xff5fe0c9);
    const juce::Colour text      (0xffeee9e0);
    const juce::Colour muted     (0xff84808f);
}

juce::String TempoConvertAudioProcessorEditor::formatMs (double ms)
{
    if (ms >= 1000.0)
        return juce::String (ms / 1000.0, 2) + "s";
    return juce::String (ms, 1) + "ms";
}

juce::String TempoConvertAudioProcessorEditor::formatHz (double hz)
{
    return juce::String (hz, hz < 10.0 ? 2 : 1) + "Hz";
}

TempoConvertAudioProcessorEditor::TempoConvertAudioProcessorEditor (TempoConvertAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (nullptr); // usa il L&F di default, coloriamo i singoli componenti

    // --- BPM display ---
    bpmValueLabel.setJustificationType (juce::Justification::centredLeft);
    bpmValueLabel.setFont (juce::Font (juce::FontOptions (44.0f, juce::Font::bold)));
    bpmValueLabel.setColour (juce::Label::textColourId, Theme::text);
    addAndMakeVisible (bpmValueLabel);

    bpmUnitLabel.setText ("BPM", juce::dontSendNotification);
    bpmUnitLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    bpmUnitLabel.setColour (juce::Label::textColourId, Theme::muted);
    addAndMakeVisible (bpmUnitLabel);

    // --- manual BPM slider (attivo solo quando "Sync to host" e' spento) ---
    manualBpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    manualBpmSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    manualBpmSlider.setRange (20.0, 300.0, 0.01);
    manualBpmSlider.setColour (juce::Slider::thumbColourId, Theme::amber);
    manualBpmSlider.setColour (juce::Slider::trackColourId, Theme::amberDim);
    manualBpmSlider.setColour (juce::Slider::textBoxTextColourId, Theme::text);
    manualBpmSlider.setColour (juce::Slider::textBoxOutlineColourId, Theme::line);
    addAndMakeVisible (manualBpmSlider);
    manualBpmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, TempoConvertAudioProcessor::MANUAL_BPM_PARAM_ID, manualBpmSlider);

    // --- sync toggle ---
    syncButton.setColour (juce::ToggleButton::textColourId, Theme::text);
    syncButton.setColour (juce::ToggleButton::tickColourId, Theme::amber);
    addAndMakeVisible (syncButton);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, TempoConvertAudioProcessor::SYNC_PARAM_ID, syncButton);

    statusLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    statusLabel.setColour (juce::Label::textColourId, Theme::muted);
    addAndMakeVisible (statusLabel);

    // --- header tabella ---
    auto setupHeader = [this] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        l.setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)));
        l.setColour (juce::Label::textColourId, Theme::muted);
        l.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (l);
    };
    setupHeader (headerNote,     "NOTA");
    setupHeader (headerStraight, "DRITTA");
    setupHeader (headerDotted,   "PUNTO");
    setupHeader (headerTriplet,  "TERZINA");
    headerNote.setJustificationType (juce::Justification::centredLeft);

    // --- righe tabella ---
    for (auto& nv : TempoConvertAudioProcessor::noteValues)
    {
        auto* nameLabel = nameCells.add (new juce::Label());
        nameLabel->setText (nv.name, juce::dontSendNotification);
        nameLabel->setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
        nameLabel->setColour (juce::Label::textColourId, Theme::text);
        addAndMakeVisible (nameLabel);

        for (auto* arrayPtr : { &straightCells, &dottedCells, &tripletCells })
        {
            auto* btn = arrayPtr->add (new juce::TextButton());
            btn->setColour (juce::TextButton::buttonColourId, Theme::panel2);
            btn->setColour (juce::TextButton::buttonOnColourId, Theme::panel2);
            btn->setColour (juce::TextButton::textColourOffId, Theme::amber);
            addAndMakeVisible (btn);
        }
    }

    setSize (560, 480);
    rebuildTable (processor.getDisplayBpm());
    startTimerHz (20);
}

TempoConvertAudioProcessorEditor::~TempoConvertAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void TempoConvertAudioProcessorEditor::copyToClipboard (const juce::String& text)
{
    juce::SystemClipboard::copyTextToClipboard (text);
    statusLabel.setText ("copiato: " + text, juce::dontSendNotification);
}

void TempoConvertAudioProcessorEditor::rebuildTable (double bpm)
{
    if (bpm <= 0.0)
        bpm = 120.0;

    const double beatMs = 60000.0 / bpm;

    for (size_t i = 0; i < TempoConvertAudioProcessor::noteValues.size(); ++i)
    {
        const auto& nv = TempoConvertAudioProcessor::noteValues[i];
        const double straightMs = beatMs * nv.beatMultiplier;
        const double dottedMs   = straightMs * 1.5;
        const double tripletMs  = straightMs * (2.0 / 3.0);

        struct Col { juce::OwnedArray<juce::TextButton>& cells; double ms; };
        Col cols[] = { { straightCells, straightMs }, { dottedCells, dottedMs }, { tripletCells, tripletMs } };

        for (auto& col : cols)
        {
            auto* btn = col.cells[(int) i];
            const juce::String label = formatMs (col.ms) + "  /  " + formatHz (1000.0 / col.ms);
            btn->setButtonText (label);

            const juce::String copyText = juce::String (col.ms, 2) + "ms / " + juce::String (1000.0 / col.ms, 3) + "Hz";
            btn->onClick = [this, copyText] { copyToClipboard (copyText); };
        }
    }
}

void TempoConvertAudioProcessorEditor::timerCallback()
{
    const double bpm = processor.getDisplayBpm();

    bpmValueLabel.setText (juce::String (bpm, 1), juce::dontSendNotification);
    statusLabel.setText (processor.isUsingHostTempo() ? "sincronizzato con la DAW" : "tempo manuale",
                          juce::dontSendNotification);

    if (std::abs (bpm - lastDisplayedBpm) > 0.005)
    {
        rebuildTable (bpm);
        lastDisplayedBpm = bpm;
    }

    repaint(); // per l'animazione del LED pulsante
}

void TempoConvertAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::bg);

    // pannello hero
    auto heroBounds = getLocalBounds().removeFromTop (120).reduced (16, 12).toFloat();
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (heroBounds, 14.0f);
    g.setColour (Theme::line);
    g.drawRoundedRectangle (heroBounds, 14.0f, 1.0f);

    // LED pulsante sincronizzato al bpm corrente
    const double bpm = processor.getDisplayBpm() > 0.0 ? processor.getDisplayBpm() : 120.0;
    const double beatMs = 60000.0 / bpm;
    const double phase = std::fmod (juce::Time::getMillisecondCounterHiRes(), beatMs) / beatMs;
    const float radius = 7.0f + (float) (1.0 - phase) * 5.0f;
    const float alpha  = (float) (1.0 - phase) * 0.9f + 0.1f;

    juce::Point<float> dotCentre (heroBounds.getX() + 24.0f, heroBounds.getCentreY());
    g.setColour (Theme::amber.withAlpha (alpha));
    g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (dotCentre));

    // pannello tabella
    auto tableBounds = getLocalBounds().withTrimmedTop (132).reduced (16, 0).toFloat();
    tableBounds.removeFromBottom (12);
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (tableBounds, 14.0f);
    g.setColour (Theme::line);
    g.drawRoundedRectangle (tableBounds, 14.0f, 1.0f);
}

void TempoConvertAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    auto hero = area.removeFromTop (120).reduced (16, 12);
    hero.removeFromLeft (44); // spazio per il LED disegnato in paint()

    auto bpmRow = hero.removeFromTop (56);
    bpmValueLabel.setBounds (bpmRow.removeFromLeft (140));
    bpmUnitLabel.setBounds (bpmRow.removeFromLeft (40));

    auto controlsRow = hero;
    syncButton.setBounds (controlsRow.removeFromTop (24));
    manualBpmSlider.setBounds (controlsRow.removeFromTop (24));
    statusLabel.setBounds (controlsRow.removeFromTop (20));

    area.removeFromTop (12);
    auto table = area.reduced (16, 0);
    table.removeFromBottom (12);

    auto headerRow = table.removeFromTop (30);
    const int nameW = 100;
    const int colW = (headerRow.getWidth() - nameW) / 3;
    headerNote.setBounds (headerRow.removeFromLeft (nameW).reduced (14, 0));
    headerStraight.setBounds (headerRow.removeFromLeft (colW));
    headerDotted.setBounds (headerRow.removeFromLeft (colW));
    headerTriplet.setBounds (headerRow);

    const int rowH = (table.getHeight()) / (int) TempoConvertAudioProcessor::noteValues.size();

    for (size_t i = 0; i < TempoConvertAudioProcessor::noteValues.size(); ++i)
    {
        auto row = table.removeFromTop (rowH);
        nameCells[(int) i]->setBounds (row.removeFromLeft (nameW).reduced (14, 0));
        straightCells[(int) i]->setBounds (row.removeFromLeft (colW).reduced (4, 6));
        dottedCells[(int) i]->setBounds (row.removeFromLeft (colW).reduced (4, 6));
        tripletCells[(int) i]->setBounds (row.reduced (4, 6));
    }
}
