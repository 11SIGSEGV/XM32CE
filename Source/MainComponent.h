#pragma once
#include <JuceHeader.h>
#include "OSCMan.h"
#include "Helpers.h"
#include "AppComponents.h"


//==============================================================================


// Class containing methods for required cue data. Just a wrapper for CurrentCueInfoVector used by
// DraggableListBoxItem and related classes.
struct CueListData: public DraggableListBoxItemData {
    CurrentCueInfoVector &cciVector;
    ActiveShowOptions &activeShowOptions;


    // Add a listener for when a ShowCommand occurs to the CueListData struct
    void addListener(ShowCommandListener* listener) {
        if (listener != nullptr) {
            listeners.push_back(listener);
        } else {
            jassertfalse; // Listener is null, this should never happen
        }
    }

    // Remove a listener
    void removeListener(ShowCommandListener* listener) {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }


    CueListData(CurrentCueInfoVector &cciVector, ActiveShowOptions& activeShowOptions):
    cciVector(cciVector), activeShowOptions(activeShowOptions) {}

    int getNumItems() override { return cciVector.getSize(); }

    void deleteItem(int index) override {
        cciVector.erase(cciVector.begin() + index);
    }

    // Not to be used as the virtual method does not provide enough info
    void addItemAtEnd() override {
    } // TODO: Figure out how to add a new CCI at the end of the vector... the virtual method does not implement passing a value to this function.

    // Semi-static function. Requires item and bounds to paint CueListItem.
    void paintContents(int rowNum, Graphics &g, Rectangle<int> bounds) override;

    void moveAfter(int indexOfItemToMove, int indexOfItemToPlaceAfter) override {
        if (indexOfItemToMove <= indexOfItemToPlaceAfter)
            cciVector.move(indexOfItemToMove, indexOfItemToPlaceAfter);
        else
            cciVector.move(indexOfItemToMove, indexOfItemToPlaceAfter + 1);
    }

    void moveBefore(int indexOfItemToMove, int indexOfItemToPlaceBefore) override {
        if (indexOfItemToMove <= indexOfItemToPlaceBefore)
            cciVector.move(indexOfItemToMove, indexOfItemToPlaceBefore - 1);
        else
            cciVector.move(indexOfItemToMove, indexOfItemToPlaceBefore);
    }



    // Sends ShowCommand to registered listeners
    void notifyListeners(ShowCommand command) {
        for (auto lstnr: listeners) {
            if (lstnr != nullptr) {
                lstnr->commandOccurred(command);
            } else {
                jassertfalse; // Listener is null, this should never happen
            }
        }
    }
    // Sends cueCommandOccurred to registered listeners
    void notifyCueListeners(ShowCommand command, std::string cciInternalID, size_t cciCurrentIndex) {
        for (auto lstnr: listeners) {
            if (lstnr != nullptr) {
                lstnr->cueCommandOccurred(command, cciInternalID, cciCurrentIndex);
            } else {
                jassertfalse; // Listener is null, this should never happen
            }
        }
    }
private:
    std::vector<ShowCommandListener*> listeners;
};


class GoToCueBtn: public Button {
public:
    GoToCueBtn(): Button("") {}
    void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {};
    void clicked() override { onClick(); };
    void clicked(const ModifierKeys &modifiers) override { onClick(); };
    void buttonStateChanged() override {};
    void paint(Graphics &g) override {};
    void paintOverChildren(Graphics &g) override {};
};


// Class for individual item (i.e., row) in Cue List.
class CueListItem: public DraggableListBoxItem {
public:
    CueListItem(DraggableListBox& lb, CueListData& data, int rn): DraggableListBoxItem(lb, data, rn), data(data), listBox(lb) {}
    ~CueListItem() override = default;

    void paint(Graphics &g) override;

    void resized() override {
        bounds = getLocalBounds();
    }

private:
    Rectangle<int> bounds;
    CueListData& data;
    DraggableListBox& listBox;
    std::unique_ptr<GoToCueBtn> goToCueBtn;
    Rectangle<int> lastBounds;
    // // Using addAndMakeVisible callbacks the parent component (i.e., this component) to repaint,
    // // causing an infinite loop.
    // bool justAddedBtn = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueListItem)
};



// Class for Draggable Cue Info List Component
class CueListModel: public DraggableListBoxModel {
public:
    CueListModel(DraggableListBox& lb, DraggableListBoxItemData& md)
        : DraggableListBoxModel(lb, md) {}

    Component* refreshComponentForRow(int rowNumber, bool, Component* componentToUpdate) override {
        std::unique_ptr<CueListItem> item(dynamic_cast<CueListItem*>(componentToUpdate));
        if (isPositiveAndBelow(rowNumber, modelData.getNumItems())) {
            item = std::make_unique<CueListItem>(listBox, (CueListData &)modelData, rowNumber);
        }
        return item.release();
    }
};


