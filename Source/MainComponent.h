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

    While most components will implement this function directly in the thread it was called, MainComponent must
    implement it as a function that will always ultimately execute the command in the main thread. This means a
    queue of awaiting ShowCommands should be used, and the main thread should process them in a loop
    until the queue is empty. This is to ensure that all ShowCommands are executed in the main thread in the recieved
    order. If one component takes too long to process a ShowCommand, this is a sacrifice that has to be made to ensure
    atomicity.
    */
    virtual void commandOccurred(ShowCommand) = 0;
};


//==============================================================================

// Action List for Current Cue Information
struct CCIActionList : public Component, public ShowCommandListener {
public:
    CCIActionList(ActiveShowOptions &activeShowOptions, std::vector<CurrentCueInfo> &currentCueInfos,
                  double targetFontSize): targetFontSize(targetFontSize), activeShowOptions(activeShowOptions),
                                          currentCueInfos(currentCueInfos) {
    }

    ~CCIActionList() = default;

    void resized() override;

    // A side note... paint() is actually the only function that access getCCI(), so when the CCI changes, realistically
    // we only need to call repaint().
    void paint(Graphics &g) override;

    void commandOccurred(ShowCommand) override;

    // TODO: Verify if repaint() also calls resized()
    void setTargetFontSize(float newFontSize, bool requiresRepaint = true) {
        targetFontSize = newFontSize;
        if (requiresRepaint) {
            resized();
            repaint();
        }
    }

    /* As this component should be inside a viewport, to determine the height required to set the content
    component without compressing or cropping this component, use this function.
    usingFontSize should be slightly smaller compared to the other UI fonts. Will use default value specified in
    construction if not specifically specified.
    NOTE: The returned value is ONLY APPROXIMATE.*/
    int getTheoreticallyRequiredHeight(float usingFontSize = -1);

    int getLastRenderHeight() const {
        return lastRenderHeight;
    }

private:
    /* Changes string based on width available for ValueStorer value. If the width is too short,
    it will adjust the string to '...', '', or concatenate the value.
    NOTE: typeAlias must only be ONE character long.
    NOTE: standarised typealiases include:
    - s: string
    - f: float
    - i: integer
    - ?: unknown (though... you should probably not use it)
    */
    String getWidthAdjustedArgumentValueString(const String &value, const String &typeAlias);

    String getWidthAdjustedArgumentValueString(const ValueStorer &value, ParamType type);

    String getWidthAdjustedVerboseName(const String &verboseName);

    String oatAppropriateForWidth(OSCActionType oat);


    CurrentCueInfo &getCCI() {
        if (activeShowOptions.numberOfCueItems == 0) {
            // Guys... we don't have a CCI... ummm
            // We return a blank!
            return _blankCCI;
        }
        // We're gonna be optimistic and assume that the currentCueIndex is valid.
        return currentCueInfos[activeShowOptions.currentCueIndex];
    }


    // Draws the formatted argument names and values to the graphics context (i.e., width-adjusted strings)
    // Returns the rectangle box in which the text was drawn - this rectangle will encompass BOTH texts drawn
    // It is recommended to specify the rounded height for the verbose name font and oat argument font, as it will save
    // a repetitive computation for every time this function is called
    Rectangle<int> drawArgumentNameAndValue(Graphics &g, int leftmostX, float currentHeight,
                                            const String &formattedVerboseName, const String &formattedArgumentValue,
                                            int verboseNameFontHeightRounded = -1,
                                            int oatArgumentFontHeightRounded = -1);

    float valueAndTypeMaxWidth;
    float argumentVerboseNameMaxWidth;

    ActiveShowOptions &activeShowOptions;
    std::vector<CurrentCueInfo> &currentCueInfos;

    CurrentCueInfo _blankCCI{};

    float targetFontSize;
    Font oscArgumentValueFont = FontOptions();
    int oscArgumentValueMaxChars;
    Font oatFont = FontOptions(); // OSC Argument Type Font (COMMANDS/FADE)
    int oatFontMaxChars;
    Font verboseNameFont = FontOptions();
    int verboseNameMaxChars;
    Font pathFont = FontOptions();
    int pathMaxChars; // These widths assume that the font is monospace

    int lastRenderHeight = 0; // The height last rendered by paint() for this component. Rounded up.
};


//==============================================================================


// Current Cue Information Side Panel
struct CCISidePanel : public Component, public ShowCommandListener {
public:
    CCISidePanel(ActiveShowOptions &activeShowOptions, std::vector<CurrentCueInfo> &currentCueInfos);

    ~CCISidePanel() = default;


    void constructImage();

    void paint(Graphics &g) override;

    void resized() override;
    void resizeActionList();

    void commandOccurred(ShowCommand) override;

private:
    ActiveShowOptions &activeShowOptions;
    std::vector<CurrentCueInfo> &currentCueInfos;
    CCIActionList actionList;

    Image panelImage;

    Image playingIndicatorImage;
    Image stoppedIndicatorImage;

