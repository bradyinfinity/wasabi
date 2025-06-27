#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 * @class WasabiLookAndFeel
 * @brief Custom LookAndFeel for the Wasabi plugin's GUI.
 *
 * Defines the visual style for sliders, buttons, and combo boxes, using a lime-green
 * and dark theme inspired by wasabi aesthetics.
 */
class WasabiLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WasabiLookAndFeel();

    /// Draws a rotary slider with a gradient fill and lime-green outline.
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        float startAngle, float endAngle, juce::Slider&) override;

    /// Draws button backgrounds with rounded rectangles and toggle state colors.
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
        bool, bool) override;

    /// Draws button text with Consolas font, adjusting color based on toggle state.
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override;

    /// Draws a combo box with a dark background, lime-green outline, and arrow.
    void drawComboBox(juce::Graphics& g, int, int, bool, int, int, int, int, juce::ComboBox& box) override;
};

/**
 * @class WasabiAudioProcessorEditor
 * @brief Editor (GUI) for the Wasabi distortion plugin.
 *
 * Manages the user interface, including sliders, buttons, XY pad, and presets,
 * with a custom layout and wasabi-themed visuals.
 */
class WasabiAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer,
    public juce::ComboBox::Listener
{
public:
    //==============================================================================
    /// Constructor. Initializes the editor with the processor reference.
    WasabiAudioProcessorEditor(WasabiAudioProcessor&);

    /// Destructor. Cleans up resources.
    ~WasabiAudioProcessorEditor() override;

    //==============================================================================
    /// Paints the background and "WASABI" text.
    void paint(juce::Graphics&) override;

    /// Positions all UI components (sliders, buttons, XY pad, etc.).
    void resized() override;

private:
    //==============================================================================
    /// Timer callback to initialize components after construction.
    void timerCallback() override;

    /// Handles preset selection changes from the combo box.
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /// Updates the distortion type based on button selection.
    void updateDistortionType(int type);

    /// Initializes sliders, buttons, and other UI components.
    void initializeComponents();

    //==============================================================================
    /**
     * @class XYPad
     * @brief Custom component for controlling mid frequency and gain via a 2D pad.
     */
    class XYPad : public juce::Component
    {
    public:
        XYPad(juce::AudioProcessorValueTreeState& p);
        void paint(juce::Graphics&) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;

        /// Updates the pad's position based on frequency and gain values.
        void updateSliders(float freq, float gain);

    private:
        /// Updates slider values based on mouse position.
        void updatePosition(const juce::MouseEvent& e);

        juce::Slider midFreqSlider, midGainSlider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midFreqAttachment, midGainAttachment;
        juce::AudioProcessorValueTreeState& parameters;
    };

    //==============================================================================
    std::vector<std::unique_ptr<juce::Slider>> sliders; ///< Rotary sliders for parameters.
    std::vector<std::unique_ptr<juce::Label>> sliderLabels; ///< Labels for sliders.
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments; ///< Parameter attachments for sliders.
    juce::ImageButton waButton, saButton, biButton; ///< Katakana buttons for distortion types.
    juce::TextButton bypassButton; ///< Toggle button for bypassing the effect.
    juce::ComboBox presetBox; ///< Dropdown for selecting presets.
    juce::ImageComponent logoComponent; ///< Displays the Wasabi logo.
    std::unique_ptr<XYPad> xyPad; ///< Interactive XY pad for mid EQ control.
    WasabiLookAndFeel lookAndFeel; ///< Custom look and feel for the UI.
    WasabiAudioProcessor& audioProcessor; ///< Reference to the processor.
    bool componentsInitialized = false; ///< Flag to track component initialization.

    // Prevent copying and detect memory leaks.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WasabiAudioProcessorEditor)
};