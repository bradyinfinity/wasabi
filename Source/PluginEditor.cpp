/**
 * @file PluginEditor.cpp
 * @brief Implementation of the WasabiAudioProcessorEditor and its UI components.
 *
 * This file defines the GUI logic for the Wasabi plugin, including the layout
 * of sliders, buttons, XY pad, and preset selection, with a wasabi-themed design.
 *
 * @license MIT License (see LICENSE file)
 */

#include "PluginEditor.h"
#include "BinaryData.h"

 //==============================================================================
WasabiLookAndFeel::WasabiLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colours::limegreen);
    setColour(juce::Slider::trackColourId, juce::Colour(30, 30, 30));
    setColour(juce::Slider::backgroundColourId, juce::Colour(50, 50, 50));
    setColour(juce::TextButton::buttonColourId, juce::Colour(30, 30, 30));
    setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(30, 30, 30));
    setColour(juce::ComboBox::outlineColourId, juce::Colours::limegreen);
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
}

void WasabiLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(width), static_cast<float>(height)).reduced(4.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = startAngle + sliderPos * (endAngle - startAngle);

    g.setColour(findColour(juce::Slider::backgroundColourId));
    g.fillEllipse(bounds);

    g.setColour(findColour(juce::Slider::trackColourId));
    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius, radius, 0.0f, startAngle, angle, true);
    g.strokePath(arc, juce::PathStrokeType(4.0f));

    g.setColour(findColour(juce::Slider::thumbColourId));
    auto thumbSize = 8.0f;
    g.fillEllipse(centreX + radius * std::cos(angle - juce::MathConstants<float>::halfPi) - thumbSize * 0.5f,
        centreY + radius * std::sin(angle - juce::MathConstants<float>::halfPi) - thumbSize * 0.5f,
        thumbSize, thumbSize);
}

void WasabiLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& /*backgroundColour*/,
    bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(shouldDrawButtonAsDown ? findColour(juce::TextButton::buttonOnColourId)
        : findColour(juce::TextButton::buttonColourId));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colours::limegreen);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void WasabiLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
    bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown)
{
    g.setFont(juce::Font("Consolas", 14.0f, juce::Font::plain));
    g.setColour(shouldDrawButtonAsDown ? juce::Colours::black : juce::Colours::white);
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

void WasabiLookAndFeel::drawComboBox(juce::Graphics& g, int /*width*/, int /*height*/, bool /*isButtonDown*/,
    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& box)
{
    auto bounds = box.getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    g.setColour(findColour(juce::ComboBox::textColourId));
    g.setFont(juce::Font("Consolas", 14.0f, juce::Font::plain));
    g.drawText(box.getText(), bounds.reduced(10, 0), juce::Justification::centredLeft);
    auto arrowZone = bounds.removeFromRight(20.0f);
    g.drawArrow(arrowZone.withTrimmedTop(5.0f).withTrimmedBottom(5.0f),
        0.0f, juce::Colours::limegreen, 1.0f);
}

//==============================================================================
WasabiAudioProcessorEditor::WasabiAudioProcessorEditor(WasabiAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(600, 350);
    setOpaque(true);
    setBufferedToImage(true);
    setLookAndFeel(&lookAndFeel);

    startTimer(100);
}

WasabiAudioProcessorEditor::~WasabiAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void WasabiAudioProcessorEditor::initializeComponents()
{
    if (componentsInitialized)
        return;

    // Define slider configurations
    struct SliderInfo {
        juce::String id;
        juce::String name;
        juce::String label;
    };
    SliderInfo infos[] = {
        {"drive", "Drive", "Drive"},
        {"range", "Range", "Range"},
        {"blend", "Blend", "Blend"},
        {"volume", "Volume", "Volume"},
        {"highPassFreq", "HPF", "Hi-Pass"},
        {"lowPassFreq", "LPF", "Lo-Pass"}
    };

    // Initialize sliders and labels
    for (const auto& info : infos)
    {
        auto slider = std::make_unique<juce::Slider>(info.name);
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        slider->setTooltip(info.label);
        if (info.id == "highPassFreq")
            slider->setRange(50.0f, 500.0f, 1.0f);
        else if (info.id == "lowPassFreq")
            slider->setRange(2000.0f, 12000.0f, 1.0f);
        else if (info.id == "range")
            slider->setRange(0.0f, 5.0f, 0.01f);
        else if (info.id == "drive")
            slider->setRange(0.0f, 2.0f, 0.01f);
        else if (info.id == "volume")
            slider->setRange(0.0f, 2.0f, 0.01f);
        else
            slider->setRange(0.0f, 1.0f, 0.01f);
        slider->setVisible(true);
        addAndMakeVisible(slider.get());
        sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, info.id, *slider));
        sliders.push_back(std::move(slider));

        auto label = std::make_unique<juce::Label>(info.name + "Label", info.label);
        label->setFont(juce::Font("Consolas", 14.0f, juce::Font::plain));
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setJustificationType(juce::Justification::centred);
        label->setMinimumHorizontalScale(1.0f);
        addAndMakeVisible(label.get());
        sliderLabels.push_back(std::move(label));
    }

    // Initialize XY pad
    xyPad = std::make_unique<XYPad>(audioProcessor.parameters);
    addAndMakeVisible(xyPad.get());

    // Initialize katakana distortion buttons
    waButton.setImages(false, true, true,
        juce::ImageCache::getFromMemory(BinaryData::wa_png, BinaryData::wa_pngSize), 1.0f, juce::Colours::transparentBlack,
        juce::ImageCache::getFromMemory(BinaryData::wa_png, BinaryData::wa_pngSize), 1.0f, juce::Colour(30, 30, 30).withAlpha(0.5f),
        juce::ImageCache::getFromMemory(BinaryData::wa_png, BinaryData::wa_pngSize), 1.0f, juce::Colours::limegreen);
    waButton.setClickingTogglesState(true);
    waButton.setRadioGroupId(1);
    waButton.setTooltip("Smooth, warm distortion");
    waButton.onClick = [this]() { updateDistortionType(0); };
    addAndMakeVisible(&waButton);

    saButton.setImages(false, true, true,
        juce::ImageCache::getFromMemory(BinaryData::sa_png, BinaryData::sa_pngSize), 1.0f, juce::Colours::transparentBlack,
        juce::ImageCache::getFromMemory(BinaryData::sa_png, BinaryData::sa_pngSize), 1.0f, juce::Colour(30, 30, 30).withAlpha(0.5f),
        juce::ImageCache::getFromMemory(BinaryData::sa_png, BinaryData::sa_pngSize), 1.0f, juce::Colours::limegreen);
    saButton.setClickingTogglesState(true);
    saButton.setRadioGroupId(1);
    saButton.setTooltip("Sharp, aggressive distortion");
    saButton.onClick = [this]() { updateDistortionType(1); };
    addAndMakeVisible(&saButton);

    biButton.setImages(false, true, true,
        juce::ImageCache::getFromMemory(BinaryData::bi_png, BinaryData::bi_pngSize), 1.0f, juce::Colours::transparentBlack,
        juce::ImageCache::getFromMemory(BinaryData::bi_png, BinaryData::bi_pngSize), 1.0f, juce::Colour(30, 30, 30).withAlpha(0.5f),
        juce::ImageCache::getFromMemory(BinaryData::bi_png, BinaryData::bi_pngSize), 1.0f, juce::Colours::limegreen);
    biButton.setClickingTogglesState(true);
    biButton.setRadioGroupId(1);
    biButton.setTooltip("Complex, folded distortion");
    biButton.onClick = [this]() { updateDistortionType(2); };
    addAndMakeVisible(&biButton);

    // Initialize bypass button
    bypassButton.setButtonText("Bypass");
    bypassButton.setClickingTogglesState(true);
    bypassButton.setTooltip("Toggle effect on/off");
    bypassButton.onClick = [this]() {
        if (auto* param = audioProcessor.parameters.getRawParameterValue("bypass"))
            audioProcessor.parameters.getParameter("bypass")->setValueNotifyingHost(bypassButton.getToggleState() ? 1.0f : 0.0f);
    };
    addAndMakeVisible(&bypassButton);

    // Sync distortion buttons with parameter state
    if (auto* distParam = audioProcessor.parameters.getRawParameterValue("distortionType"))
    {
        int type = juce::roundToInt(*distParam * 2.0f);
        waButton.setToggleState(type == 0, juce::dontSendNotification);
        saButton.setToggleState(type == 1, juce::dontSendNotification);
        biButton.setToggleState(type == 2, juce::dontSendNotification);
    }
    else
    {
        waButton.setToggleState(true, juce::dontSendNotification);
    }

    // Sync bypass button with parameter state
    if (auto* bypassParam = audioProcessor.parameters.getRawParameterValue("bypass"))
        bypassButton.setToggleState(*bypassParam > 0.5f, juce::dontSendNotification);
    else
        bypassButton.setToggleState(false, juce::dontSendNotification);

    // Load and initialize logo
    auto logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    if (logoImage.isValid())
    {
        logoComponent.setImage(logoImage);
        logoComponent.setVisible(true);
    }
    else
    {
        juce::Logger::writeToLog("Failed to load logo.png from BinaryData");
    }
    addAndMakeVisible(&logoComponent);
    logoComponent.toFront(false); // Ensure logo is not obscured

    // Initialize preset dropdown
    for (int i = 0; i < audioProcessor.getNumPrograms(); ++i)
        presetBox.addItem(audioProcessor.getProgramName(i), i + 1);
    presetBox.setSelectedId(audioProcessor.getCurrentProgram() + 1);
    presetBox.setTooltip("Select a preset");
    addAndMakeVisible(&presetBox);
    presetBox.addListener(this);

    componentsInitialized = true;
    resized();
    repaint();
}

void WasabiAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        return;

    // Draw gradient background
    juce::ColourGradient bg(juce::Colour(30, 40, 20), 0.0f, 0.0f,
        juce::Colour(151, 166, 67), 0.0f, static_cast<float>(bounds.getHeight()), false);
    g.setGradientFill(bg);
    g.fillAll();

    // Draw "WASABI" text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Consolas", 36.0f, juce::Font::bold));
    g.drawText("WASABI", 60, 10, 200, 40, juce::Justification::left);
}

void WasabiAudioProcessorEditor::resized()
{
    if (!componentsInitialized)
        return;

    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        return;

    // Layout constants
    const int knobSize = 50;
    const int spacing = 20;
    const int buttonSize = 60;
    const int padding = 10;

    // Position logo
    logoComponent.setBounds(265, 10, 100, 40);
    logoComponent.setTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi, 265 + 50, 10 + 20));

    // Position preset and bypass controls
    presetBox.setBounds(460, 10, 120, 30);
    bypassButton.setBounds(380, 10, 70, 30);

    // Position sliders and labels
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        size_t row = i / 3;
        size_t col = i % 3;
        int x = padding + 40 + static_cast<int>(col) * (knobSize + spacing);
        int y = 70 + static_cast<int>(row) * (knobSize + 50);
        if (sliders[i])
            sliders[i]->setBounds(x, y + 15, knobSize, knobSize + 20);
        if (sliderLabels[i])
            sliderLabels[i]->setBounds(x - 10, y - 10, knobSize + 40, 25);
    }

    // Position XY pad
    if (xyPad)
        xyPad->setBounds(340, 90, 175, 100);

    // Position katakana buttons
    int totalWidth = 3 * buttonSize + 2 * 5;
    int offset = (bounds.getWidth() - totalWidth) / 2;
    waButton.setBounds(offset, 280, buttonSize, buttonSize);
    saButton.setBounds(offset + buttonSize + 5, 280, buttonSize, buttonSize);
    biButton.setBounds(offset + 2 * buttonSize + 10, 280, buttonSize, buttonSize);

    repaint();
}

void WasabiAudioProcessorEditor::timerCallback()
{
    if (!componentsInitialized)
    {
        initializeComponents();
        stopTimer();
    }
}

void WasabiAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetBox)
    {
        audioProcessor.setCurrentProgram(comboBoxThatHasChanged->getSelectedId() - 1);
        presetBox.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);

        // Sync distortion buttons
        if (auto* distParam = audioProcessor.parameters.getRawParameterValue("distortionType"))
        {
            int type = juce::roundToInt(*distParam * 2.0f);
            waButton.setToggleState(type == 0, juce::dontSendNotification);
            saButton.setToggleState(type == 1, juce::dontSendNotification);
            biButton.setToggleState(type == 2, juce::dontSendNotification);
        }

        // Sync bypass button
        if (auto* bypassParam = audioProcessor.parameters.getRawParameterValue("bypass"))
            bypassButton.setToggleState(*bypassParam > 0.5f, juce::dontSendNotification);

        // Update XY pad
        if (xyPad)
        {
            if (auto* freq = audioProcessor.parameters.getRawParameterValue("midFreq"))
                if (auto* gain = audioProcessor.parameters.getRawParameterValue("midGain"))
                    xyPad->updateSliders(*freq, *gain);
        }
    }
}

