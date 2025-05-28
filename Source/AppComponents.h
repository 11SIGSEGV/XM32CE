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


struct EncoderRotary : public Slider {
    EncoderRotary(Units unit,
                  const double minDeg = 225.0, const double minValue = 0.0,
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


    void paint(Graphics &) override;

private:
    Rectangle<int> Bounds;
    const double middlePerc;
    double middlePos;
    double middleValue; // Middle value is automatically generated from min and max value

    const double minPos;
    const String minLabel;
    const double minValue;
    const double maxPos;
    const String maxLabel;
    const double maxValue;

    const double defaultPerc;
    double defaultPos;
    double defaultValue;

    const ParamType paramType;
};



struct Encoder: public Component, public Slider::Listener, public TextEditor::Listener {
    /* Initalise UIRotary.
     * When middle/defaultProvidedAsPercentage is true, the middle and default values are
     * automatically interpolated based on the middlePV/defaultPV percentage and the minValue and maxValue.
     * Otherwise, the middlePV and defaultPV are used as the actual values.
     */
    Encoder(Units unit,
        const double minDeg = 225.0, const double minValue = 0.0,
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


    const String getValueAsDisplayString() {
        auto value = encoder.getValue();
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
                return ((value > 0) ? "+" + String(value, roundTo): String(value, roundTo)) + " dB";
            }
            case NONE: default:
                return String(value, roundTo);
        }
    }


    void sliderValueChanged(Slider *) override {
        // When value changed, we'll update the manual input box
        manualInputBox.setText(getValueAsDisplayString());
        DBG("HI!"); // For debugging purposes, remove in production
    }

    void textEditorReturnKeyPressed(TextEditor &editor) override {
        // Do checks to see if the value is valid
        if (&editor == &manualInputBox) {
            // If the value is valid, set the value of the slider
            auto value = manualInputBox.getText().getDoubleValue();
            if (value < minValue || value > maxValue) {
                // Nothing happens. Invalid value.
                return;
            }
            encoder.setValue(value);
        } else {
            jassertfalse; // This should never happen
        }
    }

    // void sliderDragStarted(Slider *) override {};
    // void sliderDragEnded(Slider *) override {};

    bool waitUntilResizeReady() {
        // Wait until resized() is called before painting
        for (int i; i < 100 && !resizeReady.get(); i++) {
            Thread::sleep(10); // Sleep for 10ms to avoid busy waiting
        }
        if (resizeReady.get()) {
            return true; // Resize is ready
        }
        return false;
    }

    void paint(Graphics &g) override;
    void resized() override;


private:
    EncoderRotary encoder;
    // Save raw arguments from constructor


    TextEditor manualInputBox { "manualInputBox" };

    Atomic<bool> resizeReady { false }; // To ensure resized() is called before paint()
    Rectangle<int> Bounds;
    const double middlePerc;
    double middlePos;
    double middleValue; // Middle value is automatically generated from min and max value

    const double minPos;
    const String minLabel;
    const double minValue;
    const double maxPos;
    const String maxLabel;
    const double maxValue;

    const double defaultPerc;
    double defaultPos;
    double defaultValue;

    const Units unit;
    const int roundTo;
    const ParamType paramType;
};
