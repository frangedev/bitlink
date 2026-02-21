#include "PluginEditor.h"
#include "IFF8SVXExport.h"
#include "WAVExport.h"
#include "PresetFormat.h"
#include <functional>

struct WaveformComponent : juce::Component
{
    std::function<void(const juce::MouseEvent&)> onMouseDownCallback;
    std::function<void(const juce::MouseEvent&)> onMouseUpCallback;
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (onMouseDownCallback)
            onMouseDownCallback(e);
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        if (onMouseUpCallback)
            onMouseUpCallback(e);
    }
};

BitLinkEditor::BitLinkEditor(BitLinkProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setWantsKeyboardFocus(true);
    setLookAndFeel(&amigaLookAndFeel);
    buildAuraLook();
    addAndMakeVisible(titleLabel);
    titleLabel.setText("BitLink", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, AmigaStyleLookAndFeel::textColour());

    loadButton.setIconPath(BitLinkIcons::createLoadIcon());
    loadButton.setButtonText("Load");
    loadButton.onClick = [this] {
        auto fc = std::make_shared<juce::FileChooser>("Load WAV/AIFF", juce::File(), "*.wav;*.aiff;*.aif");
        fc->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, fc](const juce::FileChooser& chooser) {
                            if (chooser.getResult() != juce::File())
                                processorRef.loadSampleFromFile(chooser.getResult());
                        });
    };
    addAndMakeVisible(loadButton);

    clearButton.setIconPath(BitLinkIcons::createClearIcon());
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] {
        processorRef.clearSample();
        updateSliceRects();
        updateWaveform();
    };
    addAndMakeVisible(clearButton);

    autoSliceButton.setIconPath(BitLinkIcons::createAutoSliceIcon());
    autoSliceButton.setButtonText("Slice");
    autoSliceButton.onClick = [this] {
        float sens = (float)sensitivitySlider.getValue();
        processorRef.runAutoSlice(juce::jlimit(0.0f, 1.0f, sens));
        updateSliceRects();
        updateWaveform();
    };
    addAndMakeVisible(autoSliceButton);

    normalizeButton.setIconPath(BitLinkIcons::createNormIcon());
    normalizeButton.setButtonText("Norm");
    normalizeButton.onClick = [this] {
        if (processorRef.hasSample())
            processorRef.normalizeSample(0.95f);
        updateWaveform();
    };
    addAndMakeVisible(normalizeButton);

    reverseButton.setIconPath(BitLinkIcons::createReverseIcon());
    reverseButton.setButtonText("Rev");
    reverseButton.onClick = [this] {
        processorRef.reverseSample();
        updateWaveform();
    };
    addAndMakeVisible(reverseButton);

    export8SVXButton.setIconPath(BitLinkIcons::createExport8SVXIcon());
    export8SVXButton.setButtonText("8SVX");
    export8SVXButton.onClick = [this] {
        if (!processorRef.hasSample())
            return;
        auto fc = std::make_shared<juce::FileChooser>("Export 8SVX", juce::File(), "*.8svx;*.iff");
        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                        [this, fc](const juce::FileChooser& chooser) {
                            juce::File f = chooser.getResult();
                            if (f != juce::File())
                            {
                                const auto& buf = processorRef.getSampleBuffer();
                                const float* data = buf.getReadPointer(0);
                                int n = buf.getNumSamples();
                                double sr = processorRef.getSampleRate();
                                bitlink::IFF8SVXExport::writeToFile(f, data, n, sr, "BitLink");
                            }
                        });
    };
    addAndMakeVisible(export8SVXButton);

    exportWAVButton.setIconPath(BitLinkIcons::createExportWAVIcon());
    exportWAVButton.setButtonText("WAV");
    exportWAVButton.onClick = [this] {
        if (!processorRef.hasSample())
            return;
        auto fc = std::make_shared<juce::FileChooser>("Export WAV", juce::File(), "*.wav");
        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                        [this, fc](const juce::FileChooser& chooser) {
                            juce::File f = chooser.getResult();
                            if (f != juce::File())
                                bitlink::WAVExport::writeToFile(f, processorRef.getSampleBuffer(),
                                                                processorRef.getSampleRate(), 24);
                        });
    };
    addAndMakeVisible(exportWAVButton);

    savePresetButton.setIconPath(BitLinkIcons::createSavePresetIcon());
    savePresetButton.setButtonText("Preset");
    savePresetButton.onClick = [this] {
        auto fc = std::make_shared<juce::FileChooser>("Save preset", juce::File(), "*" + juce::String(bitlink::PresetFormat::presetExtension()));
        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                        [this, fc](const juce::FileChooser& chooser) {
                            juce::File f = chooser.getResult();
                            if (f != juce::File())
                            {
                                if (!f.hasFileExtension("bitlinkpreset"))
                                    f = f.withFileExtension("bitlinkpreset");
                                bitlink::PresetFormat::savePreset(f, processorRef);
                            }
                        });
    };
    addAndMakeVisible(savePresetButton);

    loadPresetButton.setIconPath(BitLinkIcons::createLoadPresetIcon());
    loadPresetButton.setButtonText("Load P");
    loadPresetButton.onClick = [this] {
        auto fc = std::make_shared<juce::FileChooser>("Load preset", juce::File(), "*" + juce::String(bitlink::PresetFormat::presetExtension()));
        fc->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, fc](const juce::FileChooser& chooser) {
                            juce::File f = chooser.getResult();
                            if (f != juce::File() && bitlink::PresetFormat::loadPreset(f, processorRef))
                            {
                                gainSlider.setValue(processorRef.getSampleGain(), juce::dontSendNotification);
                                stretchSlider.setValue(processorRef.getStretchFactor(), juce::dontSendNotification);
                                baseNoteCombo.setSelectedId(processorRef.getBaseMidiNote() + 1, juce::dontSendNotification);
                                bitDepthCombo.setSelectedId(processorRef.getCrunchBitDepth() == bitlink::CrunchEngine::BitDepth::Bits12 ? 2 : 1, juce::dontSendNotification);
                                stretchToggle.setToggleState(processorRef.isStretchEnabled(), juce::dontSendNotification);
                                thruToggle.setToggleState(processorRef.isSlicerThruMode(), juce::dontSendNotification);
                                updateSliceRects();
                                updateWaveform();
                                refreshInfoText();
                            }
                        });
    };
    addAndMakeVisible(loadPresetButton);

    saveSoundButton.setIconPath(BitLinkIcons::createSaveSoundIcon());
    saveSoundButton.setButtonText("Sound");
    saveSoundButton.onClick = [this] {
        if (!processorRef.hasSample())
            return;
        auto fc = std::make_shared<juce::FileChooser>("Save sound (WAV + slices)", juce::File(), "*.wav");
        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                        [this, fc](const juce::FileChooser& chooser) {
                            juce::File f = chooser.getResult();
                            if (f != juce::File())
                                bitlink::PresetFormat::saveSound(f, processorRef);
                        });
    };
    addAndMakeVisible(saveSoundButton);

    loadSoundButton.setIconPath(BitLinkIcons::createLoadSoundIcon());
    loadSoundButton.setButtonText("Load S");
    loadSoundButton.onClick = [this] {
        auto fc = std::make_shared<juce::FileChooser>("Load sound (WAV + slices)", juce::File(), "*.wav;*.aiff;*.aif");
        fc->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, fc](const juce::FileChooser& chooser) {
                            juce::File f = chooser.getResult();
                            if (f != juce::File() && bitlink::PresetFormat::loadSound(f, processorRef))
                            {
                                updateSliceRects();
                                updateWaveform();
                                refreshInfoText();
                            }
                        });
    };
    addAndMakeVisible(loadSoundButton);

    panelToggleButton.setIconPath(BitLinkIcons::createInfoIcon());
    panelToggleButton.setButtonText("Info");
    panelToggleButton.onClick = [this] {
        panelVisible = !panelVisible;
        infoPanel.setVisible(panelVisible);
        if (panelVisible)
            infoPanel.toFront(true);
        refreshInfoText();
        resized();
    };
    addAndMakeVisible(panelToggleButton);

    infoPanel.setVisible(false);
    infoLabel.setJustificationType(juce::Justification::topLeft);
    infoLabel.setColour(juce::Label::textColourId, AmigaStyleLookAndFeel::textColour());
    infoLabel.setColour(juce::Label::backgroundColourId, AmigaStyleLookAndFeel::panel());
    infoPanel.addAndMakeVisible(infoLabel);
    addAndMakeVisible(infoPanel);

    stretchToggle.setButtonText("Stretch");
    stretchToggle.onClick = [this] {
        processorRef.setStretchEnabled(stretchToggle.getToggleState());
    };
    addAndMakeVisible(stretchToggle);

    thruToggle.setButtonText("Thru");
    thruToggle.onClick = [this] {
        processorRef.setSlicerThruMode(thruToggle.getToggleState());
    };
    addAndMakeVisible(thruToggle);

    bitDepthCombo.addItem("8-bit", 1);
    bitDepthCombo.addItem("12-bit", 2);
    bitDepthCombo.setSelectedId(1);
    bitDepthCombo.onChange = [this] {
        bool is12 = bitDepthCombo.getSelectedId() == 2;
        processorRef.setCrunchBitDepth(is12 ? bitlink::CrunchEngine::BitDepth::Bits12 : bitlink::CrunchEngine::BitDepth::Bits8);
    };
    addAndMakeVisible(bitDepthCombo);

    for (int i = 0; i < 128; ++i)
        baseNoteCombo.addItem(juce::MidiMessage::getMidiNoteName(i, true, true, 4), i + 1);
    baseNoteCombo.setSelectedId(61);
    baseNoteCombo.onChange = [this] {
        processorRef.setBaseMidiNote(baseNoteCombo.getSelectedId() - 1);
    };
    addAndMakeVisible(baseNoteCombo);

    gainSlider.setRange(0.0, 4.0, 0.01);
    gainSlider.setValue(1.0);
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 18);
    gainSlider.onValueChange = [this] {
        processorRef.setSampleGain((float)gainSlider.getValue());
    };
    addAndMakeVisible(gainSlider);

    stretchSlider.setRange(0.25, 4.0, 0.01);
    stretchSlider.setValue(1.0);
    stretchSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    stretchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    stretchSlider.onValueChange = [this] {
        processorRef.setStretchFactor((float)stretchSlider.getValue());
    };
    addAndMakeVisible(stretchSlider);

    sensitivitySlider.setRange(0.0, 1.0, 0.05);
    sensitivitySlider.setValue(0.5);
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(sensitivitySlider);

    bpmSlider.setRange(40.0, 300.0, 1.0);
    bpmSlider.setValue(120.0);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 20);
    bpmSlider.setTextValueSuffix(" BPM");
    addAndMakeVisible(bpmSlider);
    bpmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.getAPVTS(), "bpm", bpmSlider);

    waveformDisplay = std::make_unique<WaveformComponent>();
    waveformDisplay->setInterceptsMouseClicks(true, false);
    waveformDisplay->onMouseDownCallback = [this](const juce::MouseEvent& e) {
        if (e.mods.isRightButtonDown())
        {
            if (!processorRef.hasSample())
                return;
            int len = processorRef.getSampleLength();
            int x = e.getPosition().getX();
            int w = waveformDisplay->getWidth();
            if (w > 0)
                processorRef.removeSliceNear((x * len) / w);
            updateSliceRects();
            updateWaveform();
        }
        else if (e.getNumberOfClicks() >= 2)
        {
            if (!processorRef.hasSample())
                return;
            int len = processorRef.getSampleLength();
            int x = e.getPosition().getX();
            int w = waveformDisplay->getWidth();
            if (w > 0)
                processorRef.addSliceAt((x * len) / w);
            updateSliceRects();
            updateWaveform();
        }
        else if (e.mods.isLeftButtonDown() && !e.mods.isAnyModifierKeyDown())
        {
            if (processorRef.hasSample())
            {
                int len = processorRef.getSampleLength();
                int w = waveformDisplay->getWidth();
                if (w > 0)
                {
                    int sampleIdx = (e.getPosition().getX() * len) / w;
                    sampleIdx = juce::jlimit(0, len - 1, sampleIdx);
                    processorRef.setPreviewPlayback(sampleIdx, true);
                }
            }
        }
    };
    waveformDisplay->onMouseUpCallback = [this](const juce::MouseEvent&) {
        processorRef.setPreviewPlayback(0, false);
    };
    addAndMakeVisible(*waveformDisplay);

    setSize(800, 480);
    startTimerHz(20);
}

