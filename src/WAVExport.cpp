#include "WAVExport.h"

namespace bitlink {

bool WAVExport::writeToFile(const juce::File& file,
                             const juce::AudioBuffer<float>& buffer,
                             double sampleRate,
                             int numBitsPerSample)
{
    if (!file.getParentDirectory().exists())
        file.getParentDirectory().createDirectory();
    if (buffer.getNumSamples() == 0)
        return false;

    juce::WavAudioFormat format;
    std::unique_ptr<juce::OutputStream> stream(new juce::FileOutputStream(file));
    auto opts = juce::AudioFormatWriterOptions{}
                    .withSampleRate(sampleRate)
                    .withBitsPerSample(numBitsPerSample)
                    .withChannelLayout(buffer.getNumChannels() == 1
                                          ? juce::AudioChannelSet::mono()
                                          : juce::AudioChannelSet::stereo());
    auto writer = format.createWriterFor(stream, opts);
    if (!writer)
        return false;
    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    return true;
}

} // namespace bitlink