    Rectangle<int> cueNameBox;
    Rectangle<int> cueDescriptionBox;
    Rectangle<int> stoppedPlayingIndicatorBox;
    Rectangle<int> commandsTitleBox; // Literally just for the word "COMMANDS".
    Rectangle<int> viewportBox; // We use this as resizeActionList() needs to know its X, Y and Width. Technically, we don't need this to set the bounds of the viewport, but we do need it for the actionList.


    Font titleFont = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::bold);
    Font textFont = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::plain);
    Font monospaceFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);


    Viewport cueActionListViewport;

    // If LocalBounds dimensions are zero for width and/or height, will return true.
    bool localBoundsIsInvalid() {
        auto lbnds = getLocalBounds();
        return lbnds.getWidth() <= 0 || lbnds.getHeight() <= 0;
    }
};


//==============================================================================


// The component for the header bar in the main window.
struct HeaderBar : public Component, public Timer, public DrawableButton::Listener, public ShowCommandListener {
public:
    // Constructor for HeaderBar object. Expect an active show options struct.
    HeaderBar(ActiveShowOptions &activeShowOptions): activeShowOptions(activeShowOptions) {
        setOpaque(true);
        startTimer(500); // Update every 500ms. For clock.

        stopButton.addListener(this);
        // stopButton.setOpaque(true/);
        addAndMakeVisible(stopButton);
        playButton.addListener(this);
        // playButton.setOpaque(false);
        addAndMakeVisible(playButton);
        upButton.addListener(this);
        // upButton.setOpaque(false);
        addAndMakeVisible(upButton);
        downButton.addListener(this);
        // downButton.setOpaque(false);
        addAndMakeVisible(downButton);

        commandOccurred(FULL_SHOW_RESET);
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

    void reconstructButtonBackgroundImage();

    void resized() override;

    void paint(Graphics &g) override;


    // Register show command listener
    void registerListener(ShowCommandListener *lstnr) {
        showCommandListeners.push_back(lstnr);
    }


    // Unregister show command listener
    void unregisterListener(ShowCommandListener *lstnr) {
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
    }

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

    // TODO: remove function, refactor calls
    void setButtonEnabled(DrawableButton &btn, bool enabled) {
        btn.setEnabled(enabled);
    }


    static String getCurrentTimeAsFormattedString() {
        auto timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto *localTime = std::localtime(&timeNow);
        return String::formatted("%02d:%02d:%02d", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
    }

    // If LocalBounds dimensions are zero for width and/or height, will return true.
    bool localBoundsIsInvalid() {
        auto lbnds = getLocalBounds();
        return lbnds.getWidth() <= 0 || lbnds.getHeight() <= 0;
    }

    const std::set<ShowCommand> _showCommandsRequiringImageReconstruction = {
        SHOW_NEXT_CUE, SHOW_PREVIOUS_CUE, SHOW_NAME_CHANGE,
        CURRENT_CUE_ID_CHANGE, FULL_SHOW_RESET
    };
    const std::set<ShowCommand> _showCommandsRequiringButtonReconstruction = {SHOW_STOP, SHOW_PLAY};

    std::vector<ShowCommandListener *> showCommandListeners;
    ActiveShowOptions &activeShowOptions;

    Rectangle<int> buttonsBox;

    Rectangle<int> showNameBox;
    Rectangle<int> cueIDBox;
    Rectangle<int> cueNoBox;
    Rectangle<int> stopBox;
    DrawableButton stopButton{"HeaderStopButton", DrawableButton::ImageFitted};
    Rectangle<int> downBox;
    DrawableButton downButton{"HeaderDownButton", DrawableButton::ImageFitted};
    Rectangle<int> upBox;
    DrawableButton upButton{"HeaderUpButton", DrawableButton::ImageFitted};
    Rectangle<int> playBox;
    DrawableButton playButton{"HeaderPlayButton", DrawableButton::ImageFitted};
    Rectangle<int> timeBox;
    Rectangle<float> timeTextBox;

    Image buttonsFGImage;
    Image buttonsBGImage;
    Image borderImage;

    // Let's load iconography!
    Rectangle<float> stopButtonIconBox;
    Rectangle<float> stopButtonTextBox;
    Image stopIcon{getIconImageFile(IconID::STOP)};
    Rectangle<float> downButtonIconBox;
    Image downIcon{getIconImageFile(IconID::DOWN_ARROW)};
    Rectangle<float> upButtonIconBox;
    Image upIcon{getIconImageFile(IconID::UP_ARROW)};
    Rectangle<float> playButtonIconBox;
    Rectangle<float> playButtonTextBox;
    Image playIcon{getIconImageFile(IconID::PLAY)};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};


//==============================================================================


class MainComponent : public Component, public ShowCommandListener, public OSCDispatcherListener, public HighResolutionTimer {
public:
    //==============================================================================
    // The average human reaction time is around 100ms at minimum, so we can get away with 50ms for the timer callback.
    MainComponent(int timerCallbackForShowCommandQueueIntervalMS = 50);

    ~MainComponent() override {
        dispatcher.stopThread(5000);
        headerBar.unregisterListener(this);
        stopTimer();
        removeAllChildren();
    }

    //==============================================================================
    void paint(Graphics &g) override;

    void resized() override;


    void updateActiveShowOptionsFromCCIIndex(int newIndex);
    // Updates cciIDtoIndexMap based on cuesInfo vector.
    void updateCCIIDToIndexMap();

    // Updates entire actionIDtoCCIInternalIDMap based on cuesInfo vector.
    void updateActionIDToCCIDIndexMap();

    // Updates all action IDs associated with a CCI.
    void updateActionIDToCCIDIndexMap(const CurrentCueInfo &cci);

    // Adds a running action ID to the CCI internal ID map. Used to track which actions in a CCI are currently running.
    // Should be called when a CCI's actions are dispatched to the OSCDispatcherManager.
    void addRunningActionID(std::string actionID, std::string cciInternalID = "");
    // Loops through all actions in the CCI and calls addRunningActionID() for each action.
    void addRunningActionsIDViaCCI(const CurrentCueInfo& cci);

    // Removes action ID from the map of running action IDs. Returns the number of actionIDs still running
    // for the CCI AFTER removing the actionID. (i.e., cciIDToRunningActionIDs[cci.ID].size()). Returns -1 in an error.
    int removeRunningActionID(std::string actionID, std::string cciInternalID = "");

    // Loops through all actions in the CCI and calls removeRunningActionID() for each action.
    void removeRunningActionsIDViaCCI(const CurrentCueInfo& cci);


    void setPlayStatusForCurrentCue(bool isPlaying);
    void setPlayStatusForCurrentCueByIndex(int index, bool isPlaying);
    // Expects valid CCI internal ID (should be UUID-like). Returns -1 when not found.
    int getCueIndexFromCCIInternalID(std::string cciInternalID);


    // Implemented to listen for ShowCommands. Broadcasts all commands to registered callbacks.
    void commandOccurred(ShowCommand) override;
    // Actually handles the ShowCommands. commandOccurred can be called from any thread, but this function
    // will always be called in the main thread.
    void hiResTimerCallback() override;
    void sendCommandToAllListeners(ShowCommand);

    // Receives callbacks from the OSCDispatcherManager.
    void actionFinished(std::string) override;

private:
    //==============================================================================
    // Your private member variables go here...
    Slider rotaryKnob;
    OSCSender sender;


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

    // NOTE: When switching from numberOfCueItems==0 to any other value, a FULL_SHOW_RESET command must be sent.
    // A reminder that currentCueIndex is 0-indexed... but numberOfCueItems is NOT.
    ActiveShowOptions activeShowOptions{"Terrence is Fat", "Test Description"};

    TSQueue<ShowCommand> awaitingShowCommands; // Queue of ShowCommands to be processed in the main thread.

    std::unordered_map<std::string, std::set<std::string>> cciIDToRunningActionIDs; // Previously called currentCueInformationInternalIdentificationToCueOpenSoundControlActionIdentificationsWaitingForOpenSoundControlManagerDispatcherListenerToCallbackFinishedSingleActionDispatcherJob
    std::unordered_map<std::string, std::string> actionIDtoCCIInternalIDMap; // Maps action ID to parent CCI internal ID
    std::unordered_map<std::string, int> cciIDtoIndexMap; // Maps CCI ID to index in cuesInfo vector.

    std::vector<CurrentCueInfo> cuesInfo = {
        {
            "FT1", "Speaker 2",
            "Switch to Speaker 2. Fades channel level to -inf, changes name, icon and colour.",
            {
                CueOSCAction("/ch/02/config/name", Channel::NAME.second[0], ValueStorer("Speaker 2")),
                CueOSCAction("/ch/02/config/icon", Channel::ICON.second[0], ValueStorer(52)),
                CueOSCAction("/ch/02/mix/fader", 2.f, Channel::FADER.second[0], ValueStorer(-20.f), ValueStorer(-90.f))
                }
        },
        {
            "TerrenceRealFat", "Test Cue With Very Very Long Title Which is Unreasonable",
            "Lorem ipsum dolor sit amet consectetur adipiscing elit. Consectetur adipiscing elit quisque faucibus ex sapien vitae.",
            {},
        },
        {
            "FATTerrence", "Terrence is actually so fat", "",
            {},
        }
    }; // May replace with custom struct in future
    std::vector<Component *> getComponents() {
        return activeComps;
    }


    HeaderBar headerBar{activeShowOptions};
    CCISidePanel sidePanel{activeShowOptions, cuesInfo};

    const std::vector<ShowCommandListener *> callbackCompsUponActiveShowOptionsChanged = {&headerBar, &sidePanel};

    OSCDeviceSender oscDeviceSender{"127.0.0.1", "10023", "X32"};
    OSCCueDispatcherManager dispatcher{oscDeviceSender};
    const std::vector<Component *> activeComps = {&headerBar, &sidePanel};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