void WasabiAudioProcessorEditor::updateDistortionType(int type)
{
    if (componentsInitialized)
    {
        if (auto* param = audioProcessor.parameters.getRawParameterValue("distortionType"))
        {
            audioProcessor.parameters.getParameter("distortionType")->setValueNotifyingHost(type / 2.0f);
            waButton.setToggleState(type == 0, juce::dontSendNotification);
            saButton.setToggleState(type == 1, juce::dontSendNotification);
            biButton.setToggleState(type == 2, juce::dontSendNotification);
        }
    }
}

WasabiAudioProcessorEditor::XYPad::XYPad(juce::AudioProcessorValueTreeState& p)
    : parameters(p)
{
    midFreqSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    midFreqSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p, "midFreq", midFreqSlider);
    midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p, "midGain", midGainSlider);
    addAndMakeVisible(midFreqSlider);
    addAndMakeVisible(midGainSlider);
}

void WasabiAudioProcessorEditor::XYPad::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        return;
    g.fillAll(juce::Colour(30, 30, 30));
    g.setColour(juce::Colours::limegreen);
    g.drawRect(bounds, 1);
    float x = static_cast<float>(midFreqSlider.getValue() - midFreqSlider.getMinimum()) /
        static_cast<float>(midFreqSlider.getMaximum() - midFreqSlider.getMinimum()) * (bounds.getWidth() - 10.0f) + 5.0f;
    float y = static_cast<float>(midGainSlider.getMaximum() - midGainSlider.getValue()) /
        static_cast<float>(midGainSlider.getMaximum() - midGainSlider.getMinimum()) * (bounds.getHeight() - 10.0f) + 5.0f;
    g.fillEllipse(x - 5.0f, y - 5.0f, 10.0f, 10.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Consolas", 12.0f, juce::Font::plain));
    g.drawText("Mid Freq", 5, bounds.getHeight() - 15, 60, 10, juce::Justification::left);
    g.drawText("Mid Gain", bounds.getWidth() - 65, 5, 60, 10, juce::Justification::right);
}

void WasabiAudioProcessorEditor::XYPad::resized()
{
    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        return;
    midFreqSlider.setBounds(bounds);
    midGainSlider.setBounds(bounds);
}

void WasabiAudioProcessorEditor::XYPad::mouseDown(const juce::MouseEvent& e)
{
    updatePosition(e);
}

void WasabiAudioProcessorEditor::XYPad::mouseDrag(const juce::MouseEvent& e)
{
    updatePosition(e);
}

void WasabiAudioProcessorEditor::XYPad::updateSliders(float freq, float gain)
{
    midFreqSlider.setValue(freq, juce::sendNotification);
    midGainSlider.setValue(gain, juce::sendNotification);
    repaint();
}

void WasabiAudioProcessorEditor::XYPad::updatePosition(const juce::MouseEvent& e)
{
    auto bounds = getLocalBounds().reduced(5);
    if (bounds.isEmpty())
        return;
    float x = juce::jlimit(0.0f, 1.0f, static_cast<float>(e.x - bounds.getX()) / static_cast<float>(bounds.getWidth()));
    float y = juce::jlimit(0.0f, 1.0f, static_cast<float>(bounds.getHeight() - (e.y - bounds.getY())) / static_cast<float>(bounds.getHeight()));
    float freq = static_cast<float>(midFreqSlider.getMinimum()) + x * static_cast<float>(midFreqSlider.getMaximum() - midFreqSlider.getMinimum());
    float gain = static_cast<float>(midGainSlider.getMinimum()) + y * static_cast<float>(midGainSlider.getMaximum() - midGainSlider.getMinimum());
    midFreqSlider.setValue(freq, juce::sendNotification);
    midGainSlider.setValue(gain, juce::sendNotification);
    repaint();
}