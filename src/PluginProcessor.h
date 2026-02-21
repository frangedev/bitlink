#pragma once

#include <JuceHeader.h>
#include "CrunchEngine.h"
#include "VampireSlicer.h"
#include "CyclicStretch.h"

class BitLinkProcessor : public juce::AudioProcessor
{
public:
    BitLinkProcessor();
    ~BitLinkProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // BitLink API for editor
    void loadSampleFromFile(const juce::File& file);
    void loadSampleFromMemory(const void* data, size_t sizeBytes);
    /** Load pre-crushed buffer (e.g. from preset). Does not apply crunch or auto-slice. */
    void setSampleBufferFromPreset(const juce::AudioBuffer<float>& buffer, double sampleRate);
    /** Load WAV/AIFF without applying crunch (for loading saved "sound"). */
    void loadSampleFromFileRaw(const juce::File& file);
    const juce::AudioBuffer<float>& getSampleBuffer() const { return sampleBuffer; }
    int getSampleLength() const { return sampleBuffer.getNumSamples(); }
    double getSampleRate() const { return loadedSampleRate; }
    bool hasSample() const { return sampleBuffer.getNumSamples() > 0; }

    void setCrunchBitDepth(bitlink::CrunchEngine::BitDepth d);
    bitlink::CrunchEngine::BitDepth getCrunchBitDepth() const;
    void setStretchFactor(float f);
    float getStretchFactor() const;
    void setStretchEnabled(bool e) { stretchEnabled = e; }
    bool isStretchEnabled() const { return stretchEnabled; }
    void setSlicerThruMode(bool t) { slicerThruMode = t; }
    bool isSlicerThruMode() const { return slicerThruMode; }

    void runAutoSlice(float sensitivity = 0.5f);
    void addSliceAt(int sampleIndex);
    void removeSliceNear(int sampleIndex);
    const bitlink::VampireSlicer& getSlicer() const { return slicer; }
    bitlink::VampireSlicer& getSlicer() { return slicer; }

    int getSliceForMidiNote(int note) const;
    void setBaseMidiNote(int note) { baseMidiNote = note; }
    int getBaseMidiNote() const { return baseMidiNote; }

    void clearSample();
    void reverseSample();
    float normalizeSample(float targetLevel = 0.95f);
    void setSampleGain(float gain) { sampleGain = juce::jlimit(0.0f, 4.0f, gain); }
    float getSampleGain() const { return sampleGain; }

    /** Preview playback from waveform click. startSample < 0 or active false = stop. */
    void setPreviewPlayback(int startSample, bool active);
    bool isPreviewActive() const { return previewActive.load(); }

    /** Current playback position in sample index (for playhead), or -1 if idle. */
    int getPlaybackPositionSample() const;

    void setBPM(float bpm);
    float getBPM() const;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::AudioBuffer<float> sampleBuffer;
    double loadedSampleRate = 44100.0;
    double crunchSampleRate = 22050.0;
    int baseMidiNote = 60;

    bitlink::CrunchEngine crunchEngine;
    bitlink::VampireSlicer slicer;
    bitlink::CyclicStretch cyclicStretch;

    bool stretchEnabled = false;
    bool slicerThruMode = false;

    std::atomic<float> playPos{ 0.0f };
    int currentSliceIndex = -1;
    bool slicePlayback = false;
    float sampleGain = 1.0f;

    std::atomic<int> previewStartSample{ -1 };
    std::atomic<int> previewReadPos{ -1 };
    std::atomic<bool> previewActive{ false };

    juce::AudioFormatManager formatManager;
    juce::AudioProcessorValueTreeState apvts;

    void applyCrunchToLoadedSample();
    void renderSlicePlayback(int sliceIndex, juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void renderStretchedPlayback(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitLinkProcessor)
};
