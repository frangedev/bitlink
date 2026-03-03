#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "IFF8SVXExport.h"
#include "SampleTools.h"

BitLinkProcessor::BitLinkProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BitLinkState", createParameterLayout())
{
    formatManager.registerBasicFormats();
}

BitLinkProcessor::~BitLinkProcessor() {}

const juce::String BitLinkProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BitLinkProcessor::acceptsMidi() const { return true; }
bool BitLinkProcessor::producesMidi() const { return false; }
bool BitLinkProcessor::isMidiEffect() const { return false; }
double BitLinkProcessor::getTailLengthSeconds() const { return 0.0; }

int BitLinkProcessor::getNumPrograms() { return 1; }
int BitLinkProcessor::getCurrentProgram() { return 0; }
void BitLinkProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String BitLinkProcessor::getProgramName(int index) { juce::ignoreUnused(index); return "Default"; }
void BitLinkProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

juce::AudioProcessorValueTreeState::ParameterLayout BitLinkProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterChoice>("crunchBits", "Crunch Bit Depth", juce::StringArray("8-bit", "12-bit"), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("stretch", "Stretch", juce::NormalisableRange<float>(0.25f, 4.0f, 0.01f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("bpm", "BPM", juce::NormalisableRange<float>(40.0f, 300.0f, 1.0f), 120.0f));
    return layout;
}

void BitLinkProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    cyclicStretch.reset();
}

void BitLinkProcessor::releaseResources() {}

bool BitLinkProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

void BitLinkProcessor::loadSampleFromFile(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader)
        return;
    loadedSampleRate = reader->sampleRate;
    sampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&sampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    applyCrunchToLoadedSample();
    slicer.clear();
    runAutoSlice(0.5f);
    cyclicStretch.reset();
}

void BitLinkProcessor::loadSampleFromMemory(const void* data, size_t sizeBytes)
{
    auto stream = std::make_unique<juce::MemoryInputStream>(static_cast<const char*>(data), sizeBytes, false);
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(std::move(stream)));
    if (!reader)
        return;
    loadedSampleRate = reader->sampleRate;
    sampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&sampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    applyCrunchToLoadedSample();
    slicer.clear();
    runAutoSlice(0.5f);
    cyclicStretch.reset();
}

void BitLinkProcessor::setSampleBufferFromPreset(const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    sampleBuffer.makeCopyOf(buffer);
    loadedSampleRate = sampleRate;
    cyclicStretch.reset();
}

void BitLinkProcessor::loadSampleFromFileRaw(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader)
        return;
    loadedSampleRate = reader->sampleRate;
    sampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&sampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    slicer.clear();
    cyclicStretch.reset();
}

void BitLinkProcessor::setPreviewPlayback(int startSample, bool active)
{
    if (active && startSample >= 0)
    {
        previewStartSample = startSample;
        previewReadPos = startSample;
        previewActive = true;
    }
    else
    {
        previewActive = false;
        previewReadPos = -1;
    }
}

int BitLinkProcessor::getPlaybackPositionSample() const
{
    if (previewActive.load() && hasSample())
    {
        int pos = previewReadPos.load();
        if (pos >= 0)
            return pos;
        return previewStartSample.load();
    }
    if (stretchEnabled && hasSample())
        return cyclicStretch.getReadPositionSamples();
    return -1;
}

void BitLinkProcessor::setBPM(float bpm)
{
    if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("bpm")))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(juce::jlimit(40.0f, 300.0f, bpm)));
}

float BitLinkProcessor::getBPM() const
{
    if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("bpm")))
        return p->get();
    return 120.0f;
}

void BitLinkProcessor::applyCrunchToLoadedSample()
{
    if (sampleBuffer.getNumSamples() == 0)
        return;
    crunchEngine.processMono(sampleBuffer, loadedSampleRate);
    loadedSampleRate = bitlink::CrunchEngine::kCrunchSampleRate;
}

void BitLinkProcessor::setCrunchBitDepth(bitlink::CrunchEngine::BitDepth d)
{
    crunchEngine.setBitDepth(d);
    if (hasSample())
        applyCrunchToLoadedSample();
}

bitlink::CrunchEngine::BitDepth BitLinkProcessor::getCrunchBitDepth() const
{
    return crunchEngine.getBitDepth();
}

