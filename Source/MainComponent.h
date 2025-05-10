#pragma once
#include <JuceHeader.h>

#include "Helpers.h"


namespace UICfg {
    const String DEFAULT_SANS_SERIF_FONT_NAME = Font::getDefaultSansSerifFontName();
    const String DEFAULT_SERIF_FONT_NAME = Font::getDefaultSerifFontName();
    const int DEFAULT_TEXT_HEIGHT = 100;
    const Font DEFAULT_FONT = FontOptions(DEFAULT_SANS_SERIF_FONT_NAME, static_cast<float>(DEFAULT_TEXT_HEIGHT), Font::plain);
    const Colour BG_COLOUR (34, 34, 34);
    const Colour TEXT_COLOUR (238, 238, 238);
    const Colour TEXT_COLOUR_DARK (100, 100, 100);

    // For UI element sizes use relative sizes
    const float STD_PADDING = 1.f/30.f;

    // Look and feel Configs
    const Colour TEXT_EDITOR_BG_COLOUR (0.f, 0.f, 0.f, 0.f);
}



inline void loadUICfgIntoStdLnF(LookAndFeel_V4 &lnf) {
    lnf.setColour(TextEditor::backgroundColourId, UICfg::TEXT_EDITOR_BG_COLOUR);

}


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
    void paint (juce::Graphics&) override;
    void resized() override;

    static void showConnectionErrorMessage (const juce::String& messageText)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
            "Connection error",
            messageText,
            "OK");
    }


    void showAlertWindow();
private:
    //==============================================================================
    // Your private member variables go here...
    juce::Slider rotaryKnob; // [1]
    juce::OSCSender sender; // [2]



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


class OSCDeviceSelectorComponent: public Component, public TextEditor::Listener {
public:
    OSCDeviceSelectorComponent() {
        setOpaque(true);
        setSize(600, 400);
        loadUICfgIntoStdLnF(lnf);
        ipAddressTextEditor.setTextToShowWhenEmpty("XXX.XXX.XXX.XXX", UICfg::TEXT_COLOUR_DARK);
        ipAddressTextEditor.addListener(this);
        setVisible(true);
    }

    ~OSCDeviceSelectorComponent() override = default;

    void paint(juce::Graphics& g) override {
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
        g.fillAll(UICfg::BG_COLOUR);


        // For debugging, let's paint all the boxes

        for (auto box: getAllBoundingBoxes()) {
            g.setColour(juce::Colours::red);
            g.drawRect(box);
        }


        g.setFont(titleFont);
        GlyphArrangement ga;

        auto str = "OSC Device Selector";
        g.setColour(UICfg::TEXT_COLOUR);
        g.drawFittedText(str, titleArea.toNearestInt(), Justification::centredLeft, 1);


        ipAddressTextEditor.setLookAndFeel(&lnf);
        ipAddressTextEditor.setFont(inputBoxFont);
        ipAddressTextEditor.setBounds(ipAddrBox);
        addAndMakeVisible(ipAddressTextEditor);

    }


    void resized() override {
        resizeReady = false;

        auto winBounds = getLocalBounds();

        // int width = winBounds.getWidth();
        // int height = winBounds.getHeight();

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
        inputBoxFont = FontOptions(UICfg::DEFAULT_SERIF_FONT_NAME, heightTenth, Font::plain);


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
        deviceNameBox = contentBounds.removeFromTop(heightTenth * 2).removeFromLeft(widthTenth * 6);

        // 1/10 of the window height for padding
        contentBounds.removeFromTop(heightTenth);

        // Apply and Cancel buttons
        applyButtonBox = contentBounds.removeFromLeft(widthTenth * 2);
        contentBounds.removeFromLeft(widthTenth);
        cancelButtonBox = contentBounds.removeFromLeft(widthTenth * 2);

        resizeReady = true;
    }




    void textEditorTextChanged(TextEditor& editor) override {
        // if (&editor == &ipAddressTextEditor) {
        //     // Do something with the text
        //     // DBG("IP Address: " << ipAddressTextEditor.getText());
        // }
    }

    void textEditorReturnKeyPressed(TextEditor & editor) override {
        std::cout << "Callback!" << std::endl;
        // If the ipAddressTextEditor is returned, then check if it's valid
        if (&editor == &ipAddressTextEditor) {
            // Check if the IP address is valid
            auto ipAddress = ipAddressTextEditor.getText();
            if (!isValidIPv4(ipAddress)) {
                std::cout<< "Invalid IP address: " << ipAddress << std::endl;
            }
        }
    }
private:
    LookAndFeel_V4 lnf;

    TextEditor ipAddressTextEditor;

    std::vector<juce::Component*> getComps() {
        return { &ipAddressTextEditor };
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

    std::vector<Rectangle<int>> getAllBoundingBoxes() {
        return {titleArea, ipAddrBox, portBox, deviceNameBox, applyButtonBox, cancelButtonBox,
                ipAddrLabelBox, portLabelBox, deviceNameLabelBox};
    }

    FontOptions titleFont;
    FontOptions inputBoxFont;
};


class OSCDeviceSelectorWindow: public DocumentWindow {
public:
    OSCDeviceSelectorWindow (String name = "OSC Device Selector")
        : DocumentWindow (name,
                          Desktop::getInstance().getDefaultLookAndFeel()
                                                      .findColour (ResizableWindow::backgroundColourId),
                          DocumentWindow::allButtons) {
        setUsingNativeTitleBar (true);
        setFullScreen(false);
        setSize (1280, 540);
        setContentOwned (new OSCDeviceSelectorComponent(), true);
        setResizable (true, true);
        setResizeLimits (640, 270, 1920, 1080);

        #if JUCE_IOS || JUCE_ANDROID
                setFullScreen (true);
        #else
                setResizable (true, true);
                centreWithSize (getWidth(), getHeight());
        #endif

        setVisible (true);
    }


    void closeButtonPressed() override
    {
        // For now, quit the app when the window is closed.
        JUCEApplication::getInstance()->systemRequestedQuit();
        // delete this;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCDeviceSelectorWindow)
};
