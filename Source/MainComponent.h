#pragma once
#include <JuceHeader.h>
#include "OSCMan.h"
#include "Helpers.h"
#include "AppComponents.h"


class ShowCommandListener {
    public:
    virtual ~ShowCommandListener() = default;

    /* Called when command is sent by a child component.
    A child component should implement this function but the parent should as well.
    When a child component detects a change, it should broadcast it to the parent through this function, then
    the parent should broadcast it to all registered children also through this function.
    Each component should implement actions for each ShowCommand it supports, and can simply ignore all other
    irrelevant ShowCommands.
    */
    virtual void commandOccurred(ShowCommand) = 0;
};



// The component for the header bar in the main window.
struct HeaderBar: public Component, public Timer, public DrawableButton::Listener, public ShowCommandListener {
public:
    // Constructor for HeaderBar object. Expect an active show options struct.
    HeaderBar(ActiveShowOptions& activeShowOptions):
    activeShowOptions(activeShowOptions) {
        setOpaque(true);
        startTimer(500); // Update every 500ms. For clock.
        addAndMakeVisible(stopButton);
        stopButton.addListener(this);
        addAndMakeVisible(playButton);
        playButton.addListener(this);
        addAndMakeVisible(upButton);
        upButton.addListener(this);
        addAndMakeVisible(downButton);
        downButton.addListener(this);
    }


    // Remove all listeners
    ~HeaderBar() override {
        stopButton.removeListener(this);
        playButton.removeListener(this);
        upButton.removeListener(this);
        downButton.removeListener(this);
        removeAllChildren();
    };


    // Repainting for Clock
    void timerCallback() override {
        repaint();
    }

    // Reconstructs image used to save from re-rendering the entire screen upon every paint() call.
    // Should only realistically be called when relevant activeShowOptions (e.g., title) and on resize.
    // paint() should hence never draw anything except the clock and the image drawn by this function.
    void reconstructImage();

    void resized() override;
    void paint(Graphics& g) override;


    // Register show command listener
    void registerListener(ShowCommandListener* lstnr) {
        showCommandListeners.push_back(lstnr);
    }


    // Unregister show command listener
    void unregisterListener(ShowCommandListener* lstnr) {
        showCommandListeners.erase(
            std::remove(showCommandListeners.begin(), showCommandListeners.end(), lstnr),
            showCommandListeners.end());
    }

    // Literally no fucking clue what this function does... it's not part of a listener,
    // it's not called by anything, I'm genuinely so confused
    void showOptionsChanged(ShowCommand command);

    // Used to listen for when command occurs. Part of inherited ShowCommandListener
    void commandOccurred(ShowCommand command) override;

    // Button Listener for all DrawableButton objects
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




private:
    // For all listeners registered, send the ShowCommand to the listener
    void _dispatchToListeners(ShowCommand command) {
        for (auto lstnr: showCommandListeners) {
            lstnr->commandOccurred(command);
        }
    }

    void activeShowOptionsChanged() {
        repaint();
    }




    static String getCurrentTimeAsFormattedString() {
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


    // Implemented to listen for ShowCommands. Broadcasts all commands to registered callbacks.
    void commandOccurred(ShowCommand) override;


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
    std::vector<CurrentCueInfo> cuesInfo;
    HeaderBar headerBar {activeShowOptions};
    const std::vector<Component*> activeComps = { &headerBar };
    const std::vector<ShowCommandListener*> callbackCompsUponActiveShowOptionsChanged = { &headerBar };

    std::vector<Component*> getComponents() {
        return activeComps;
    }

    OSCDeviceSender oscDeviceSender {"127.0.0.1", "10023", "X32"};
    OSCCueDispatcherManager dispatcher {oscDeviceSender};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
