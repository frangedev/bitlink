#pragma once

#include <JuceHeader.h>

namespace bitlink {

/** In-place sample utility functions. */
struct SampleTools
{
    /** Reverse the buffer in place (all channels). */
    static void reverse(juce::AudioBuffer<float>& buffer);

    /** Normalize so peak is targetLevel (0..1). Returns actual peak before normalization. */
    static float normalize(juce::AudioBuffer<float>& buffer, float targetLevel = 0.95f);

    /** Apply gain to buffer (linear). */
    static void applyGain(juce::AudioBuffer<float>& buffer, float gain);

    /** Get peak level (mono or stereo). */
    static float getPeakLevel(const juce::AudioBuffer<float>& buffer);

    /** Trim silence from start/end (threshold 0..1). Returns new start sample. */
    static int trimSilence(juce::AudioBuffer<float>& buffer, float threshold = 0.001f);
};

} // namespace bitlink