BitLinkEditor::~BitLinkEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void BitLinkEditor::buildAuraLook()
{
    auto& laf = getLookAndFeel();
    laf.setColour(juce::ResizableWindow::backgroundColourId, AmigaStyleLookAndFeel::background());
    laf.setColour(juce::ComboBox::backgroundColourId, AmigaStyleLookAndFeel::panel());
    laf.setColour(juce::ComboBox::textColourId, AmigaStyleLookAndFeel::textColour());
    laf.setColour(juce::PopupMenu::backgroundColourId, AmigaStyleLookAndFeel::panel());
    laf.setColour(juce::Slider::thumbColourId, AmigaStyleLookAndFeel::accent());
    laf.setColour(juce::Slider::rotarySliderFillColourId, AmigaStyleLookAndFeel::buttonFace());
    laf.setColour(juce::Slider::trackColourId, AmigaStyleLookAndFeel::panel());
    laf.setColour(juce::TextButton::buttonColourId, AmigaStyleLookAndFeel::buttonFace());
    laf.setColour(juce::TextButton::textColourOffId, AmigaStyleLookAndFeel::textColour());
    laf.setColour(juce::ToggleButton::tickColourId, AmigaStyleLookAndFeel::accent());
    laf.setColour(juce::ToggleButton::textColourId, AmigaStyleLookAndFeel::textColour());
}

