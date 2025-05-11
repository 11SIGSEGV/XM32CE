#pragma once
#include <JuceHeader.h>

#include <utility>
#include "Helpers.h"


namespace UICfg {
    const String DEFAULT_SANS_SERIF_FONT_NAME = Font::getDefaultSansSerifFontName();
    const String DEFAULT_SERIF_FONT_NAME = Font::getDefaultSerifFontName();
    const String DEFAULT_MONOSPACE_FONT_NAME = Font::getDefaultMonospacedFontName();
    const int DEFAULT_TEXT_HEIGHT = 100;
    const Font DEFAULT_FONT = FontOptions(DEFAULT_SANS_SERIF_FONT_NAME, static_cast<float>(DEFAULT_TEXT_HEIGHT), Font::plain);

    const Colour BG_COLOUR (34, 34, 34);
    const Colour TEXT_COLOUR (238, 238, 238);
    const Colour TEXT_COLOUR_DARK (100, 100, 100);

    const Colour POSITIVE_BUTTON_COLOUR (89, 177, 128);
    const Colour POSITIVE_OVER_BUTTON_COLOUR (102, 208, 149);
    const Colour POSITIVE_DOWN_BUTTON_COLOUR (102, 208, 149);
    const Colour NEGATIVE_BUTTON_COLOUR (226, 67, 67);
    const Colour NEGATIVE_OVER_BUTTON_COLOUR (254, 38, 38);
    const Colour NEGATIVE_DOWN_BUTTON_COLOUR (132, 10, 10);

    // For UI element sizes use relative sizes
    const float STD_PADDING = 1.f/30.f;

    // Look and feel Configs
    const Colour TEXT_EDITOR_BG_COLOUR (0.f, 0.f, 0.f, 0.f);
}



inline void loadUICfgIntoStdLnF(LookAndFeel_V4 &lnf) {
    lnf.setColour(TextEditor::backgroundColourId, UICfg::TEXT_EDITOR_BG_COLOUR);

}


struct OSCDevice {
    String ipAddress;
    int port;
    String deviceName;
};


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    static void showConnectionErrorMessage (const String& messageText)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
            "Connection error",
            messageText,
            "OK");
    }


    void showAlertWindow();
private:
    //==============================================================================
    // Your private member variables go here...
    Slider rotaryKnob; // [1]
    OSCSender sender; // [2]



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


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
            for (int i = 0; i < 100; i++) {
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
