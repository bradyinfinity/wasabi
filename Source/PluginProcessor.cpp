/**
 * @file PluginProcessor.cpp
 * @brief Implementation of the WasabiAudioProcessor class for audio processing.
 *
 * This file contains the core audio processing logic for the Wasabi distortion plugin,
 * including distortion algorithms, filter processing, and preset management.
 *
 * @license MIT License (see LICENSE file)
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

 // Parameter IDs
namespace ParamIDs {
    static const juce::String drive = "drive";
    static const juce::String range = "range";
    static const juce::String blend = "blend";
    static const juce::String volume = "volume";
    static const juce::String midFreq = "midFreq";
    static const juce::String midGain = "midGain";
    static const juce::String highPassFreq = "highPassFreq";
    static const juce::String lowPassFreq = "lowPassFreq";
    static const juce::String distortionType = "distortionType";
    static const juce::String bypass = "bypass";
}

WasabiAudioProcessor::WasabiAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    parameters(*this, nullptr, juce::Identifier("WasabiParameters"), createParameterLayout())
{
}

WasabiAudioProcessor::~WasabiAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout WasabiAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::drive, "Drive", 0.0f, 2.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::range, "Range", 0.0f, 5.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::blend, "Blend", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::volume, "Volume", 0.0f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::midFreq, "Mid Frequency", 500.0f, 2000.0f, 1000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::midGain, "Mid Gain", 0.0f, 12.0f, 6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::highPassFreq, "High Pass Freq", 50.0f, 500.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::lowPassFreq, "Low Pass Freq", 2000.0f, 12000.0f, 6000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::distortionType, "Distortion Type", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::bypass, "Bypass", false));
    return layout;
}

int WasabiAudioProcessor::getNumPrograms()
{
    return 5;
}

int WasabiAudioProcessor::getCurrentProgram()
{
    return parameters.state.getProperty("currentProgram", 0);
}

void WasabiAudioProcessor::setCurrentProgram(int index)
{
    juce::ValueTree state = parameters.state;
    state.setProperty("currentProgram", juce::jlimit(0, getNumPrograms() - 1, index), nullptr);

    auto setParamValue = [&](const juce::String& id, float value) {
        if (auto* param = parameters.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(value));
    };

    switch (index)
    {
    case 0: // Wasabi Warfare (Balanced Distortion)
        setParamValue(ParamIDs::drive, 1.0f);
        setParamValue(ParamIDs::range, 2.0f);
        setParamValue(ParamIDs::blend, 0.9f);
        setParamValue(ParamIDs::volume, 1.0f);
        setParamValue(ParamIDs::midFreq, 1000.0f);
        setParamValue(ParamIDs::midGain, 6.0f);
        setParamValue(ParamIDs::highPassFreq, 100.0f);
        setParamValue(ParamIDs::lowPassFreq, 6000.0f);
        setParamValue(ParamIDs::distortionType, 0.0f);
        break;
    case 1: // Up Your Nose (Heavy Crunch)
        setParamValue(ParamIDs::drive, 1.5f);
        setParamValue(ParamIDs::range, 3.0f);
        setParamValue(ParamIDs::blend, 0.95f);
        setParamValue(ParamIDs::volume, 1.2f);
        setParamValue(ParamIDs::midFreq, 800.0f);
        setParamValue(ParamIDs::midGain, 8.0f);
        setParamValue(ParamIDs::highPassFreq, 150.0f);
        setParamValue(ParamIDs::lowPassFreq, 5000.0f);
        setParamValue(ParamIDs::distortionType, 0.5f);
        break;
    case 2: // Sushi Roll (Lead Tone)
        setParamValue(ParamIDs::drive, 1.2f);
        setParamValue(ParamIDs::range, 2.5f);
        setParamValue(ParamIDs::blend, 0.9f);
        setParamValue(ParamIDs::volume, 1.3f);
        setParamValue(ParamIDs::midFreq, 1200.0f);
        setParamValue(ParamIDs::midGain, 7.0f);
        setParamValue(ParamIDs::highPassFreq, 120.0f);
        setParamValue(ParamIDs::lowPassFreq, 7000.0f);
        setParamValue(ParamIDs::distortionType, 1.0f);
        break;
    case 3: // Soy Sauce (High-Gain Metal)
        setParamValue(ParamIDs::drive, 1.8f);
        setParamValue(ParamIDs::range, 4.0f);
        setParamValue(ParamIDs::blend, 1.0f);
        setParamValue(ParamIDs::volume, 1.0f);
        setParamValue(ParamIDs::midFreq, 900.0f);
        setParamValue(ParamIDs::midGain, 9.0f);
        setParamValue(ParamIDs::highPassFreq, 200.0f);
        setParamValue(ParamIDs::lowPassFreq, 4500.0f);
        setParamValue(ParamIDs::distortionType, 0.5f);
        break;
    case 4: // Soba (Vintage Crunch)
        setParamValue(ParamIDs::drive, 0.8f);
        setParamValue(ParamIDs::range, 1.5f);
        setParamValue(ParamIDs::blend, 0.85f);
        setParamValue(ParamIDs::volume, 1.1f);
        setParamValue(ParamIDs::midFreq, 1100.0f);
        setParamValue(ParamIDs::midGain, 5.0f);
        setParamValue(ParamIDs::highPassFreq, 80.0f);
        setParamValue(ParamIDs::lowPassFreq, 8000.0f);
        setParamValue(ParamIDs::distortionType, 0.0f);
        break;
    }
}

const juce::String WasabiAudioProcessor::getProgramName(int index)
{
    static const juce::String names[] = { "Wasabi Warfare", "Up Your Nose", "Sushi Roll", "Soy Sauce", "Soba" };
    return (index >= 0 && index < getNumPrograms()) ? names[index] : juce::String();
}

void WasabiAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
    // Preset names are fixed; modification not supported.
}

void WasabiAudioProcessor::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = static_cast<float>(newSampleRate);
    juce::dsp::ProcessSpec spec{ newSampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(getTotalNumOutputChannels()) };

    highPassFilter.prepare(spec);
    lowPassFilter.prepare(spec);
    midBoostFilter.prepare(spec);
    oversampler.initProcessing(samplesPerBlock);
}

void WasabiAudioProcessor::releaseResources()
{
    oversampler.reset();
}

bool WasabiAudioProcessor::isBusesLayoutSupported(const BusesLayout & layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}

void WasabiAudioProcessor::processBlock(juce::AudioBuffer<float> & buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();

    // Bypass processing if enabled
    if (*parameters.getRawParameterValue(ParamIDs::bypass) > 0.5f)
    {
        return;
    }

    // Clear unused output channels
    for (auto i = numInputChannels; i < numOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Retrieve parameter values
    float drive = *parameters.getRawParameterValue(ParamIDs::drive);
    float range = *parameters.getRawParameterValue(ParamIDs::range);
    float blend = *parameters.getRawParameterValue(ParamIDs::blend);
    float volume = *parameters.getRawParameterValue(ParamIDs::volume);
    float midFreq = *parameters.getRawParameterValue(ParamIDs::midFreq);
    float midGain = *parameters.getRawParameterValue(ParamIDs::midGain);
    float highPassFreq = *parameters.getRawParameterValue(ParamIDs::highPassFreq);
    float lowPassFreq = *parameters.getRawParameterValue(ParamIDs::lowPassFreq);
    int distortionType = static_cast<int>(*parameters.getRawParameterValue(ParamIDs::distortionType) * 2);

    // Update filter coefficients
    *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, highPassFreq);
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lowPassFreq, 0.7f);
    *midBoostFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, midFreq, 1.0f, juce::Decibels::decibelsToGain(midGain));

    // Process audio with oversampling
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler.processSamplesUp(buffer);
    for (int channel = 0; channel < numInputChannels; ++channel)
    {
        auto* data = oversampledBlock.getChannelPointer(channel);
        juce::dsp::ProcessContextReplacing<float> context(oversampledBlock);

        // Apply high-pass and mid-boost filters
        highPassFilter.process(context);
        midBoostFilter.process(context);

        // Apply distortion and noise gate
        for (int sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
        {
            float clean = data[sample];

            // Noise gate (-40 dB threshold)
            float threshold = 0.01f;
            float gate = std::abs(clean) < threshold ? 0.1f : 1.0f;
            clean *= gate;

            float preGain = range * 5.0f;
            float distorted = clean * preGain;

            switch (distortionType)
            {
            case 0: // Smooth Tanh (Wa)
                distorted = std::tanh(distorted * (1.0f + drive)) * 0.9f;
                break;
            case 1: // Hard Clipping (Sa)
                distorted = std::tanh(distorted * 0.6f) * 1.8f;
                distorted = juce::jlimit(-0.9f, 0.9f, distorted * (1.0f + drive * 2.0f));
                break;
            case 2: // Wavefolding (Bi)
                float folded = distorted - 0.2f * distorted * distorted * distorted;
                distorted = std::sin(folded * juce::MathConstants<float>::pi * (0.5f + drive * 0.5f));
                distorted = juce::jlimit(-0.9f, 0.9f, distorted);
                break;
            }

            distorted *= (0.5f + volume * 1.5f);
            data[sample] = (distorted * blend + clean * (1.0f - blend)) * gate;
        }

        // Apply low-pass filter
        lowPassFilter.process(context);
    }

    // Downsample back to original rate
    juce::dsp::AudioBlock<float> outputBlock(buffer);
    oversampler.processSamplesDown(outputBlock);
}

juce::AudioProcessorEditor* WasabiAudioProcessor::createEditor()
{
    return new WasabiAudioProcessorEditor(*this);
}

void WasabiAudioProcessor::getStateInformation(juce::MemoryBlock & destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WasabiAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WasabiAudioProcessor();
}