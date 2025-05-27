#pragma once
#include <JuceHeader.h>
#include "OSCMan.h"
#include "Helpers.h"


namespace UICfg {
    const String DEFAULT_SANS_SERIF_FONT_NAME = Font::getDefaultSansSerifFontName();
    const String DEFAULT_SERIF_FONT_NAME = Font::getDefaultSerifFontName();
    const String DEFAULT_MONOSPACE_FONT_NAME = Font::getDefaultMonospacedFontName();
    constexpr int DEFAULT_TEXT_HEIGHT = 100;
    const Font DEFAULT_FONT = FontOptions(DEFAULT_SANS_SERIF_FONT_NAME, static_cast<float>(DEFAULT_TEXT_HEIGHT), Font::plain);
    const Font DEFAULT_MONOSPACE_FONT = FontOptions(DEFAULT_MONOSPACE_FONT_NAME, static_cast<float>(DEFAULT_TEXT_HEIGHT), Font::plain);

    const Colour BG_COLOUR (34, 34, 34);
    const Colour TEXT_COLOUR (238, 238, 238);
    const Colour TEXT_COLOUR_DARK (100, 100, 100);

    const Colour POSITIVE_BUTTON_COLOUR (89, 177, 128);
    const Colour POSITIVE_OVER_BUTTON_COLOUR (102, 208, 149);
    const Colour POSITIVE_DOWN_BUTTON_COLOUR (102, 208, 149);
    const Colour NEGATIVE_BUTTON_COLOUR (226, 67, 67);
    const Colour NEGATIVE_OVER_BUTTON_COLOUR (254, 38, 38);
    const Colour NEGATIVE_DOWN_BUTTON_COLOUR (132, 10, 10);

    constexpr float ROTARY_POINTER_WIDTH = 0.05f; // X% of half the bounding width (i.e., x% of the radius)
    const Colour ROTARY_POINTER_COLOUR (28U, 21u, 11u); // Darker variant of the background colour
    constexpr float ROTARY_TEXT_PADDING = 1.4f; // X% of half bounding width (i.e., x% of the radius). Should be >1.f.
    constexpr float ROTARY_TEXT_HEIGHT = 0.25f; // X% of half bounding width (i.e., x% of the radius)
    const Colour ROTARY_ENABLED_COLOUR (242u, 194u, 63u);
    const Colour ROTARY_DISABLED_COLOUR (87u, 76u, 48u);

    constexpr int ROUND_TO_WHEN_IN_DOUBT = 2; // Round to 2 decimal places when in doubt (e.g., no unit for the value)


    // For UI element sizes use relative sizes
    constexpr float STD_PADDING = 1.f/30.f;

    // Look and feel Configs
    const Colour TEXT_EDITOR_BG_COLOUR (0.f, 0.f, 0.f, 0.f);
}


enum Units {
    HERTZ,
    DB,
    NONE
};

const std::unordered_map<Units, int> ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT = {
    {HERTZ, 2},
    {NONE, UICfg::ROUND_TO_WHEN_IN_DOUBT}
};


inline void loadUICfgIntoStdLnF(LookAndFeel_V4 &lnf) {
    lnf.setColour(TextEditor::backgroundColourId, UICfg::TEXT_EDITOR_BG_COLOUR);

}


struct AppLnF: juce::LookAndFeel_V4 {
    void drawRotarySlider(Graphics &g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, Slider &slide) override;
};


