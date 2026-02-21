#include "VampireSlicer.h"
#include <cmath>
#include <algorithm>

namespace bitlink {

void VampireSlicer::addSlice(int sampleIndex)
{
    auto it = std::lower_bound(slicePoints.begin(), slicePoints.end(), sampleIndex);
    if (it == slicePoints.end() || *it != sampleIndex)
        slicePoints.insert(it, sampleIndex);
}

void VampireSlicer::removeSliceNear(int sampleIndex)
{
    if (slicePoints.empty())
        return;
    auto it = std::lower_bound(slicePoints.begin(), slicePoints.end(), sampleIndex);
    if (it != slicePoints.end())
    {
        int idx = static_cast<int>(it - slicePoints.begin());
        if (idx > 0 && std::abs(slicePoints[idx - 1] - sampleIndex) < std::abs(slicePoints[idx] - sampleIndex))
            --idx;
        else if (idx == (int)slicePoints.size() - 1 && idx > 0 && std::abs(slicePoints[idx] - sampleIndex) > std::abs(slicePoints[idx - 1] - sampleIndex))
            --idx;
        slicePoints.erase(slicePoints.begin() + idx);
    }
    else
        slicePoints.pop_back();
}

int VampireSlicer::getSliceStart(int index) const
{
    if (index < 0 || index >= (int)slicePoints.size())
        return 0;
    return slicePoints[index];
}

int VampireSlicer::getSliceEnd(int index) const
{
    if (index < 0 || index >= (int)slicePoints.size())
        return 0;
    if (index + 1 < (int)slicePoints.size())
        return slicePoints[index + 1];
    return slicePoints[index] + kMinSliceLength;
}

void VampireSlicer::analyse(const juce::AudioBuffer<float>& buffer, float sensitivity)
{
    slicePoints.clear();
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples < kMinSliceLength * 2 || numChannels == 0)
        return;

    juce::HeapBlock<float> rms(numSamples);
    const int winSize = juce::jmin(256, numSamples / 4);
    const int halfWin = winSize / 2;

    for (int i = 0; i < numSamples; ++i)
    {
        float sum = 0.0f;
        int start = juce::jmax(0, i - halfWin);
        int end = juce::jmin(numSamples, i + halfWin);
        int n = 0;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* p = buffer.getReadPointer(ch);
            for (int j = start; j < end; ++j, ++n)
                sum += p[j] * p[j];
        }
        rms[i] = n > 0 ? std::sqrt(sum / (float)n) : 0.0f;
    }

    float threshold = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        threshold += rms[i];
    threshold /= numSamples;
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        peak = juce::jmax(peak, rms[i]);
    threshold += (peak - threshold) * (0.2f + 0.6f * (1.0f - sensitivity));

    slicePoints.push_back(0);
    bool above = rms[0] > threshold;
    for (int i = 1; i < numSamples - kMinSliceLength; ++i)
    {
        bool nowAbove = rms[i] > threshold;
        if (!above && nowAbove)
        {
            int last = slicePoints.empty() ? 0 : slicePoints.back();
            if (i - last >= kMinSliceLength)
                slicePoints.push_back(i);
        }
        above = nowAbove;
    }
}

} // namespace bitlink
