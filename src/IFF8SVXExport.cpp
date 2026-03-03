#include "IFF8SVXExport.h"
#include <cstring>

namespace
{
void writeU32BE(uint8_t* dest, uint32_t v)
{
    dest[0] = static_cast<uint8_t>((v >> 24) & 0xff);
    dest[1] = static_cast<uint8_t>((v >> 16) & 0xff);
    dest[2] = static_cast<uint8_t>((v >> 8) & 0xff);
    dest[3] = static_cast<uint8_t>(v & 0xff);
}
}

namespace bitlink {

static void writeU32BEToStream(juce::MemoryOutputStream& out, uint32_t v)
{
    out.writeByte(static_cast<juce::juce_wchar>((v >> 24) & 0xff));
    out.writeByte(static_cast<juce::juce_wchar>((v >> 16) & 0xff));
    out.writeByte(static_cast<juce::juce_wchar>((v >> 8) & 0xff));
    out.writeByte(static_cast<juce::juce_wchar>(v & 0xff));
}

static void writeChunk(juce::MemoryOutputStream& out, const char* id, const void* data, size_t size)
{
    out.write(id, 4);
    writeU32BEToStream(out, static_cast<uint32_t>(size));
    if (data && size > 0)
        out.write(data, size);
    if (size & 1)
        out.writeByte(0);
}

bool IFF8SVXExport::writeToFile(const juce::File& file,
                                const float* samples,
                                int numSamples,
                                double sampleRate,
                                const juce::String& name)
{
    if (!file.getParentDirectory().exists())
        file.getParentDirectory().createDirectory();
    if (!samples || numSamples <= 0)
        return false;

    juce::MemoryOutputStream formStream;
    formStream.write("FORM", 4);
    uint32_t formSizePos = static_cast<uint32_t>(formStream.getDataSize());
    writeU32BEToStream(formStream, 0);
    formStream.write("8SVX", 4);

    uint32_t oneShot = static_cast<uint32_t>(numSamples);
    uint32_t rate = static_cast<uint32_t>(juce::jmin(65535.0, sampleRate));
    uint32_t volumeFixed = 0x10000;
    uint8_t vhdr[20] = {
        static_cast<uint8_t>((oneShot >> 24) & 0xff),
        static_cast<uint8_t>((oneShot >> 16) & 0xff),
        static_cast<uint8_t>((oneShot >> 8) & 0xff),
        static_cast<uint8_t>(oneShot & 0xff),
        0, 0, 0, 0,
        0, 0, 0, 0,
        static_cast<uint8_t>((rate >> 8) & 0xff),
        static_cast<uint8_t>(rate & 0xff),
        1,
        0,
        static_cast<uint8_t>((volumeFixed >> 24) & 0xff),
        static_cast<uint8_t>((volumeFixed >> 16) & 0xff),
        static_cast<uint8_t>((volumeFixed >> 8) & 0xff),
        static_cast<uint8_t>(volumeFixed & 0xff)
    };
    writeChunk(formStream, "VHDR", vhdr, 20);

    if (name.isNotEmpty())
    {
        juce::MemoryOutputStream nameStream;
        nameStream.writeString(name);
        size_t nameLen = nameStream.getDataSize();
        writeChunk(formStream, "NAME", nameStream.getData(), nameLen);
    }

    juce::HeapBlock<uint8_t> body(static_cast<size_t>(numSamples));
    for (int i = 0; i < numSamples; ++i)
    {
        float s = juce::jlimit(-1.0f, 1.0f, samples[i]);
        int8_t v = static_cast<int8_t>(static_cast<int>(s * 127.0f));
        body[i] = static_cast<uint8_t>(v);
    }
    writeChunk(formStream, "BODY", body.getData(), static_cast<size_t>(numSamples));

    uint32_t total = static_cast<uint32_t>(formStream.getDataSize()) - 8;
    size_t totalSize = formStream.getDataSize();
    juce::HeapBlock<char> buf(totalSize);
    std::memcpy(buf.getData(), formStream.getData(), totalSize);
    writeU32BE(reinterpret_cast<uint8_t*>(buf.getData()) + formSizePos, total);

    return file.replaceWithData(buf.getData(), totalSize);
}

} // namespace bitlink
