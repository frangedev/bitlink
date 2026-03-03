#pragma once

#include <JuceHeader.h>

namespace bitlink {

/** Export float buffer to WAV file. */
class WAVExport
{
public:
    /** Write WAV file. Returns true on success. */
    static bool writeToFile(const juce::File& file,
                           const juce::AudioBuffer<float>& buffer,
                           double sampleRate,
                           int numBitsPerSample = 24);
};

} // namespace bitlink