void BitLinkEditor::paint(juce::Graphics& g)
{
    g.fillAll(AmigaStyleLookAndFeel::background());
    if (waveformDisplay)
    {
        auto wfBounds = waveformDisplay->getBounds().toFloat();
        g.setColour(AmigaStyleLookAndFeel::panel());
        g.fillRect(waveformDisplay->getBounds());
        // Draw slice regions in waveform coordinates (offset by display position)
        g.setColour(AmigaStyleLookAndFeel::accent().withAlpha(0.14f));
        for (const auto& r : sliceRects)
            g.fillRect(r + wfBounds.getPosition());
        // Slice boundary lines (between slices, not at left edge)
        g.setColour(AmigaStyleLookAndFeel::accent().withAlpha(0.55f));
        for (const auto& r : sliceRects)
        {
            if (r.getX() > 0.5f)
                g.drawVerticalLine(juce::roundToInt(r.getX() + wfBounds.getX()), wfBounds.getY(), wfBounds.getBottom());
        }
        g.setColour(AmigaStyleLookAndFeel::buttonFace());
        g.drawRect(waveformDisplay->getBounds(), 1);
        if (processorRef.hasSample())
        {
            int len = processorRef.getSampleLength();
            double sr = processorRef.getSampleRate();
            float bpm = processorRef.getBPM();
            auto r = waveformDisplay->getBounds().toFloat();
            // Time/beat guides: vertical lines at beat boundaries
            if (sr > 0 && bpm > 0 && len > 0)
            {
                double samplesPerBeat = (60.0 / (double)bpm) * sr;
                g.setColour(AmigaStyleLookAndFeel::textColour().withAlpha(0.22f));
                for (double s = samplesPerBeat; s < (double)len; s += samplesPerBeat)
                {
                    float x = r.getX() + r.getWidth() * (float)(s / (double)len);
                    if (x >= r.getX() && x <= r.getRight())
                        g.drawVerticalLine(juce::roundToInt(x), r.getY(), r.getBottom());
                }
            }
            const auto& buf = processorRef.getSampleBuffer();
            const float* data = buf.getReadPointer(0);
            g.setColour(AmigaStyleLookAndFeel::textColour());
            juce::Path path;
            for (int i = 0; i < len; ++i)
            {
                float x = r.getX() + r.getWidth() * (float)i / (float)len;
                float y = r.getCentreY() - data[i] * r.getHeight() * 0.45f;
                if (i == 0)
                    path.startNewSubPath(x, y);
                else
                    path.lineTo(x, y);
            }
            g.strokePath(path, juce::PathStrokeType(1.0f));

            int playPos = processorRef.getPlaybackPositionSample();
            if (playPos >= 0 && len > 0)
            {
                float x = r.getX() + r.getWidth() * (float)playPos / (float)len;
                x = juce::jlimit(r.getX(), r.getRight(), x);
                g.setColour(AmigaStyleLookAndFeel::accent());
                g.drawLine(x, r.getY(), x, r.getBottom(), 2.0f);
            }
        }
    }
}