/* Function to map a percentage (0-1) to a value between minVal and maxVal using different algorithms
When percentage is 0, it returns minVal.
When percentage is 1, it returns maxVal.
When percentage is between 0 and 1, it uses the specified algorithm to interpolate between minVal and maxVal.
If an error occurs (e.g., percentage is out of range), it asserts false and fallbacks by returning minVal.
*/
inline const double inferValueFromMinMaxAndPercentage(
   double minVal, double maxVal, double percentage, ParamType algorithm = ParamType::LINF) {
    if (percentage == 0.0) {
        // If default is 0, then we can assume that the value is just the min.
        return minVal;
    } if (percentage == 1.0) {
        // If default is 1, then we can assume that the value is just the max.
        return maxVal;
    } if (percentage > 0.0 && percentage < 1.0) {
        switch (algorithm) {
            case ParamType::LINF:
                // Linear interpolation
                return jmap(percentage, minVal, maxVal);
            case ParamType::LOGF:
                // Logarithmic interpolation
                return mapToLog10(percentage, minVal, maxVal);
            case ParamType::LEVEL_161: {
                // Level 161 interpolation - for XM32
                // First, find the level equivalent of the percentage
                if (minVal != -90.0 || maxVal != 10.0) {
                    jassertfalse; // Level 161 is only defined for -90 to 10 dB
                }
                float targetLevel = jmap(percentage, minVal, maxVal);
                // Then, find the closest level 161 value.
                return XM32::roundToNearest(targetLevel, levelValues_161);
            }
            case ParamType::LEVEL_1024: {
                // Level 1024 interpolation - for XM32. Much simpler! This uses Music Tribe's approximation for float to dB log scale
                // NOTE: Here, if minVal and maxVal are not -90 and 10, respectively, it does not matter. It will still assume -90 and 10.
                if (minVal != -90.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                } if (maxVal != 10.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                return XM32::floatToDb(percentage);
            }
            default:
                jassertfalse;
                return minVal; // Fallback to min value if algorithm is not recognised
        }
    }
    jassertfalse; // Value is out of range from 0-1
    return minVal;
}


/* Basically, inverse of inferValueFromMinMaxAndPercentage. (i.e., normalises a value).
 * When value is out of range, fallbacks to returning 0.0 */
inline const double inferPercentageFromMinMaxAndValue(
    double minVal, double maxVal, double value, ParamType algorithm = ParamType::LINF) {
    if (value == minVal) {
        // If default value is the minimum, the normalised percentage is 0.
        return 0.0;
    } if (value == maxVal) {
        // If default value is the maximum, the normalised percentage is 1.
        return 1.0;
    } if (value > minVal && value < maxVal) {
        switch (algorithm) {
            case ParamType::LINF:
                // Linear interpolation
                return (value-minVal)/(maxVal-minVal);
            case ParamType::LOGF:
                // Logarithmic interpolation
                return mapFromLog10(value, minVal, maxVal);
            case ParamType::LEVEL_161: {
                // Level 161 'un'-interpolation - for XM32
                if (minVal != -90.0 || maxVal != 10.0) {
                    jassertfalse; // Level 161 is only defined for -90 to 10 dB
                }
                auto val = levelToFloat_161.find(value);
                if (val == levelToFloat_161.end()) {
                    jassertfalse; // Value is not a valid level 161 value
                    return 0.0; // Fallback to 0.0 if value is not a valid level 161 value
                }
                return val->second;
            }
            case ParamType::LEVEL_1024: {
                // Level 1024 interpolation - for XM32.
                // NOTE: Here, if minVal and maxVal are not -90 and 10, respectively, it does not matter. It will still assume -90 and 10.
                if (minVal != -90.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                } if (maxVal != 10.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                if (value < -90.0 || value > 10.0) {
                    jassertfalse; // Value is out of range for level 1024
                    return 0.0; // Fallback to 0.0 if value is out of range. We don't want to reach XM32::dbToFloat with an out of range value, as it will throw an exception
                }
                return XM32::dbToFloat(value);
            }
            default:
                jassertfalse;
                return 0.0; // Fallback to min value if algorithm is not recognised
        }
    }
    jassertfalse; // Value is out of range from minVal to maxVal
    return 0.0;
}


// Generates a NormalisableRange for a logarithmic slider. From https://forum.juce.com/t/logarithmic-slider-for-frequencies-iir-hpf/37569/10
static inline NormalisableRange<double> getNormalisableRangeExp(double min, double max)
{
    jassert(min > 0.0);
    jassert(max > 0.0);
    jassert(min < max);

    double logmin = std::log(min);
    double logmax = std::log(max);
    double logrange = (logmax - logmin);

    jassert(logrange > 0.0);

    return NormalisableRange<double>(
        min, max,
        [logmin,logrange](double start, double end, double normalized) {
            normalized = std::max(0.0, std::min(1.0, normalized));
            double value = exp((normalized * logrange) + logmin);
            return std::max(start, std::min(end, value));
    },
        [logmin,logrange](double start, double end, double value) {
            value = std::max(start, std::min(end, value));
            double logvalue = std::log(value);
            return (logvalue - logmin) / logrange;
    },
    [](double start, double end, double value) {
        return std::max(start, std::min(end, value));
    });
}

const inline NormalisableRange<double> LEVEL_161_NORMALISABLE_RANGE(
    -90.0, 10.0,
    [](double start, double end, double normalized) {
        normalized = std::max(0.0, std::min(1.0, normalized));  // Ensure normalized is between 0 and 1
        return XM32::roundToNearest(
            static_cast<float>(jmap(normalized, start, end)), levelValues_161);
    },
    [](double start, double end, double value) {
        value = std::max(start, std::min(end, value)); // Ensure value is within the range
        auto it = levelToFloat_161.find(value);
        if (it == levelToFloat_161.end()) {
            // Not valid level 161 value, so we snap it to the nearest level 161 value
            value = XM32::roundToNearest(static_cast<float>(value), levelValues_161);
        }
        return it->second; // Return the corresponding float value
    },
    [](double start, double end, double value) {
        return std::max(start, std::min(end, value));
    });

const inline NormalisableRange<double> LEVEL_1024_NORMALISABLE_RANGE(
    -90.0, 10.0,
    [](double start, double end, double normalized) {
        normalized = std::max(0.0, std::min(1.0, normalized));  // Ensure normalized is between 0 and 1
        return XM32::floatToDb(normalized);
    },
    [](double start, double end, double value) {
        value = std::max(start, std::min(end, value)); // Ensure value is within the range
        return XM32::dbToFloat(value); // Return the corresponding dB value
    },
    [](double start, double end, double value) {
        return std::max(start, std::min(end, value));
    });



struct UIRotary: public Slider {
    /* Initalise UIRotary.
     * When middle/defaultProvidedAsPercentage is true, the middle and default values are
     * automatically interpolated based on the middlePV/defaultPV percentage and the minValue and maxValue.
     * Otherwise, the middlePV and defaultPV are used as the actual values.
     */
    UIRotary(Units unit,
        const double minDeg, const double minValue,
        const double maxDeg, const double maxValue,
        const double middlePV = 0.5,
        const double defaultPV = 0.0,
        const String &minLabel = "",
        const String &maxLabel = "",
        const int overrideRoundingToXDecimalPlaces = -1,
        const ParamType paramType = ParamType::LINF,
        const bool middleProvidedAsPercentage = true,
        const bool defaultProvidedAsPercentage = true
        );
    // Returns vector of {minLabel, maxLabel}
    const std::vector<String> getMinMaxLabel() {
        return {minLabel, maxLabel};
    }

    const String getValueAsDisplayString() {
        auto value = getValue();
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



    void paint(Graphics &g) override;

private:
    const double middlePerc;
    double middlePos;
    double middleValue; // Middle value is automatically generated from min and max value

    const double minLabelPos;
    const String minLabel;
    const double minValue;
    const double maxLabelPos;
    const String maxLabel;
    const double maxValue;

    const double defaultPerc;
    double defaultPos;
    double defaultValue;

    const Units unit;
    const int roundTo;
    const ParamType paramType;
};


class MainComponent  : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (Graphics &g) override;
    void resized() override;

    static void showConnectionErrorMessage (const String& messageText)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
            "Connection error",
            messageText,
            "OK");
    }


private:
    //==============================================================================
    // Your private member variables go here...
    Slider rotaryKnob; // [1]
    OSCSender sender; // [2]
    AppLnF lnf; // [3]
    UIRotary testRotary {
        HERTZ, 45.0, 20.0, 315.0, 20000.0, 0.5, 0.0, "20Hz",
     "20kHz", 2, ParamType::LOGF, true, true};

    UIRotary testRotary2 {
        DB, 45.0, -90.0, 315.0, 10.0, 0.5, 0.0, "-inf",
     "+10.0dB", 2, ParamType::LEVEL_1024, true, true};


    std::vector<Component*> activeComps = { &rotaryKnob, &testRotary, &testRotary2 };

    std::vector<Component*> getComponents() {
        return activeComps;
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


// class OSCMessageCreatorComponent: public Component, public TextEditor::Listener, public TextButton::Listener {
// public:
//     OSCMessageCreatorComponent() {
//         setOpaque(true);
//         setSize(1000, 800);
//         setVisible(true);
//     }
//
//     void paint(Graphics& g) override;
//
//     void resized() override;
// };





// TODO: Make Component static elements backgroundImage instead of rerendering each paint() call
// TODO: Refactor into OSCMan
class OSCDeviceSelectorComponent: public Component, public TextEditor::Listener, public TextButton::Listener {
public:
    OSCDeviceSelectorComponent() {
        setOpaque(true);
        setSize(600, 400);
        loadUICfgIntoStdLnF(lnf);
        initaliseComponents();
        setVisible(true);
    }

    void initaliseComponents() {
        ipAddressTextEditor.setTextToShowWhenEmpty("XXX.XXX.XXX.XXX", UICfg::TEXT_COLOUR_DARK);
        ipAddressTextEditor.addListener(this);
        portTextEditor.setTextToShowWhenEmpty("0-65535", UICfg::TEXT_COLOUR_DARK);
        portTextEditor.addListener(this);
        deviceNameTextEditor.setTextToShowWhenEmpty("Device Name", UICfg::TEXT_COLOUR_DARK);
        deviceNameTextEditor.addListener(this);

        inputErrors.setReadOnly(true);
        inputErrors.setMultiLine(true);

        // Don't use Look and Feel for colours as applyButton and cancelButton use different colours
        applyButton.setColour(TextButton::buttonColourId, UICfg::POSITIVE_BUTTON_COLOUR);
        applyButton.setColour(TextButton::buttonOver, UICfg::POSITIVE_OVER_BUTTON_COLOUR);
        applyButton.addListener(this);
        cancelButton.setColour(TextButton::buttonColourId, UICfg::NEGATIVE_BUTTON_COLOUR);
        cancelButton.setColour(TextButton::buttonOver, UICfg::NEGATIVE_OVER_BUTTON_COLOUR);
        cancelButton.addListener(this);
        // cancelButton.setLookAndFeel(&lnf);
    }

    // Keeping as default as JUCE does a pretty good job cleaning up
    ~OSCDeviceSelectorComponent() override = default;

    void paint(juce::Graphics& g) override {
        waitForResize();

        g.fillAll(UICfg::BG_COLOUR);

        // For debugging, let's paint all the boxes
        for (auto box: getAllBoundingBoxes()) {
            g.setColour(juce::Colours::red);
            g.drawRect(box);
        }


        g.setFont(titleFont);
        GlyphArrangement ga;

        g.setColour(UICfg::TEXT_COLOUR);
        g.drawFittedText("OSC Device Selector", titleArea.toNearestInt(), Justification::centredLeft, 1);

        inputErrors.setText(inputErrorsString);

        paintTextEditor(ipAddressTextEditor, ipAddrBox);
        paintTextEditor(portTextEditor, portBox);
        paintTextEditor(deviceNameTextEditor, deviceNameBox);

        paintTextEditor(inputErrors, inputErrorsBox);
        inputErrors.setFont(errorBoxFont);


        // Ok, now let's paint the labels
        g.setFont(titleFont);
        g.setColour(UICfg::TEXT_COLOUR);
        g.drawFittedText("IP Address", ipAddrLabelBox.toNearestInt(), Justification::centredLeft, 1);
        g.drawFittedText("Port", portLabelBox.toNearestInt(), Justification::centredLeft, 1);
        g.drawFittedText("Device Name", deviceNameLabelBox.toNearestInt(), Justification::centredLeft, 1);

        // Now, Apply and Cancel buttons
        applyButton.setBounds(applyButtonBox);
        addAndMakeVisible(applyButton);
        cancelButton.setBounds(cancelButtonBox);
        addAndMakeVisible(cancelButton);
    }


    void paintTextEditor(TextEditor &editor, Rectangle<int> &box) {
        editor.setBounds(box);
        editor.setLookAndFeel(&lnf);
        editor.setFont(inputBoxFont);
        editor.setJustification(Justification::centredLeft);
        addAndMakeVisible(editor);
    }


    void resized() override {
        resizeReady = false;

        auto winBounds = getLocalBounds();

        // Ok, let's go one by one.
        // First, let's do the area of the window title. Let's also use relative sizes
        // to make it easier to resize the window.

        Rectangle<int> tempBox;


        // Deal with Padding
        auto contentBounds = winBounds;
        contentBounds.removeFromBottom(UICfg::STD_PADDING * winBounds.getHeight());
        contentBounds.removeFromTop(UICfg::STD_PADDING * winBounds.getHeight());
        contentBounds.removeFromLeft(UICfg::STD_PADDING * winBounds.getWidth());
        contentBounds.removeFromRight(UICfg::STD_PADDING * winBounds.getWidth());

        // One tenth of the window width and height
        int widthTenth = contentBounds.getWidth() / 10;
        int heightTenth = contentBounds.getHeight() / 10;

        // Now, let's remove the area for the title bar
        titleArea = contentBounds.removeFromTop(heightTenth);
        titleFont = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, titleArea.getHeight()/2.f, Font::bold);

        // Let's first set the font for the input boxes
        inputBoxFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, heightTenth*0.9, Font::plain);
        errorBoxFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, heightTenth*0.4, Font::plain);

        // There is a bug where the font size is not set correctly when the window is resized for text already painted.
        // So, let's clear the text editor text, and set the text again
        ipAddressTextEditor.setText(""); ipAddressTextEditor.setText(ipAddressString);
        portTextEditor.setText(""); portTextEditor.setText(portString);
        deviceNameTextEditor.setText(""); deviceNameTextEditor.setText(deviceNameString);


        // Now IP address bar and Port bar
        // The height of these bars are 2/10 of the window heightFont
        // The width for IP Addr. is 7/10, the width for Port is 3/10

        // Labels
        tempBox = contentBounds.removeFromTop(heightTenth);
        ipAddrLabelBox = tempBox.removeFromLeft(widthTenth * 7);
        portLabelBox = tempBox;

        // Boxes
        tempBox = contentBounds.removeFromTop(heightTenth * 2);
        ipAddrBox = tempBox.removeFromLeft(widthTenth * 7);
        portBox = tempBox;


        // Device name bar
        deviceNameLabelBox = contentBounds.removeFromTop(heightTenth);
        deviceNameBox = contentBounds.removeFromTop(heightTenth * 2);

        // 1/10 of the window height for padding
        contentBounds.removeFromTop(heightTenth);

        // Apply and Cancel buttons
        applyButtonBox = contentBounds.removeFromLeft(widthTenth * 2);
        contentBounds.removeFromLeft(widthTenth);
        cancelButtonBox = contentBounds.removeFromLeft(widthTenth * 2);
        contentBounds.removeFromLeft(widthTenth);

        // Input errors box
        inputErrorsBox = contentBounds.removeFromTop(heightTenth * 2);

        resizeReady = true;
    }


    void textEditorTextChanged(TextEditor& editor) override {
        if (&editor == &ipAddressTextEditor)
            ipAddressString = ipAddressTextEditor.getText();
        else if (&editor == &portTextEditor)
            portString = portTextEditor.getText();
        else if (&editor == &deviceNameTextEditor)
            deviceNameString = deviceNameTextEditor.getText();
    }


    /* Returns true when inputs are valid, false when not.*/
    bool validateTextEditorOutputs() {
        inputErrorsString = String();
        bool noError = true;

        // Check if IP is valid
        auto validatorOut = isValidIPv4(ipAddressString);
        if (!validatorOut.isValid) {
            inputErrorsString << "Invalid IP Address: " << validatorOut.errorMessage << "\n";
            noError = false;
        }

        // Check if the port is valid
        validatorOut = isValidPort(portString);
        if (!validatorOut.isValid) {
            inputErrorsString << "Invalid Port: " << validatorOut.errorMessage << "\n";
            noError = false;
        }

        // Check if the device name is valid
        validatorOut = isValidDeviceName(deviceNameString);
        if (!validatorOut.isValid) {
            inputErrorsString << "Invalid Device Name: " << validatorOut.errorMessage << "\n";
            noError = false;
        }

        return noError;
    }


    void textEditorReturnKeyPressed(TextEditor & editor) override {
        // If the ipAddressTextEditor is returned, then check if it's valid
        if (&editor == &ipAddressTextEditor)
            ipAddressString = ipAddressTextEditor.getText();
        else if (&editor == &portTextEditor)
            portString = portTextEditor.getText();
        else if (&editor == &deviceNameTextEditor)
            deviceNameString = deviceNameTextEditor.getText();
        else
            jassertfalse; // This should never happen

        // Now, call the validator
        validateTextEditorOutputs();
    }


    void buttonClicked(Button *button) override {
        if (button == &applyButton) {
            if (validateTextEditorOutputs())
                exitPopup();
        }
        else if (button == &cancelButton)
            exitPopup();
        else
            jassertfalse; // This should never happen
    }

    OSCDevice getDevice() {
        if (!validateTextEditorOutputs()) {
            return OSCDevice();
        }
        OSCDevice device;
        device.ipAddress = ipAddressString;
        device.port = portString.getIntValue();
        device.deviceName = deviceNameString;
        return device;
    }

    void exitPopup() {
        exitModalState();
        getParentComponent()->userTriedToCloseWindow();
        delete this;
    }


