#pragma once

#include "PluginProcessor.h"
#include "AmigaStyle.h"
#include <JuceHeader.h>

struct WaveformComponent;

class BitLinkEditor : public juce::AudioProcessorEditor,
                      public juce::FileDragAndDropTarget,
                      public juce::Timer
{
public:
    explicit BitLinkEditor(BitLinkProcessor&);
    ~BitLinkEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress& key) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    BitLinkProcessor& processorRef;
    AmigaStyleLookAndFeel amigaLookAndFeel;

    juce::Label titleLabel;
    IconTextButton loadButton;
    IconTextButton clearButton;
    IconTextButton autoSliceButton;
    IconTextButton normalizeButton;
    IconTextButton reverseButton;
    IconTextButton export8SVXButton;
    IconTextButton exportWAVButton;
    IconTextButton savePresetButton;
    IconTextButton loadPresetButton;
    IconTextButton saveSoundButton;
    IconTextButton loadSoundButton;
    IconTextButton panelToggleButton;
    juce::ToggleButton stretchToggle;
    juce::ToggleButton thruToggle;
    juce::ComboBox bitDepthCombo;
    juce::ComboBox baseNoteCombo;
    juce::Slider stretchSlider;
    juce::Slider sensitivitySlider;
    juce::Slider bpmSlider;
    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpmAttachment;

    std::unique_ptr<WaveformComponent> waveformDisplay;
    std::vector<juce::Rectangle<float>> sliceRects;

    juce::Component infoPanel;
    juce::Label infoLabel;
    bool panelVisible = false;

    void buildAuraLook();
    void updateWaveform();
    void updateSliceRects();
    void refreshInfoText();
    void runShortcutAction(int key, juce::ModifierKeys mods = juce::ModifierKeys());

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitLinkEditor)
};
