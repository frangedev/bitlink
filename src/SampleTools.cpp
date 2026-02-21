#include "SampleTools.h"
#include <algorithm>
#include <cmath>

namespace bitlink {

void SampleTools::reverse(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* p = buffer.getWritePointer(ch);
        std::reverse(p, p + numSamples);
    }
}

float SampleTools::normalize(juce::AudioBuffer<float>& buffer, float targetLevel)
{
    float peak = getPeakLevel(buffer);
    if (peak <= 0.0f || targetLevel <= 0.0f)
        return peak;
    float gain = targetLevel / peak;
    applyGain(buffer, gain);
    return peak;
}

void SampleTools::applyGain(juce::AudioBuffer<float>& buffer, float gain)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    for (int ch = 0; ch < numChannels; ++ch)
        buffer.applyGain(ch, 0, numSamples, gain);
}

float SampleTools::getPeakLevel(const juce::AudioBuffer<float>& buffer)
{
    float peak = 0.0f;
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* p = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            peak = juce::jmax(peak, std::abs(p[i]));
    }
    return peak;
}

int SampleTools::trimSilence(juce::AudioBuffer<float>& buffer, float threshold)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples == 0)
        return 0;

    int start = 0;
    for (int i = 0; i < numSamples; ++i)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            peak = juce::jmax(peak, std::abs(buffer.getSample(ch, i)));
        if (peak > threshold)
        {
            start = i;
            break;
        }
    }

    int end = numSamples;
    for (int i = numSamples - 1; i >= start; --i)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            peak = juce::jmax(peak, std::abs(buffer.getSample(ch, i)));
        if (peak > threshold)
        {
            end = i + 1;
            break;
        }
    }

    if (start > 0 || end < numSamples)
    {
        const int newLength = end - start;
        juce::AudioBuffer<float> trimmed(numChannels, newLength);
        for (int ch = 0; ch < numChannels; ++ch)
            trimmed.copyFrom(ch, 0, buffer, ch, start, newLength);
        buffer.makeCopyOf(trimmed);
    }
    return start;
}

} // namespace bitlink