private:
    LookAndFeel_V4 lnf;

    TextEditor ipAddressTextEditor;
    TextEditor portTextEditor;
    TextEditor deviceNameTextEditor;

    String ipAddressString {};
    String portString {};
    String deviceNameString {};

    // This will be a read-only text editor that will show invalid inputs and reasons.
    TextEditor inputErrors;
    String inputErrorsString {};

    TextButton applyButton { "Apply", "Apply changes to OSC Device" };
    TextButton cancelButton { "Cancel", "Cancel changes to OSC Device" };

    std::vector<juce::Component*> getComps() {
        return { &ipAddressTextEditor, &portTextEditor, &deviceNameTextEditor,
                 &inputErrors, &applyButton, &cancelButton };
    }


    // As the window is resized, we need to resize the components. When size has been called, then we can paint.
    bool resizeReady = false;

    // This is where we will list all the bounding boxes, declared in resized().
    Rectangle<int> titleArea;
    Rectangle<int> ipAddrBox;
    Rectangle<int> ipAddrLabelBox;
    Rectangle<int> portBox;
    Rectangle<int> portLabelBox;
    Rectangle<int> deviceNameBox;
    Rectangle<int> deviceNameLabelBox;
    Rectangle<int> applyButtonBox;
    Rectangle<int> cancelButtonBox;
    Rectangle<int> inputErrorsBox;

    std::vector<Rectangle<int>> getAllBoundingBoxes() {
        return {titleArea, ipAddrBox, portBox, deviceNameBox, applyButtonBox, cancelButtonBox,
                ipAddrLabelBox, portLabelBox, deviceNameLabelBox};
    }

    FontOptions titleFont;
    FontOptions inputBoxFont;
    FontOptions errorBoxFont;

    void waitForResize(int waitLoops = 100) {
        if (!resizeReady) {
            bool naturalExit = true;
            // Loop 100 times to wait for the resize to be ready. If not ready by end of for loop, return
            for (int i = 0; i < waitLoops; i++) {
                if (resizeReady) {
                    naturalExit = false;
                    break;
                }
            }
            if (naturalExit)
                DBG("Exited paint() due to resize not being ready");
            return;
        }
    }

};


