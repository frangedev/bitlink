#include "PresetFormat.h"
#include "PluginProcessor.h"
#include "WAVExport.h"
#include <cstring>

namespace bitlink {

static const uint32_t kPresetMagic = 0x424C5052;
static const uint32_t kPresetVersion = 1;

static bool writeSlicesFile(const juce::File& baseFile, const std::vector<int>& points)
{
    juce::File f = baseFile.getSiblingFile(baseFile.getFileNameWithoutExtension() + PresetFormat::slicesExtension());
    juce::FileOutputStream os(f);
    if (!os.openedOk())
        return false;
    for (int p : points)
        os.writeText(juce::String(p) + "\n", false, false, nullptr);
    return true;
}

static bool readSlicesFile(const juce::File& baseFile, std::vector<int>& points)
{
    juce::File f = baseFile.getSiblingFile(baseFile.getFileNameWithoutExtension() + PresetFormat::slicesExtension());
    if (!f.existsAsFile())
        return true;
    juce::String content = f.loadFileAsString();
    points.clear();
    for (auto line : juce::StringArray::fromLines(content))
    {
        int v = line.trim().getIntValue();
        if (line.trim().isNotEmpty())
            points.push_back(v);
    }
    return true;
}

bool PresetFormat::savePreset(const juce::File& file, BitLinkProcessor& processor)
{
    if (!file.getParentDirectory().exists())
        file.getParentDirectory().createDirectory();

    juce::MemoryOutputStream out;
    out.writeInt(static_cast<int>(kPresetMagic));
    out.writeInt(static_cast<int>(kPresetVersion));

    juce::MemoryBlock stateBlock;
    processor.getStateInformation(stateBlock);
    out.writeInt(static_cast<int>(stateBlock.getSize()));
    out.write(stateBlock.getData(), stateBlock.getSize());

    if (!processor.hasSample())
    {
        out.writeInt(0);
    }
    else
    {
        const auto& buf = processor.getSampleBuffer();
        int numSamples = buf.getNumSamples();
        int numChannels = buf.getNumChannels();
        double rate = processor.getSampleRate();
        out.writeInt(numSamples);
        out.writeInt(numChannels);
        out.writeDouble(rate);
        for (int ch = 0; ch < numChannels; ++ch)
            out.write(buf.getReadPointer(ch), sizeof(float) * static_cast<size_t>(numSamples));
    }

    const auto& slicer = processor.getSlicer();
    const auto& points = slicer.getSlicePoints();
    out.writeInt(static_cast<int>(points.size()));
    for (int p : points)
        out.writeInt(p);

    out.writeFloat(processor.getSampleGain());
    out.writeInt(processor.getBaseMidiNote());
    out.writeBool(processor.isStretchEnabled());
    out.writeBool(processor.isSlicerThruMode());
    out.writeInt(processor.getCrunchBitDepth() == bitlink::CrunchEngine::BitDepth::Bits12 ? 1 : 0);
    out.writeFloat(processor.getStretchFactor());

    return file.replaceWithData(out.getData(), out.getDataSize());
}

bool PresetFormat::loadPreset(const juce::File& file, BitLinkProcessor& processor)
{
    juce::FileInputStream in(file);
    if (!in.openedOk())
        return false;
    if (in.readInt() != static_cast<int>(kPresetMagic) || in.readInt() != static_cast<int>(kPresetVersion))
        return false;

    int stateLen = in.readInt();
    if (stateLen < 0 || stateLen > 1024 * 1024 * 10)
        return false;
    juce::MemoryBlock stateBlock;
    stateBlock.setSize(static_cast<size_t>(stateLen));
    if (in.read(stateBlock.getData(), stateLen) != static_cast<size_t>(stateLen))
        return false;
    processor.setStateInformation(stateBlock.getData(), stateLen);

    int numSamples = in.readInt();
    if (numSamples > 0)
    {
        int numChannels = in.readInt();
        double rate = in.readDouble();
        juce::AudioBuffer<float> buf(numChannels, numSamples);
        for (int ch = 0; ch < numChannels; ++ch)
            if (in.read(buf.getWritePointer(ch), sizeof(float) * static_cast<size_t>(numSamples)) != sizeof(float) * static_cast<size_t>(numSamples))
                return false;
        processor.setSampleBufferFromPreset(buf, rate);
    }
    else
        processor.clearSample();

    int numSlices = in.readInt();
    std::vector<int> points;
    for (int i = 0; i < numSlices; ++i)
        points.push_back(in.readInt());
    processor.getSlicer().setSlicePoints(points);

    processor.setSampleGain(in.readFloat());
    processor.setBaseMidiNote(in.readInt());
    processor.setStretchEnabled(in.readBool());
    processor.setSlicerThruMode(in.readBool());
    int bits12 = in.readInt();
    processor.setCrunchBitDepth(bits12 ? bitlink::CrunchEngine::BitDepth::Bits12 : bitlink::CrunchEngine::BitDepth::Bits8);
    processor.setStretchFactor(in.readFloat());

    return true;
}

bool PresetFormat::saveSound(const juce::File& wavFile, BitLinkProcessor& processor)
{
    if (!processor.hasSample())
        return false;
    if (!WAVExport::writeToFile(wavFile, processor.getSampleBuffer(), processor.getSampleRate(), 24))
        return false;
    return writeSlicesFile(wavFile, processor.getSlicer().getSlicePoints());
}

bool PresetFormat::loadSound(const juce::File& wavFile, BitLinkProcessor& processor)
{
    if (!wavFile.existsAsFile())
        return false;
    processor.loadSampleFromFileRaw(wavFile);
    std::vector<int> points;
    if (readSlicesFile(wavFile, points))
        processor.getSlicer().setSlicePoints(points);
    return true;
}

} // namespace bitlink
