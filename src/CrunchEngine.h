#pragma once

#include <JuceHeader.h>

namespace bitlink {

/** Fixed 22 kHz resampling and 8-bit / 12-bit quantization (PAULA-style crunch). */
class CrunchEngine
{
public:
    enum class BitDepth { Bits8 = 8, Bits12 = 12 };

    CrunchEngine() = default;

    void setBitDepth(BitDepth depth);
    BitDepth getBitDepth() const { return bitDepth; }

    /** Resample and quantize a buffer in place. In: any sample rate, out: 22 kHz-style crunch. */
    void process(juce::AudioBuffer<float>& buffer, double sourceSampleRate);

    /** Process a single channel buffer (mono). Used when resampling loaded sample to "crunch" rate. */
    void processMono(juce::AudioBuffer<float>& buffer, double sourceSampleRate);

    static constexpr double kCrunchSampleRate = 22050.0;

private:
    BitDepth bitDepth = BitDepth::Bits8;
    float linearToQuantized(float x) const;
    void resampleTo22k(juce::AudioBuffer<float>& buffer, double sourceSampleRate);
};

} // namespace bitlink
