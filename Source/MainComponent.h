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
    };

    void addItemAtEnd() override { } // TODO: Figure out how to add a new CCI at the end of the vector... the virtual method does not implement passing a value to this function.

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
private:
    std::vector<ShowCommandListener*> listeners;

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
};


// Class for individual item (i.e., row) in Cue List.
class CueListItem: public DraggableListBoxItem {
public:
    CueListItem(DraggableListBox& lb, CueListData& data, int rn): DraggableListBoxItem(lb, data, rn), data(data) {}
    ~CueListItem() override = default;

    void paint(Graphics &g) override {
        DraggableListBoxItem::paint(g);
    }

    void resized() override {
        bounds = getLocalBounds();
    }

private:
    Rectangle<int> bounds;
    CueListData& data;

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
    String getWidthAdjustedArgumentValueString(const String &value, const String &typeAlias);

    String getWidthAdjustedArgumentValueString(const ValueStorer &value, ParamType type);

    String getWidthAdjustedVerboseName(const String &verboseName);

    String oatAppropriateForWidth(OSCActionType oat);


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

    void commandOccurred(ShowCommand) override;

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


class MainComponent : public Component, public ShowCommandListener, public OSCDispatcherListener {
public:
    //==============================================================================
    MainComponent();

    ~MainComponent() override {
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
    void sendCueCommandToAllListeners(ShowCommand, std::string cciInternalID, size_t cciCurrentIndex);

    // Set the correct index for the new CCI. Useful for when cue is moved.
    void setNewIndexForCCI() {
        activeShowOptions.currentCueIndex = cciVector.getIndexByCCIInternalID(activeShowOptions.currentCueInternalID);
    }

    // Receives callbacks from the OSCDispatcherManager when an individual action is finished.
    // Callbacks to cueCommandOccurred when an entire CCI's actions is completed.
    void actionFinished(std::string) override;

private:
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
    ActiveShowOptions activeShowOptions{"Terrence is Fat", "Test Description"};


    CurrentCueInfoVector cciVector = CurrentCueInfoVector(
        {
            {
                "FT1", "Speaker 2",
                "Switch to Speaker 2. Fades channel level to -inf, changes name, icon and colour.",
                {
                    CueOSCAction("/ch/02/config/name", Channel::NAME.second[0], ValueStorer("Speaker 2")),
                    CueOSCAction("/ch/02/config/icon", Channel::ICON.second[0], ValueStorer(52)),
                    CueOSCAction("/ch/02/config/color", Channel::COLOUR.second[0], ValueStorer(11)),
                    CueOSCAction("/ch/02/mix/fader", 2.f, Channel::FADER.second[0], ValueStorer(-20.f), ValueStorer(-90.f))
                }
            },
            {
                "FT2", "Speaker 2 Prepare",
                "Prepares Speaker 2. Fades chanel up and changes colour.",
                {
                    CueOSCAction("/ch/02/mix/fader", 5.f, Channel::FADER.second[0], ValueStorer(-90.f), ValueStorer(0.f)),
                    CueOSCAction("/ch/02/config/color", Channel::COLOUR.second[0], ValueStorer(3)),
                },
            },
            {
                "FATTerrence", "Terrence is actually so fat", "",
                {
                    CueOSCAction("/ch/02/delay/time", Channel::DELAY_TIME.second[0], ValueStorer(100.f))
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

    OSCDeviceSender oscDeviceSender{"192.168.0.100", "10023", "X32"};
    OSCCueDispatcherManager dispatcher{oscDeviceSender};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
