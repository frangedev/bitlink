#include "CrunchEngine.h"

namespace bitlink {

void CrunchEngine::setBitDepth(BitDepth depth)
{
    bitDepth = depth;
}

float CrunchEngine::linearToQuantized(float x) const
{
    float scale;
    int levels;
    switch (bitDepth)
    {
    case BitDepth::Bits8:
        levels = 256;
        scale = 127.0f;
        break;
    case BitDepth::Bits12:
        levels = 4096;
        scale = 2047.0f;
        break;
    default:
        levels = 256;
        scale = 127.0f;
        break;
    }
    float clamped = juce::jlimit(-1.0f, 1.0f, x);
    float quantized = std::round(clamped * scale) / scale;
    return quantized;
}

void CrunchEngine::resampleTo22k(juce::AudioBuffer<float>& buffer, double sourceSampleRate)
{
    if (sourceSampleRate <= 0.0 || buffer.getNumSamples() == 0)
        return;

    const double ratio = kCrunchSampleRate / sourceSampleRate;
    const int newLength = static_cast<int>(std::ceil(buffer.getNumSamples() * ratio));
    if (newLength <= 0)
        return;

    juce::AudioBuffer<float> resampled(buffer.getNumChannels(), newLength);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const float* src = buffer.getReadPointer(ch);
        float* dst = resampled.getWritePointer(ch);
        for (int i = 0; i < newLength; ++i)
        {
            double srcIndex = i / ratio;
            int i0 = static_cast<int>(srcIndex);
            int i1 = juce::jmin(i0 + 1, buffer.getNumSamples() - 1);
            float f = static_cast<float>(srcIndex - i0);
            float s0 = src[i0];
            float s1 = src[i1];
            dst[i] = linearToQuantized(s0 + f * (s1 - s0));
        }
    }
    buffer.makeCopyOf(resampled);
}

void CrunchEngine::process(juce::AudioBuffer<float>& buffer, double sourceSampleRate)
{
    if (sourceSampleRate <= 0.0)
        return;
    resampleTo22k(buffer, sourceSampleRate);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* p = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            p[i] = linearToQuantized(p[i]);
    }
}

void CrunchEngine::processMono(juce::AudioBuffer<float>& buffer, double sourceSampleRate)
{
    if (buffer.getNumChannels() < 1)
        return;
    resampleTo22k(buffer, sourceSampleRate);
    float* p = buffer.getWritePointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        p[i] = linearToQuantized(p[i]);
}

} // namespace bitlink
