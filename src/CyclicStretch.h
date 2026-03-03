#pragma once

#include <JuceHeader.h>

namespace bitlink {

/** Granular time-stretch: grain size locked to pitch for metallic S1000-style artifacts. */
class CyclicStretch
{
public:
    CyclicStretch() = default;

    /** Set stretch factor (e.g. 2.0 = twice as long, same pitch). */
    void setStretchFactor(float factor);
    float getStretchFactor() const { return stretchFactor; }

    /** Set grain size in samples (or 0 = auto from original pitch). */
    void setGrainSize(int samples);
    int getGrainSize() const { return grainSize; }

    /** Process: read from source buffer, write stretched output. Mono. */
    void process(const float* source, int sourceLength,
                 float* dest, int destLength,
                 double sourceSampleRate);

    /** Reset read position. */
    void reset() { readPos = 0.0; }

    /** Current read position in samples (for playhead display). */
    int getReadPositionSamples() const { return static_cast<int>(readPos); }

private:
    float stretchFactor = 1.0f;
    int grainSize = 512;
    double readPos = 0.0;
};

} // namespace bitlink
