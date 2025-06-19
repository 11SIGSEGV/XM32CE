#pragma once
#include <JuceHeader.h>
#include "OSCMan.h"
#include "Helpers.h"
#include "AppComponents.h"





inline void loadUICfgIntoStdLnF(LookAndFeel_V4 &lnf) {
    lnf.setColour(TextEditor::backgroundColourId, UICfg::TEXT_EDITOR_BG_COLOUR);
}



class ShowCommandListener {
    public:
    virtual ~ShowCommandListener() = default;

    // Called when command is sent by a child component
    virtual void commandOccured(ShowCommand) = 0;
};



// The component for the header bar in the main window.
struct HeaderBar: public Component, public Timer, public DrawableButton::Listener, public ShowCommandListener {
public:
    HeaderBar(ActiveShowOptions& activeShowOptions):
    activeShowOptions(activeShowOptions) {
        setOpaque(true);
        startTimer(500); // Update every 500ms
        addAndMakeVisible(stopButton);
        stopButton.addListener(this);
        addAndMakeVisible(playButton);
        playButton.addListener(this);
        addAndMakeVisible(upButton);
        upButton.addListener(this);
        addAndMakeVisible(downButton);
        downButton.addListener(this);
    }

    ~HeaderBar() override {
        stopButton.removeListener(this);
        playButton.removeListener(this);
        upButton.removeListener(this);
        downButton.removeListener(this);
        removeAllChildren();
    };


    void timerCallback() override {
        repaint();
    }

    void reconstructImage();

    void resized() override;
    void paint(Graphics& g) override;

    void registerListener(ShowCommandListener* lstnr) {
        showCommandListeners.push_back(lstnr);
    }


    void unregisterListener(ShowCommandListener* lstnr) {
        showCommandListeners.erase(
            std::remove(showCommandListeners.begin(), showCommandListeners.end(), lstnr),
            showCommandListeners.end());
    }

    // For all listeners registered, send the showcommand to the listener
    void _dispatchToListeners(ShowCommand command) {
        for (auto lstnr: showCommandListeners) {
            lstnr->commandOccured(command);
        }
    }

    void showOptionsChanged(ShowCommand command);

    void commandOccured(ShowCommand command) override;

    void buttonClicked(Button *btn) override {
        if (auto *button = dynamic_cast<DrawableButton *>(btn)) {
            if (button == &stopButton) {
                _dispatchToListeners(SHOW_STOP);
            } else if (button == &playButton) {
                _dispatchToListeners(SHOW_PLAY);
            } else if (button == &upButton) {
                _dispatchToListeners(SHOW_PREVIOUS_CUE);
            } else if (button == &downButton) {
                _dispatchToListeners(SHOW_NEXT_CUE);
            } else {
                jassertfalse; // Invalid button caught by listener
            }
        } else {
            jassertfalse; // This should never happen! Button listeners should always be DrawableButton
        }
    };


    void activeShowOptionsChanged() {
        repaint();
    }

private:
    String getCurrentTimeAsFormattedString() const {
        auto timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto *localTime = std::localtime(&timeNow);
        return String::formatted("%02d:%02d:%02d", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
    }

    const std::set<ShowCommand> _showCommandsRequiringImageReconstruction = {SHOW_CUE_INDEX_CHANGE, SHOW_NAME_CHANGE,
        CURRENT_CUE_ID_CHANGE};

    std::vector<ShowCommandListener*> showCommandListeners;
    ActiveShowOptions& activeShowOptions;
    Rectangle<int> showNameBox;
    Rectangle<int> cueIDBox;
    Rectangle<int> cueNoBox;
    Rectangle<int> stopBox;
    DrawableButton stopButton {"HeaderStopButton", DrawableButton::ImageFitted};
    Rectangle<int> downBox;
    DrawableButton downButton {"HeaderDownButton", DrawableButton::ImageFitted};
    Rectangle<int> upBox;
    DrawableButton upButton {"HeaderUpButton", DrawableButton::ImageFitted};
    Rectangle<int> playBox;
    DrawableButton playButton {"HeaderPlayButton", DrawableButton::ImageFitted};
    Rectangle<int> timeBox;
    Rectangle<float> timeTextBox;

    Image borderImage;

    // Let's load iconography!
    Rectangle<float> stopButtonIconBox;
    Rectangle<float> stopButtonTextBox;
    Image stopIcon {getIconImageFile(IconID::STOP)};
    Rectangle<float> downButtonIconBox;
    Image downIcon {getIconImageFile(IconID::DOWN_ARROW)};
    Rectangle<float> upButtonIconBox;
    Image upIcon {getIconImageFile(IconID::UP_ARROW)};
    Rectangle<float> playButtonIconBox;
    Rectangle<float> playButtonTextBox;
    Image playIcon {getIconImageFile(IconID::PLAY)};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderBar)
};



class MainComponent  : public Component, public ShowCommandListener
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



    void commandOccured(ShowCommand) override;


private:
    //==============================================================================
    // Your private member variables go here...
    Slider rotaryKnob; // [1]
    OSCSender sender; // [2]


    /* Encoder testRotary {
    //     HERTZ, -135.0, 20.0, 135.0, 20000.0, 0.5, 0.0, "20Hz",
    //  "20kHz", 2, ParamType::LOGF, true, true};
    //
    //
    // Encoder testRotary2 {
    //     DB, -135.0, -90.0, 135.0, 10.0, 0.5, 0.0, "-inf",
    //  "+10.0dB", 2, ParamType::LEVEL_1024, true, true};
    //
    //
    // EncoderRotary testRotary3 {
    //     DB, -135.0, -90.0, 135.0, 10.0, 0.5, 0.0, "-inf",
    //  "+10.0dB", 2, ParamType::LEVEL_1024, true, true};
    //
    // Encoder testRotary4 {
    //     OptionParam("test", "This is a cool test", "a long winded description", {"Hello", "world", "Bye", "World"}),};
    //
    // Encoder testRotary5 {
    //     EnumParam("test2", "This is a cool test2", "a very very long winded description", {"I", "Hate", "C++"}),};
    //
    // std::vector<Component*> activeComps = { &rotaryKnob, &testRotary, &testRotary2, &testRotary3, &testRotary4, &testRotary5 };
    */

    ActiveShowOptions activeShowOptions {"012345678901234567890123", "Test Description", "CueID123", 1, false, 10};
    HeaderBar headerBar {activeShowOptions};
    const std::vector<Component*> activeComps = { &headerBar };
    const std::vector<ShowCommandListener*> callbackCompsUponActiveShowOptionsChanged = { &headerBar };

    std::vector<Component*> getComponents() {
        return activeComps;
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};






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