void BitLinkProcessor::setStretchFactor(float f)
{
    cyclicStretch.setStretchFactor(f);
}

float BitLinkProcessor::getStretchFactor() const
{
    return cyclicStretch.getStretchFactor();
}

void BitLinkProcessor::runAutoSlice(float sensitivity)
{
    if (sampleBuffer.getNumSamples() > 0)
        slicer.analyse(sampleBuffer, sensitivity);
}

void BitLinkProcessor::addSliceAt(int sampleIndex)
{
    slicer.addSlice(sampleIndex);
}

void BitLinkProcessor::removeSliceNear(int sampleIndex)
{
    slicer.removeSliceNear(sampleIndex);
}

int BitLinkProcessor::getSliceForMidiNote(int note) const
{
    int index = note - baseMidiNote;
    if (index < 0 || index >= slicer.getNumSlices())
        return -1;
    return index;
}

void BitLinkProcessor::clearSample()
{
    sampleBuffer.setSize(0, 0);
    slicer.clear();
    cyclicStretch.reset();
}

void BitLinkProcessor::reverseSample()
{
    if (hasSample())
        bitlink::SampleTools::reverse(sampleBuffer);
}

float BitLinkProcessor::normalizeSample(float targetLevel)
{
    if (!hasSample())
        return 0.0f;
    return bitlink::SampleTools::normalize(sampleBuffer, targetLevel);
}

void BitLinkProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1)
        return;

    const float* sampleData = hasSample() ? sampleBuffer.getReadPointer(0) : nullptr;
    const int sampleLength = hasSample() ? sampleBuffer.getNumSamples() : 0;
    const float gain = sampleGain;

    if (previewActive.load() && sampleData != nullptr && sampleLength > 0)
    {
        int pos = previewReadPos.load();
        if (pos < 0)
            pos = previewStartSample.load();
        for (int i = 0; i < numSamples && pos < sampleLength; ++i, ++pos)
        {
            float s = sampleData[pos] * gain;
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.addSample(ch, i, s);
        }
        if (pos >= sampleLength)
            previewActive = false;
        else
            previewReadPos = pos;
        return;
    }

    if (!hasSample())
        return;

    if (stretchEnabled)
    {
        float* outL = buffer.getWritePointer(0);
        float* outR = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
        cyclicStretch.process(sampleData, sampleLength, outL, numSamples, loadedSampleRate);
        if (outR)
            for (int i = 0; i < numSamples; ++i)
                outR[i] = outL[i];
        buffer.applyGain(gain);
        return;
    }

    for (const auto midiMetadata : midiMessages)
    {
        const auto msg = midiMetadata.getMessage();
        const int sampleOffset = midiMetadata.samplePosition;
        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            int sliceIdx = getSliceForMidiNote(note);
            if (sliceIdx >= 0)
            {
                int start = slicer.getSliceStart(sliceIdx);
                int end = (sliceIdx + 1) < slicer.getNumSlices()
                              ? slicer.getSliceEnd(sliceIdx)
                              : sampleLength;
                end = juce::jmin(end, sampleLength);
                if (slicerThruMode)
                {
                    for (int i = start; i < end && (i - start) < numSamples - sampleOffset; ++i)
                    {
                        float s = sampleData[i] * gain;
                        for (int ch = 0; ch < numChannels; ++ch)
                            buffer.addSample(ch, sampleOffset + (i - start), s);
                    }
                }
                else
                {
                    for (int i = 0; i < numSamples - sampleOffset && i < (end - start); ++i)
                    {
                        float s = sampleData[juce::jmin(start + i, sampleLength - 1)] * gain;
                        for (int ch = 0; ch < numChannels; ++ch)
                            buffer.addSample(ch, sampleOffset + i, s);
                    }
                }
            }
            else
            {
                if (!slicerThruMode && sampleOffset < numSamples)
                {
                    for (int i = 0; i < numSamples - sampleOffset && i < sampleLength; ++i)
                    {
                        float s = sampleData[i] * gain;
                        for (int ch = 0; ch < numChannels; ++ch)
                            buffer.addSample(ch, sampleOffset + i, s);
                    }
                }
            }
        }
    }
}

// Required for plugin host
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitLinkProcessor();
}

bool BitLinkProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* BitLinkProcessor::createEditor()
{
    return new BitLinkEditor(*this);
}

void BitLinkProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BitLinkProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