//==============================================================================


// Action List for Current Cue Information
struct CCIActionList : public Component, public ShowCommandListener {
public:
    CCIActionList(ActiveShowOptions &activeShowOptions, CurrentCueInfoVector &cciVector,
                  double targetFontSize): targetFontSize(targetFontSize), activeShowOptions(activeShowOptions),
                                          cciVector(cciVector) {
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
    String getWidthAdjustedArgumentValueString(const String &value, const String &typeAlias) const;

    String getWidthAdjustedArgumentValueString(const ValueStorer &value, ParamType type) const;

    String getWidthAdjustedVerboseName(const String &verboseName) const;

    String oatAppropriateForWidth(OSCActionType oat) const;


    CurrentCueInfo& getCCI() {
        return cciVector.getCurrentCueInfoByIndex(activeShowOptions.currentCueIndex);
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
    CurrentCueInfoVector &cciVector;

    CurrentCueInfo _blankCCI{};
    std::string lastRenderedCCIInternalID;

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
    CCISidePanel(ActiveShowOptions &activeShowOptions, CurrentCueInfoVector& cciVector);

    ~CCISidePanel() = default;


    void constructImage();

    void paint(Graphics &g) override;

    void resized() override;

    void resizeActionList();

    // Handles when the displayed cue changes. Not NEXT-PREVIOUS dependent (supports jumping to a random cue)
    void selectedCueChanged();

    void commandOccurred(ShowCommand) override;

    void cueCommandOccurred(ShowCommand, std::string cciInternalID, size_t cciCurrentIndex) override;

private:
    std::string lastCCIInternalID = ""; // Used to determine if the CCI has changed, so we can reconstruct the image

    ActiveShowOptions &activeShowOptions;
    CurrentCueInfoVector &cciVector;
    CCIActionList actionList;

    Image panelImage;

    Image playingIndicatorImage;
    Image stoppedIndicatorImage;

    Rectangle<int> cueNameBox;
    Rectangle<int> cueDescriptionBox;
    Rectangle<int> stoppedPlayingIndicatorBox;
    Rectangle<int> commandsTitleBox; // Literally just for the word "COMMANDS".
    Rectangle<int> viewportBox;
    // We use this as resizeActionList() needs to know its X, Y and Width. Technically, we don't need this to set the bounds of the viewport, but we do need it for the actionList.


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
        startTimer(249); // Update every 249ms. For clock.

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

    // Reconstruct the image behind the buttons. This is used whenever the buttons' enabled/disabled state is changed.
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

    // Pulls the new cue from ActiveShowOptions and updates component appropriately.
    void selectedCueChanged();

    // Used to listen for when command occurs. Part of inherited ShowCommandListener
    void commandOccurred(ShowCommand command) override;
    // Used to listen specifically for JUMP_TO_CUE actions.
    void cueCommandOccurred(ShowCommand, std::string cciInternalID, size_t cciCurrentIndex) override;

    // Button Listener for all DrawableButton objects
    void buttonClicked(Button *btn) override {
        if (auto *button = dynamic_cast<DrawableButton *>(btn)) {
            if (button == &stopButton) {
                _dispatchToListeners(SHOW_STOP);
            } else if (button == &playButton) {
                _dispatchToListeners(SHOW_PLAY);
                if (activeShowOptions.currentCueIndex + 1 < activeShowOptions.numberOfCueItems) {
                    // We want to automatically move to the next cue also, so simulate a NEXT_CUE
                    _dispatchToListeners(SHOW_NEXT_CUE);
                }
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


    // Get time as string in format of HH:MM:SS (24 hours, all zero padded)
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
        SHOW_NEXT_CUE, SHOW_PREVIOUS_CUE, SHOW_NAME_CHANGE, FULL_SHOW_RESET, CUES_ADDED, CUES_DELETED,
        CUE_INDEXS_CHANGED
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

    // Image buttonsFGImage;
    Image buttonsBGImage;
    Image borderImage;

    // Let's load iconography!
    Rectangle<float> stopButtonIconBox;
    Rectangle<float> stopButtonTextBox;
    Image stopIcon{getIconImageFile(IconID::STOP)};
    Image stopIconDisabled{getIconImageFile(IconID::STOP, true)};
    Rectangle<float> downButtonIconBox;
    Image downIcon{getIconImageFile(IconID::DOWN_ARROW)};
    Image downIconDisabled{getIconImageFile(IconID::DOWN_ARROW, true)};
    Rectangle<float> upButtonIconBox;
    Image upIcon{getIconImageFile(IconID::UP_ARROW)};
    Image upIconDisabled{getIconImageFile(IconID::UP_ARROW, true)};
    Rectangle<float> playButtonIconBox;
    Rectangle<float> playButtonTextBox;
    Image playIcon{getIconImageFile(IconID::PLAY)};
    Image playIconDisabled{getIconImageFile(IconID::PLAY, true)};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};


//==============================================================================


class MainComponent : public Component, public ShowCommandListener, public OSCDispatcherListener,
    public ParentWindowListener, public KeyListener {
public:
    //==============================================================================
    MainComponent();

    ~MainComponent() override {
        terminateChildWindows();
        dispatcher.stopThread(5000);
        headerBar.unregisterListener(this);
        removeAllChildren();
    }

    //==============================================================================
    void paint(Graphics &g) override;

    void resized() override;

    // Updates values of activeShowOptions based on the new index of the CCI Vector
    void updateActiveShowOptionsFromCCIIndex(size_t newIndex);


    // Implemented to listen for ShowCommands.
    void commandOccurred(ShowCommand) override;

    // Broadcasts all commands to registered callbacks
    void sendCommandToAllListeners(ShowCommand cmd, bool currentCueListItemRequiresRedraw = false);


    // Implemented to listen for individual-cue ShowCommands
    void cueCommandOccurred(ShowCommand, std::string cciInternalID, size_t cciCurrentIndex) override;
    // Broadcasts cue commands to registered callbacks
    void sendCueCommandToAllListeners(ShowCommand, const std::string &cciInternalID, size_t cciCurrentIndex) const;

    // Set the correct index for the new CCI. Useful for when cue is moved.
    void setNewIndexForCCI() {
        activeShowOptions.currentCueIndex = cciVector.getIndexByCCIInternalID(activeShowOptions.currentCueInternalID);
    }

    // Receives callbacks from the OSCDispatcherManager when an individual action is finished.
    // Callbacks to cueCommandOccurred when an entire CCI's actions is completed.
    void actionFinished(std::string) override;

    // Receives callbacks when a child window needs to close.
    void closeRequested(WindowType windowType, std::string uuid) override;

    // Closes all child windows registered.
    void terminateChildWindows();

    bool keyPressed(const KeyPress &key, Component *originatingComponent) override;;

private:
    std::unordered_map<std::string, std::unique_ptr<OSCCCIConstructor>> cciConstructorWindows;
    Image backgroundPrerender;
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
    ActiveShowOptions activeShowOptions{"Demo Show", "Demo Show Description"};


    CurrentCueInfoVector cciVector = CurrentCueInfoVector(
        {
            {
                "S1", "Unmute and Live",
                "Initial Level and Unmute",
                {
                    CueOSCAction("/ch/01/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/02/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/03/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/05/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/07/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/08/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/09/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/10/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/11/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/13/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/05/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                    CueOSCAction("/ch/08/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                    CueOSCAction("/ch/09/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                    CueOSCAction("/ch/10/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                }
            },
            {
                "S2", "BG Fade",
                "Fades Non-Vocals.",
                {
                    CueOSCAction("/ch/05/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/06/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/08/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/09/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/10/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                },
            },
            {
                "S3", "Organ", "Brings up Organ",
                {
                    CueOSCAction("/ch/11/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f),
                                 ValueStorer(-5.f)),
                },
            },
            {
                "S4", "Backings", "Brings up Backing for Recp.",
                {
                    CueOSCAction("/ch/13/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f),
                                 ValueStorer(-2.f)),
                    CueOSCAction("/ch/03/mix/fader", 1.5f, Channel::FADER.NONITER, ValueStorer(-90.f),
                                 ValueStorer(5.f)),
                },
            },
            {
                "S5", "Fade Out", "Fade all channels out",
                {
                    CueOSCAction("/ch/03/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(5.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/05/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/07/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/08/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/09/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/10/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/11/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/13/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                },
            },
            {
                "D1", "Demo", "Demo EQ 1",
                {
                    CueOSCAction("/ch/01/eq/1/type", Channel::EQ_BAND_TYPE.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/01/eq/3/type", Channel::EQ_BAND_TYPE.getRawMessageArgument(), ValueStorer(2)),
                    CueOSCAction("/ch/01/eq/4/type", Channel::EQ_BAND_TYPE.getRawMessageArgument(), ValueStorer(4)),
                    CueOSCAction("/ch/01/eq/1/f", Channel::EQ_BAND_FREQ.getRawMessageArgument(), ValueStorer(185.f)),
                    CueOSCAction("/ch/01/eq/3/f", Channel::EQ_BAND_FREQ.getRawMessageArgument(), ValueStorer(4500.f)),
                    CueOSCAction("/ch/01/eq/4/f", Channel::EQ_BAND_FREQ.getRawMessageArgument(), ValueStorer(13600.f)),
                    CueOSCAction("/ch/01/eq/1/g", Channel::EQ_BAND_GAIN.getRawMessageArgument(), ValueStorer(6.f)),
                    CueOSCAction("/ch/01/eq/3/g", Channel::EQ_BAND_GAIN.getRawMessageArgument(), ValueStorer(9.8f)),
                    CueOSCAction("/ch/01/eq/4/g", Channel::EQ_BAND_GAIN.getRawMessageArgument(), ValueStorer(11.4f)),
                    CueOSCAction("/ch/01/eq/3/q", Channel::EQ_BAND_QLTY.getRawMessageArgument(), ValueStorer(0.8f)),
                },
            },
            {
                "S1", "Unmute and Live",
                "Initial Level and Unmute",
                {
                    CueOSCAction("/ch/01/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/02/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/03/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/05/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/07/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/08/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/09/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/10/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/11/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/13/mix/on", Channel::ON.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/05/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                    CueOSCAction("/ch/08/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                    CueOSCAction("/ch/09/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                    CueOSCAction("/ch/10/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f), ValueStorer(0.f)),
                }
            },
            {
                "S2", "BG Fade",
                "Fades Non-Vocals.",
                {
                    CueOSCAction("/ch/05/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/06/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/08/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/09/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                    CueOSCAction("/ch/10/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(0.f), ValueStorer(-2.f)),
                },
            },
            {
                "S3", "Organ", "Brings up Organ",
                {
                    CueOSCAction("/ch/11/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f),
                                 ValueStorer(-5.f)),
                },
            },
            {
                "S4", "Backings", "Brings up Backing for Recp.",
                {
                    CueOSCAction("/ch/13/mix/fader", 1.f, Channel::FADER.NONITER, ValueStorer(-90.f),
                                 ValueStorer(-2.f)),
                    CueOSCAction("/ch/03/mix/fader", 1.5f, Channel::FADER.NONITER, ValueStorer(-90.f),
                                 ValueStorer(5.f)),
                },
            },
            {
                "S5", "Fade Out", "Fade all channels out",
                {
                    CueOSCAction("/ch/03/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(5.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/05/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/07/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/08/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/09/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/10/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/11/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                    CueOSCAction("/ch/13/mix/fader", 2.f, Channel::FADER.NONITER, ValueStorer(-2.f), ValueStorer(-90.f)),
                },
            },
            {
                "D1", "Demo", "Demo EQ 1",
                {
                    CueOSCAction("/ch/01/eq/1/type", Channel::EQ_BAND_TYPE.getRawMessageArgument(), ValueStorer(1)),
                    CueOSCAction("/ch/01/eq/3/type", Channel::EQ_BAND_TYPE.getRawMessageArgument(), ValueStorer(2)),
                    CueOSCAction("/ch/01/eq/4/type", Channel::EQ_BAND_TYPE.getRawMessageArgument(), ValueStorer(4)),
                    CueOSCAction("/ch/01/eq/1/f", Channel::EQ_BAND_FREQ.getRawMessageArgument(), ValueStorer(185.f)),
                    CueOSCAction("/ch/01/eq/3/f", Channel::EQ_BAND_FREQ.getRawMessageArgument(), ValueStorer(4500.f)),
                    CueOSCAction("/ch/01/eq/4/f", Channel::EQ_BAND_FREQ.getRawMessageArgument(), ValueStorer(13600.f)),
                    CueOSCAction("/ch/01/eq/1/g", Channel::EQ_BAND_GAIN.getRawMessageArgument(), ValueStorer(6.f)),
                    CueOSCAction("/ch/01/eq/3/g", Channel::EQ_BAND_GAIN.getRawMessageArgument(), ValueStorer(9.8f)),
                    CueOSCAction("/ch/01/eq/4/g", Channel::EQ_BAND_GAIN.getRawMessageArgument(), ValueStorer(11.4f)),
                    CueOSCAction("/ch/01/eq/3/q", Channel::EQ_BAND_QLTY.getRawMessageArgument(), ValueStorer(0.8f)),
                },
            }
        }
    );


    HeaderBar headerBar{activeShowOptions};
    CCISidePanel sidePanel{activeShowOptions, cciVector};
    DraggableListBox cueListBox;
    CueListModel cueListModel;
    CueListData cueListData{cciVector, activeShowOptions};

    const std::vector<ShowCommandListener *> callbackCompsUponActiveShowOptionsChanged = {&headerBar, &sidePanel};
    const std::vector<Component *> activeComps = {&headerBar, &sidePanel, &cueListBox};

    OSCDeviceSender oscDeviceSender{"127.0.0.1", "10023", "X32"};
    OSCCueDispatcherManager dispatcher{oscDeviceSender};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