class OSCDeviceSelectorWindow: public DocumentWindow {
public:
    OSCDeviceSelectorWindow (String name = "OSC Device Selector")
        : DocumentWindow (name,
                          Desktop::getInstance().getDefaultLookAndFeel()
                                                      .findColour (ResizableWindow::backgroundColourId),
                          DocumentWindow::allButtons) {
        oscDeviceSelectorComponent = new OSCDeviceSelectorComponent();
        setUsingNativeTitleBar (true);
        setFullScreen(false);
        setSize (1280, 540);
        setContentOwned (oscDeviceSelectorComponent, true);
        setResizable (true, true);
        setResizeLimits (640, 270, 1920, 1080);
        setAlwaysOnTop(true);

        #if JUCE_IOS || JUCE_ANDROID
                setFullScreen (true);
        #else
                setResizable (true, true);
                centreWithSize (getWidth(), getHeight());
        #endif

        setVisible (true);
    }


    void closeButtonPressed() override {
        returnedOSCDev = oscDeviceSelectorComponent->getDevice();
        if (!returnedOSCDev.deviceName.isEmpty()) {
            // Means valid OSC Device was inputted
            validOSCDevice = true;
        }
        // For now, quit the app when the window is closed.
        JUCEApplication::getInstance()->systemRequestedQuit();
        // delete this;
    }

private:
    String returnedIPAddr {};
    String returnedPort {};
    String returnedDeviceName {};

    OSCDeviceSelectorComponent *oscDeviceSelectorComponent { nullptr };
    OSCDevice returnedOSCDev {};
    bool validOSCDevice { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCDeviceSelectorWindow)
};
