/*
  ==============================================================================

    AppComponents.h
    Created: 28 May 2025 3:53:04pm
    Author:  anony

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Helpers.h"


// TODO: Test if setting non-default minDeg and maxDeg works as expected... I'm scared.
struct EncoderRotary : public Slider {
    explicit EncoderRotary(Units unit,
                  double minDeg = -135.0, double minValue = 0.0,
                  double maxDeg = 135.0, double maxValue = 1.0,
                  double middlePV = 0.5,
                  double defaultPV = 0.0,
                  const String &minLabel = "",
                  const String &maxLabel = "",
                  int overrideRoundingToXDecimalPlaces = -1,
                  ParamType paramType = ParamType::LINF,
                  bool middleProvidedAsPercentage = true,
                  bool defaultProvidedAsPercentage = true
    );

    // We'll also allow Enum and OptionParams
    explicit EncoderRotary(const OptionParam &option,
                           double minDeg = -135.0,
                           double maxDeg = 135.0,
                           int defaultIndex = 0,
                           const String &minLabel = "",
                            const String &maxLabel = ""
    );

    explicit EncoderRotary(const EnumParam &enumParam,
                           double minDeg = -135.0,
                           double maxDeg = 135.0,
                           int defaultIndex = 0,
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
    explicit Encoder(Units unit,
                     double minDeg = -135.0, double minValue = 0.0,
                     double maxDeg = 135.0, double maxValue = 1.0,
                     double middlePV = 0.5,
                     double defaultPV = 0.0,
                     const String &minLabel = "",
                     const String &maxLabel = "",
                     int overrideRoundingToXDecimalPlaces = -1,
                     ParamType paramType = LINF,
                     bool middleProvidedAsPercentage = true,
                     bool defaultProvidedAsPercentage = true
    );

    explicit Encoder(const OptionParam &option,
                     double minDeg = -135.0,
                     double maxDeg = 135.0,
                     int defaultIndex = 0,
                     const String &minLabel = "",
                     const String &maxLabel = "");

    explicit Encoder(const EnumParam &enumParam,
                     double minDeg = -135.0,
                     double maxDeg = 135.0,
                     int defaultIndex = 0,
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


    String getValueAsDisplayString() const {
        // Value should be index when isOptionParam or isEnumParam, and a numerical value otherwise.
        auto value = encoder.getValue();

        if (_isOptionParam) {
            return _option.value.at(static_cast<int>(value));
        } if (_isEnumParam) {
            return _enumParam.value.at(static_cast<int>(value));
        }
        switch (unit) {
            case HERTZ: {
                return value < 1000.0 ?
                    String(value, roundTo) + " Hz" :
                    String(value / 1000.0, roundTo) + " kHz";
            }
            case DB: {
                if (value <= -90.0) {
                    return "-inf";
                }
                return value <= -90.0 ? "-inf" :
                    (
                        (value > 0) ?
                            "+" + String(value, roundTo) :
                            String(value, roundTo)
                        ) + " dB";
            }
            case NONE: default:
                return String(value, roundTo);
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



class ParentWindowListener {
public:
    enum WindowType {
        AppComponents_OSCActionConstructor,
    };
    virtual ~ParentWindowListener() = default;
    // Please don't pass std::string uuid as reference... this is setting ourselves up for SIGSEGVs!
    virtual void closeRequested(WindowType windowType, std::string uuid) = 0;
};



class OSCActionConstructor : public DocumentWindow
{
public:
    OSCActionConstructor (const std::string& uuid, String name = "Cue OSC Action Constructor") : DocumentWindow (name,
                                         UICfg::BG_COLOUR,
                                         allButtons), uuid(uuid)
    {
        centreWithSize (1000, 800);
        setUsingNativeTitleBar(true);
        setAlwaysOnTop(true);
        setContentOwned (new MainComp(), true);
        setVisible (true);
    }

    void closeButtonPressed() override {
        if (registeredListener != nullptr) {
            registeredListener->closeRequested(ParentWindowListener::AppComponents_OSCActionConstructor, uuid);
        }
    }

    void setParentListener(ParentWindowListener* lstnr) {
        registeredListener = lstnr;
    }

    void removeParentListener() { registeredListener = nullptr; }


    class MainComp: public Component, public ComboBox::Listener {
        public:
            MainComp() {
                setSize (1000, 800);
            }
            ~MainComp() override {};

            void resized() override;

            void paint(Graphics &g) override;

            void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override {};

    private:
        ComboBox templateCategoryDropdown;
        ComboBox templateDropdown;

        Rectangle<int> templateSelectionBox;
        Rectangle<int> pathBox;
        Rectangle<int> argumentsBox;
        Rectangle<int> buttonsBox;
    };
private:
    const std::string uuid;
    ParentWindowListener* registeredListener = nullptr;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCActionConstructor)
};





