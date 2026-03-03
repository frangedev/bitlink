#pragma once

#include <JuceHeader.h>

class BitLinkProcessor;

namespace bitlink {

/** Save/load full BitLink presets (all settings + sample + slices). */
struct PresetFormat
{
    static const char* presetExtension() { return ".bitlinkpreset"; }
    static const char* slicesExtension() { return ".bitlink-slices"; }

    /** Save full preset to file. Returns true on success. */
    static bool savePreset(const juce::File& file, BitLinkProcessor& processor);

    /** Load full preset from file. Returns true on success. */
    static bool loadPreset(const juce::File& file, BitLinkProcessor& processor);

    /** Save sound only: WAV + optional .bitlink-slices. Returns true on success. */
    static bool saveSound(const juce::File& wavFile, BitLinkProcessor& processor);

    /** Load sound: WAV + optional .bitlink-slices (same base path). Returns true on success. */
    static bool loadSound(const juce::File& wavFile, BitLinkProcessor& processor);
};

} // namespace bitlink