void BitLinkEditor::resized()
{
    auto b = getLocalBounds();
    titleLabel.setBounds(b.removeFromTop(26).reduced(6, 2));
    auto row1 = b.removeFromTop(30).reduced(4);
    loadButton.setBounds(row1.removeFromLeft(68));
    clearButton.setBounds(row1.removeFromLeft(48));
    autoSliceButton.setBounds(row1.removeFromLeft(68));
    normalizeButton.setBounds(row1.removeFromLeft(40));
    reverseButton.setBounds(row1.removeFromLeft(32));
    row1.removeFromLeft(4);
    sensitivitySlider.setBounds(row1.removeFromLeft(90));
    thruToggle.setBounds(row1.removeFromLeft(40));
    stretchToggle.setBounds(row1.removeFromLeft(52));
    stretchSlider.setBounds(row1.removeFromLeft(72));
    bitDepthCombo.setBounds(row1.removeFromLeft(64));
    baseNoteCombo.setBounds(row1.removeFromLeft(64));
    bpmSlider.setBounds(row1.removeFromLeft(72));
    gainSlider.setBounds(row1.removeFromLeft(90));
    export8SVXButton.setBounds(row1.removeFromLeft(40));
    exportWAVButton.setBounds(row1.removeFromLeft(40));

    auto row2 = b.removeFromTop(28).reduced(4);
    savePresetButton.setBounds(row2.removeFromLeft(82));
    loadPresetButton.setBounds(row2.removeFromLeft(82));
    saveSoundButton.setBounds(row2.removeFromLeft(82));
    loadSoundButton.setBounds(row2.removeFromLeft(82));
    panelToggleButton.setBounds(row2.removeFromLeft(100));

    b.removeFromTop(4);
    if (panelVisible)
    {
        int panelH = juce::jmin(140, b.getHeight() / 2);
        infoPanel.setBounds(b.removeFromBottom(panelH).reduced(6, 0));
        infoLabel.setBounds(8, 8, juce::jmax(0, infoPanel.getWidth() - 16), juce::jmax(0, infoPanel.getHeight() - 16));
    }
    else
        infoPanel.setBounds(0, 0, 0, 0);
    if (waveformDisplay)
        waveformDisplay->setBounds(b.reduced(6, 0));
    updateSliceRects();
}

