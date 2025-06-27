#pragma once

#include <JuceHeader.h>

/**
 * @class WasabiAudioProcessor
 * @brief Core audio processing class for the Wasabi distortion plugin.
 *
 * Handles audio processing, parameter management, and preset functionality
 * for a stereo distortion effect with multiple distortion types and filters.
 */
class WasabiAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    /// Constructor. Initializes the processor with default settings.
    WasabiAudioProcessor();

    /// Destructor. Cleans up resources.
    ~WasabiAudioProcessor() override;

    //==============================================================================
    /// Prepares the processor for playback, initializing filters and oversampler.
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /// Releases allocated resources.
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    /// Checks if the bus layout (e.g., stereo or mono) is supported.
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    /// Processes an audio block, applying distortion, filters, and effects.
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    /// Creates the plugin's editor (GUI).
    juce::AudioProcessorEditor* createEditor() override;

    /// Returns true as the plugin has a custom editor.
    bool hasEditor() const override { return true; }

    /// Returns the plugin's name ("Wasabi").
    const juce::String getName() const override { return JucePlugin_Name; }

    //==============================================================================
    /// Indicates that the plugin accepts MIDI input.
    bool acceptsMidi() const override { return true; }

    /// Indicates that the plugin does not produce MIDI output.
    bool producesMidi() const override { return false; }

    /// Indicates that this is not a MIDI effect plugin.
    bool isMidiEffect() const override { return false; }

    /// Returns the tail length (0.0 as no tail is produced).
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    /// Returns the number of preset programs (5).
    int getNumPrograms() override;

    /// Gets the index of the current preset.
    int getCurrentProgram() override;

    /// Sets the current preset by index.
    void setCurrentProgram(int index) override;

    /// Gets the name of a preset by index.
    const juce::String getProgramName(int index) override;

    /// Not implemented; preset names are fixed.
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    /// Saves the plugin's state (parameters and preset) to a memory block.
    void getStateInformation(juce::MemoryBlock& destData) override;

    /// Restores the plugin's state from a memory block.
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    /// Manages plugin parameters (e.g., drive, range, blend, filters).
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    /// Creates the parameter layout for the plugin.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /// High-pass filter for removing low frequencies.
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPassFilter;

    /// Low-pass filter for removing high frequencies.
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowPassFilter;

    /// Mid-band boost filter for EQ shaping.
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> midBoostFilter;

    /// Oversampler to reduce aliasing in distortion processing.
    juce::dsp::Oversampling<float> oversampler{ 2, 1, juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR };

    /// Current sample rate, used for filter calculations.
    float sampleRate = 44100.0f;

    // Prevent copying and detect memory leaks.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WasabiAudioProcessor)
};