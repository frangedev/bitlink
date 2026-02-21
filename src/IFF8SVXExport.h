#pragma once

#include <JuceHeader.h>

namespace bitlink {

/** Export mono float buffer to IFF/8SVX (Amiga) file. */
class IFF8SVXExport
{
public:
    /** Write 8SVX file. Float samples in [-1,1], sampleRate in Hz. Returns true on success. */
    static bool writeToFile(const juce::File& file,
                            const float* samples,
                            int numSamples,
                            double sampleRate,
                            const juce::String& name = "BitLink Export");
};

} // namespace bitlink