bool BitLinkEditor::keyPressed(const juce::KeyPress& key)
{
    runShortcutAction(key.getKeyCode(), key.getModifiers());
    return true;
}

void BitLinkEditor::runShortcutAction(int key, juce::ModifierKeys mods)
{
    if (key == 'l' || key == 'L')
        loadButton.triggerClick();
    else if (key == 'c' || key == 'C')
        clearButton.triggerClick();
    else if (key == 'a' || key == 'A')
        autoSliceButton.triggerClick();
    else if (key == 'n' || key == 'N')
        normalizeButton.triggerClick();
    else if (key == 'r' || key == 'R')
        reverseButton.triggerClick();
    else if (key == '8')
        export8SVXButton.triggerClick();
    else if (key == 'w' || key == 'W')
        exportWAVButton.triggerClick();
    else if (key == 's' || key == 'S')
    {
        if (mods.isShiftDown())
            savePresetButton.triggerClick();
        else
            saveSoundButton.triggerClick();
    }
    else if (key == 'o' || key == 'O')
        loadPresetButton.triggerClick();
    else if (key == 'i' || key == 'I')
        panelToggleButton.triggerClick();
}

void BitLinkEditor::refreshInfoText()
{
    juce::String t;
    t << "--- Sample ---\n";
    if (processorRef.hasSample())
    {
        double sec = processorRef.getSampleLength() / processorRef.getSampleRate();
        t << "Length: " << processorRef.getSampleLength() << " samples ("
          << juce::String(sec, 2) << " s)\n";
        t << "Rate: " << (int)processorRef.getSampleRate() << " Hz\n";
        t << "Slices: " << processorRef.getSlicer().getNumSlices() << "\n";
    }
    else
        t << "No sample loaded\n";
    t << "BPM: " << (int)processorRef.getBPM() << "\n";
    t << "\n--- Shortcuts ---\n";
    t << "L Load  C Clear  A Auto-Slice  N Norm  R Rev\n";
    t << "8 8SVX  W WAV  S Save sound  Shift+S Save preset  O Load preset\n";
    t << "I Info panel";
    infoLabel.setText(t, juce::dontSendNotification);
}

void BitLinkEditor::timerCallback()
{
    updateWaveform();
}

void BitLinkEditor::updateWaveform()
{
    repaint();
}

void BitLinkEditor::updateSliceRects()
{
    sliceRects.clear();
    if (!processorRef.hasSample() || !waveformDisplay)
        return;
    int len = processorRef.getSampleLength();
    int w = waveformDisplay->getWidth();
    int h = waveformDisplay->getHeight();
    if (w <= 0 || len <= 0)
        return;
    const auto& slicer = processorRef.getSlicer();
    for (int i = 0; i < slicer.getNumSlices(); ++i)
    {
        int start = slicer.getSliceStart(i);
        int end = slicer.getSliceEnd(i);
        int x0 = (start * w) / len;
        int x1 = (end * w) / len;
        if (x1 > x0)
            sliceRects.push_back(juce::Rectangle<float>((float)x0, 0.0f, (float)(x1 - x0), (float)h));
    }
}

bool BitLinkEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aiff") || f.endsWithIgnoreCase(".aif"))
            return true;
    return false;
}

void BitLinkEditor::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);
    for (const auto& path : files)
    {
        juce::File f(path);
        if (f.existsAsFile() && (f.hasFileExtension("wav") || f.hasFileExtension("aiff") || f.hasFileExtension("aif")))
        {
            processorRef.loadSampleFromFile(f);
            updateSliceRects();
            updateWaveform();
            break;
        }
    }
}
