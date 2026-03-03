#pragma once

#include <JuceHeader.h>
#include <vector>

namespace bitlink {

/** Transient detection and slice points for Vampire Slicer. */
class VampireSlicer
{
public:
    VampireSlicer() = default;

    /** Analyse buffer and fill slicePoints (sample indices). Sensitivity 0..1. */
    void analyse(const juce::AudioBuffer<float>& buffer, float sensitivity = 0.5f);

    /** Get slice count. */
    int getNumSlices() const { return static_cast<int>(slicePoints.size()); }

    /** Get slice start sample index. Index 0..getNumSlices()-1. */
    int getSliceStart(int index) const;

    /** Get slice end sample index (exclusive). */
    int getSliceEnd(int index) const;

    /** Clear slices. */
    void clear() { slicePoints.clear(); }

    /** Add a manual slice at sample index. */
    void addSlice(int sampleIndex);

    /** Remove slice that contains this sample. */
    void removeSliceNear(int sampleIndex);

    const std::vector<int>& getSlicePoints() const { return slicePoints; }
    void setSlicePoints(std::vector<int> points) { slicePoints = std::move(points); }

private:
    std::vector<int> slicePoints;
    static constexpr int kMinSliceLength = 64;
};

} // namespace bitlink
