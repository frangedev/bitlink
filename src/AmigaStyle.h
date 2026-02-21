#pragma once

#include <JuceHeader.h>

/** Amiga Workbench / tracker-style look: 3D bevels, copper-blue palette, chunky controls. */
class AmigaStyleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AmigaStyleLookAndFeel();

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                             const juce::Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted,
                             bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    static juce::Colour background();
    static juce::Colour panel();
    static juce::Colour buttonFace();
    static juce::Colour buttonHighlight();
    static juce::Colour buttonShadow();
    static juce::Colour textColour();
    static juce::Colour accent();
};

/** Simple geometric icons for buttons (paths in 0..1 unit box). */
namespace BitLinkIcons
{
    juce::Path createLoadIcon();      // folder
    juce::Path createClearIcon();     // X / trash
    juce::Path createAutoSliceIcon(); // waveform + dividers
    juce::Path createNormIcon();      // peak / level
    juce::Path createReverseIcon();   // swap arrows
    juce::Path createExport8SVXIcon(); // disk + 8
    juce::Path createExportWAVIcon();  // waveform
    juce::Path createSavePresetIcon(); // disk
    juce::Path createLoadPresetIcon(); // folder + doc
    juce::Path createSaveSoundIcon();  // disk + wave
    juce::Path createLoadSoundIcon();  // folder + wave
    juce::Path createInfoIcon();       // i
}

/** Button that draws an Amiga-style bevel + icon + text. */
class IconTextButton : public juce::TextButton
{
public:
    IconTextButton() = default;
    void setIconPath(const juce::Path& path) { iconPath = path; }
    void paint(juce::Graphics& g) override;

private:
    juce::Path iconPath;
};
