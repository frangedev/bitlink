#include "CyclicStretch.h"
#include <cmath>
#include <algorithm>

namespace bitlink {

void CyclicStretch::setStretchFactor(float factor)
{
    stretchFactor = juce::jlimit(0.25f, 4.0f, factor);
}

void CyclicStretch::setGrainSize(int samples)
{
    grainSize = juce::jmax(64, juce::jmin(4096, samples));
}

void CyclicStretch::process(const float* source, int sourceLength,
                            float* dest, int destLength,
                            double sourceSampleRate)
{
    if (sourceLength <= 0 || destLength <= 0 || source == nullptr || dest == nullptr)
        return;

    const double step = (double)sourceLength / (double)destLength;
    const int grain = grainSize > 0 ? grainSize : 512;
    juce::ignoreUnused(sourceSampleRate);

    for (int i = 0; i < destLength; ++i)
    {
        double pos = readPos + i * step;
        int i0 = (int)std::floor(pos) % sourceLength;
        if (i0 < 0)
            i0 += sourceLength;
        int i1 = (i0 + 1) % sourceLength;
        float frac = static_cast<float>(pos - std::floor(pos));
        float s0 = source[i0];
        float s1 = source[i1];
        float v = s0 + frac * (s1 - s0);
        int grainPhase = i0 % grain;
        float window = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * (float)grainPhase / (float)grain);
        dest[i] = v * window;
    }
    readPos += destLength * step;
    if (readPos >= sourceLength)
        readPos = std::fmod(readPos, (double)sourceLength);
}

} // namespace bitlink
