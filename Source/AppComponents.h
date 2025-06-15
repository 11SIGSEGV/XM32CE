/*
  ==============================================================================

    AppComponents.h
    Created: 28 May 2025 3:53:04pm
    Author:  anony

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "OSCMan.h"
#include "Helpers.h"


// TODO: Test if setting non-default minDeg and maxDeg works as expected... I'm scared.
struct EncoderRotary : public Slider {
    EncoderRotary(Units unit,
                  const double minDeg = -135.0, const double minValue = 0.0,
                  const double maxDeg = 135.0, const double maxValue = 1.0,
                  const double middlePV = 0.5,
                  const double defaultPV = 0.0,
                  const String &minLabel = "",
                  const String &maxLabel = "",
                  const int overrideRoundingToXDecimalPlaces = -1,
                  const ParamType paramType = ParamType::LINF,
                  const bool middleProvidedAsPercentage = true,
                  const bool defaultProvidedAsPercentage = true
    );

    // We'll also allow Enum and OptionParams
    EncoderRotary(const OptionParam &option,
                  const double minDeg = -135.0,
                  const double maxDeg = 135.0,
                  const int defaultIndex = 0,
                  const String &minLabel = "",
                  const String &maxLabel = ""
    );

    EncoderRotary(const EnumParam &enumParam,
                  const double minDeg = -135.0,
                  const double maxDeg = 135.0,
                  const int defaultIndex = 0,
                  const String &minLabel = "",
                  const String &maxLabel = ""
    );

    void resized() override;

    void paint(Graphics &) override;

private:
    const bool isOptionParam;
    const bool isEnumParam;
    OptionParam option; // Only used if isOptionParam is true
    EnumParam enumParam; // Only used if isEnumParam is true
    Rectangle<int> Bounds;
    double middlePos;
    double middleValue; // Middle value is automatically generated from min and max value

    const double minPos;
    const String minLabel;
    const double minValue;
    const double maxPos;
    const String maxLabel;
    const double maxValue;

    double defaultPos;
    double defaultValue;

    const ParamType paramType;

    Image predrawnLabels;
    float encoderRadius;
    Rectangle<int> boundsCopy;
    float textHeight;
};


struct Encoder : public Component, public Slider::Listener, public TextEditor::Listener {
    /* Initalise UIRotary.
     * When middle/defaultProvidedAsPercentage is true, the middle and default values are
     * automatically interpolated based on the middlePV/defaultPV percentage and the minValue and maxValue.
     * Otherwise, the middlePV and defaultPV are used as the actual values.
     */
    Encoder(Units unit,
            const double minDeg = -135.0, const double minValue = 0.0,
            const double maxDeg = 135.0, const double maxValue = 1.0,
            const double middlePV = 0.5,
            const double defaultPV = 0.0,
            const String &minLabel = "",
            const String &maxLabel = "",
            const int overrideRoundingToXDecimalPlaces = -1,
            const ParamType paramType = ParamType::LINF,
            const bool middleProvidedAsPercentage = true,
            const bool defaultProvidedAsPercentage = true
    );

    Encoder(const OptionParam &option,
                  const double minDeg = -135.0,
                  const double maxDeg = 135.0,
                  const int defaultIndex = 0,
                  const String &minLabel = "",
                  const String &maxLabel = "");

    Encoder(const EnumParam &enumParam,
                  const double minDeg = -135.0,
                  const double maxDeg = 135.0,
                  const int defaultIndex = 0,
                  const String &minLabel = "",
                  const String &maxLabel = "");

    void init() {
        manualInputBox.setText(getValueAsDisplayString());
        manualInputBox.addListener(this);
        manualInputBox.setJustification(Justification::centred);
        encoder.addListener(this);
        addAndMakeVisible(encoder);
        addAndMakeVisible(manualInputBox);

    }

    ~Encoder() override {
        encoder.removeListener(this);
        manualInputBox.removeListener(this);
        setLookAndFeel(nullptr);
        deleteAllChildren();
    }


    const String getValueAsDisplayString() {
        // Value should be index when isOptionParam or isEnumParam, and a numerical value otherwise.
        auto value = encoder.getValue();

        if (_isOptionParam) {
            return String(_option.value.at(static_cast<int>(value)));
        } else if (_isEnumParam) {
            return String(_enumParam.value.at(static_cast<int>(value)));
        } else {
            switch (unit) {
                case HERTZ: {
                    if (value < 1000.0) {
                        return String(value, roundTo) + " Hz";
                    }
                    return String(value / 1000.0, roundTo) + " kHz";
                }
                case DB: {
                    if (value <= -90.0) {
                        return "-inf";
                    }
                    return ((value > 0) ? "+" + String(value, roundTo) : String(value, roundTo)) + " dB";
                }
                case NONE: default:
                    return String(value, roundTo);
            }
        }
    }


    void sliderValueChanged(Slider *) override {
        // When value changed, we'll update the manual input box
        manualInputBox.setText(getValueAsDisplayString());
    }

    void textEditorReturnKeyPressed(TextEditor &editor) override {
        // Do checks to see if the value is valid
        auto [valid, value] = getDoubleValueFromTextEditor(editor.getText());
        if (valid) {
            std::cout << value << std::endl;
            // If valid, set the value to the encoder
            // Check if the value is within the range of the encoder
            if (value < minValue || value > maxValue) {
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                                 "Invalid Input",
                                                 "Value is out of range. Please enter a value between "
                                                 + String(minValue) + " and " + String(maxValue) + ".");
                return;
            }
            // Set the rounded value.
            encoder.setValue(std::round(value * _roundingMultiplier) / _roundingMultiplier);
        } else {
            // If not valid, show an error message
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                             "Invalid Input",
                                             "Please enter a valid value.");
        }
    }

    // void sliderDragStarted(Slider *) override {};
    // void sliderDragEnded(Slider *) override {};


    void paint(Graphics &g) override;

    void resized() override;

private:
    EncoderRotary encoder;

    TextEditor manualInputBox{"manualInputBox"};

    Rectangle<int> Bounds;
    const bool _isOptionParam;
    const bool _isEnumParam;
    OptionParam _option; // Only used if isOptionParam is true
    EnumParam _enumParam; // Only used if isEnumParam is true

    double middlePos;
    double middleValue; // Middle value is automatically generated from min and max value

    const double minPos;
    const String minLabel;
    const double minValue;
    const double maxPos;
    const String maxLabel;
    const double maxValue;

    double defaultPos;
    double defaultValue;

    const Units unit;
    const int roundTo;
    const double _roundingMultiplier;
    const ParamType paramType;
};
