/*
  ==============================================================================

    Helpers.h
    Created: 10 May 2025 4:19:51pm
    Author:  anony

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <unordered_map>
#include "XM32Maps.h"
#include "modules.h"
#include "X32Templates.h"



enum ShowCommand {
    SHOW_PLAY,
    SHOW_STOP,
    SHOW_NEXT_CUE,
    SHOW_PREVIOUS_CUE,
    SHOW_NAME_CHANGE,
    CUES_ADDED,
    CUES_DELETED,
    JUMP_TO_CUE, // Note this is called to cueCommandOccurred, not commandOccurred.
    FULL_SHOW_RESET, // Reset all UI and local variables

    CUE_INDEXS_CHANGED,

    // Individual Cue Commands
    CUE_STOPPED
};

/*
// Maps the ShowCommand cue for an Individual Cue Command when sent for an entire Show. Used when the individual cue
// command applies the currently selected CCI.
// (i.e., cueCommandOccurred() --> commandOccurred())
// E.g., CUE_STOPPED would map to SHOW_STOP.
inline constexpr std::unordered_map<ShowCommand, ShowCommand> INDIVIDUAL_CUE_TO_SHOW_CUE_MORPHS = {
    {CUE_STOPPED, SHOW_STOP}
};
*/

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


    /* Called for specific commands affecting only one single cue. commandOccurred will still be callbacked when a
     * command applies the current cue, but if a cue event happens to a specific cue that is not currently selected,
     * some components may still require it (e.g., CueList classes). This is for those components.
     */
    virtual void cueCommandOccurred(ShowCommand, std::string cciInternalID, size_t cciCurrentIndex);
};


inline UUIDGenerator uuidGen;


namespace UICfg {
    const String DEFAULT_SANS_SERIF_FONT_NAME = "Public Sans"/*Font::getDefaultSansSerifFontName()*/;
    const String DEFAULT_SERIF_FONT_NAME = "Public Sans"/*Font::getDefaultSerifFontName()*/;
    const String DEFAULT_MONOSPACE_FONT_NAME = "JetBrains Mono"/*Font::getDefaultMonospacedFontName()*/;
    constexpr int DEFAULT_TEXT_HEIGHT = 100;
    const Font DEFAULT_FONT = FontOptions(DEFAULT_SANS_SERIF_FONT_NAME, static_cast<float>(DEFAULT_TEXT_HEIGHT),
                                          Font::plain);
    const Font DEFAULT_MONOSPACE_FONT = FontOptions(DEFAULT_MONOSPACE_FONT_NAME,
                                                    static_cast<float>(DEFAULT_TEXT_HEIGHT), Font::plain);

    const Colour TRANSPARENT(0.f, 0.f, 0.f, 0.f);

    const Colour BG_COLOUR(34, 34, 34);
    const Colour BG_SECONDARY_COLOUR(51, 51, 51);
    const Colour LIGHT_BG_COLOUR(100, 100, 100);
    const Colour TEXT_COLOUR(238, 238, 238);
    const Colour TEXT_ACCENTED_COLOUR(212, 235, 255);
    const Colour TEXT_COLOUR_DARK(100, 100, 100);
    const Colour STRONG_BORDER_COLOUR(212, 212, 212);
    const Colour HEADER_BG_COLOUR (0, 0, 0);
    const Colour HEADER_BTN_DISABLED_BG_COLOUR (25, 25, 25);

    constexpr float COMPONENT_OUTLINE_THICKNESS_PROPORTIONAL_TO_PARENT_HEIGHT = 0.001;

    const Colour SELECTED_CUE_LIST_ITEM_BG_COLOUR(60, 60, 60);
    const Colour CUE_LIST_ITEM_OUTLINE_COLOUR(30, 30, 30);
    const Colour CUE_LIST_ITEM_INSIDE_OUTLINES_COLOUR(40, 40, 40);

    const Colour POSITIVE_BUTTON_COLOUR(89, 177, 128);
    const Colour POSITIVE_OVER_BUTTON_COLOUR(102, 208, 149);
    const Colour POSITIVE_DOWN_BUTTON_COLOUR(102, 208, 149);
    const Colour NEGATIVE_BUTTON_COLOUR(226, 67, 67);
    const Colour NEGATIVE_OVER_BUTTON_COLOUR(254, 38, 38);
    const Colour NEGATIVE_DOWN_BUTTON_COLOUR(132, 10, 10);

    constexpr float ROTARY_POINTER_WIDTH = 0.05f; // X% of half the bounding width (i.e., x% of the radius)
    const Colour ROTARY_POINTER_COLOUR(28U, 21u, 11u); // Darker variant of the background colour
    constexpr float ROTARY_TEXT_PADDING = 1.4f; // X% of half bounding width (i.e., x% of the radius). Should be >1.f.
    constexpr float ROTARY_TEXT_HEIGHT = 0.25f; // X% of half bounding width (i.e., x% of the radius)
    const Colour ROTARY_ENABLED_COLOUR(85u, 117u, 171u);
    const Colour ROTARY_TEXT_COLOUR = TEXT_COLOUR;
    const Colour ROTARY_DISABLED_COLOUR(40u, 50u, 65u);

    constexpr float FADER_CENTERLINE_SIDE_PADDING = 0.45f; // The amount from each side to reduce for the center line. Must be <0.5f.
    const Colour CENTERLINE_COLOUR (82, 88, 104);
    constexpr float LEVEL_MARKER_THICKNESS = 0.005f; // Thickness as a percentage of the bounds height
    constexpr float FADER_KNOB_WIDTH_AS_PROPORTION_TO_BOUNDS_WIDTH = 0.15f; // The percentage of the fader knob's width in proportion to the bounds width.
    constexpr float FADER_KNOB_HEIGHT_AS_PROPORTION_TO_WIDTH = 2.f; // The height of the fader knob versus its width
    constexpr float FADER_MARKER_LABEL_FONT_HEIGHT = 0.04f; // The height of the fader label text (e.g., -60, +5, etc.) in proportion to the bounds height
    constexpr float FADER_KNOB_LINE_HEIGHT_AS_PROPORTION_TO_HEIGHT = 0.1f; // The height of the line in the middle of the fader knob proportional to the height of the fader knob
    constexpr float FADER_VALUE_TEXT_FONT_HEIGHT = 0.07f; // The height of the fader value text proportional to the fader bounds height (not localbounds height)

    constexpr int ROUND_TO_WHEN_IN_DOUBT = 2; // Round to 2 decimal places when in doubt (e.g., no unit for the value).


    // For UI element sizes use relative sizes
    constexpr float STD_PADDING = 1.f / 40.f;

    // Look and feel Configs
    const Colour TEXT_EDITOR_BG_COLOUR(0.f, 0.f, 0.f, 0.f);
}


namespace IconID {
    constexpr int DOWN_ARROW = 1;
    constexpr int UP_ARROW = 2;
    constexpr int OCTAGON = 3;
    constexpr int STOP = OCTAGON;
    constexpr int PLAY = 4;
    constexpr int EDIT = 5;
    constexpr int DELETE = 6;
    constexpr int PLUS = 7;
}


inline const std::unordered_map<int, std::string> ICON_FILE_MAP = {
    {IconID::DOWN_ARROW, "down.png"},
    {IconID::UP_ARROW, "up.png"},
    {IconID::PLAY, "play.png"},
    {IconID::OCTAGON, "octagon.png"},
    {IconID::EDIT, "edit.png"},
    {IconID::DELETE, "delete.png"},
    {IconID::PLUS, "plus.png"},
};


const std::unordered_map<Units, int> ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT = {
    {HERTZ, 2},
    {NONE, 2}, // WARNING! Do not round to 2 digits for NONE. In fact, DO NOT ROUND AT ALL. This is merely here to prevent out_of_range exceptions!
    {DB, 2},
    {MS, 1}
};


String formatValueUsingUnit(const Units unit, double value);


namespace FileInfo {
    // NOTE: File::currentApplicationFile is actually executable file built... which will be in ./Builds/LinuxMakeFile/
    // TODO: On Deployment, change to .getParentDirectory().getParentDirectory() to get the root of the project.
    const File PARENT_DIRECTORY = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().
            getParentDirectory().getParentDirectory().getParentDirectory();
    const File RESOURCES_DIRECTORY = PARENT_DIRECTORY.getChildFile("Resources");
    const File ICONS_DIRECTORY = RESOURCES_DIRECTORY.getChildFile("Icons");
}

// NO LONGER USING ICONS! Just using ridiculously high-res PNGs. Juce really REALLY HATES SVGs. Even the official svg-->path tools in projucer don't work.
// Icons will be loaded by each individual component. If a component requires, it will load it.

// This function takes the icon ID and returns an Image object of the icon
Image getIconImageFile(int iconID, bool getDisabledVersion = false);


// Abbreviated as OAT
enum OSCActionType {
    OAT_COMMAND,
    OAT_FADE,
    EXIT_THREAD
};



struct CueOSCAction {
    explicit CueOSCAction(bool exitThread): oat(EXIT_THREAD), oscAddress("/") {
    }

    // For OAT_COMMAND, the arguments are used to fill in the OSC Message.
    CueOSCAction(OSCAddressPattern oscAddress, OSCMessageArguments argumentTemplate, ValueStorer argument, std::string argumentTemplateID = ""): oat(OAT_COMMAND),
        oatCommandOSCArgumentTemplate(argumentTemplate), argument(argument), oscAddress(oscAddress), ID(uuidGen.generate()),
    argumentTemplateID(argumentTemplateID) {
    }


    // For OAT_FADE, the fadeTime is used to determine the fade time in seconds.
    CueOSCAction(OSCAddressPattern oscAddress, float fadeTime, NonIter oscArgumentTemplate, ValueStorer startValue,
                 ValueStorer endValue, std::string argumentTemplateID = ""): oscAddress(oscAddress), oat(OAT_FADE), fadeTime(fadeTime),
                                        oscArgumentTemplate(oscArgumentTemplate),
                                        startValue(startValue), endValue(endValue), ID(uuidGen.generate()),
    argumentTemplateID(argumentTemplateID) {
        _checks();
    }

    // CueOSCAction(const CueOSCAction& other): oat(other.oat),
    //       oscAddress(other.oscAddress),
    //       oatCommandOSCArgumentTemplate(other.oatCommandOSCArgumentTemplate),
    //       argument(other.argument),
    //       fadeTime(other.fadeTime),
    //       oscArgumentTemplate(other.oscArgumentTemplate),
    //       startValue(other.startValue),
    //       endValue(other.endValue),
    //       ID(uuidGen.generate()), // Generate a new unique ID for the copy
    //       argumentTemplateID(other.argumentTemplateID)
    // {
    // }


    void _checks() const {
        if (oat == OAT_FADE) {
            if (oscArgumentTemplate._meta_PARAMTYPE == INT &&
                (startValue.intValue < oscArgumentTemplate.intMin ||
                 startValue.intValue > oscArgumentTemplate.intMax ||
                 endValue.intValue < oscArgumentTemplate.intMin ||
                 endValue.intValue > oscArgumentTemplate.intMax)
            ) {
                jassertfalse; // Start and end values must be within the NonIter template's intMin and intMax
            } else if (oscArgumentTemplate._meta_PARAMTYPE == _GENERIC_FLOAT &&
                       (startValue.floatValue < oscArgumentTemplate.floatMin ||
                        startValue.floatValue > oscArgumentTemplate.floatMax ||
                        endValue.floatValue < oscArgumentTemplate.floatMin ||
                        endValue.floatValue > oscArgumentTemplate.floatMax)
            ) {
                jassertfalse; // Start and end values must be within the NonIter template's floatMin and floatMax
            }
        }
    }

    const std::string ID;

    OSCActionType oat;
    OSCAddressPattern oscAddress;

    // For OAT_COMMAND
    // Let's sacrifice the extra memory for the sake of reliability and stability.
    OSCMessageArguments oatCommandOSCArgumentTemplate = nullNonIter;
    ValueStorer argument; // The arguments to send with the OSC Message, only used for OAT_COMMAND


    // For OAT_FADE
    float fadeTime{0.f}; // The fade time in seconds, only used for OAT_FADE
    NonIter oscArgumentTemplate = nullNonIter; // Used to find algorithm and type for parameter
    ValueStorer startValue;
    ValueStorer endValue;

    // Can be empty. Will be when unknown or template not used.
    std::string argumentTemplateID {}; // Correlates to XM32Template object used.
};


struct CurrentCueInfo {
    // When INTERNAL_ID is empty, it is implied the CCI is not valid. This can be used when no CCIs are in a CCI Vector, so a blank CCI can be used.
    String id;
    String name;
    String description;
    std::vector<CueOSCAction> actions;
    bool currentlyPlaying {false};


    CurrentCueInfo(const String &id, const String &name, const String &description,
                   const std::vector<CueOSCAction>& actions): id(id), name(name), description(description),
                                                       actions(actions), INTERNAL_ID(uuidGen.generate()) {
    }

    // Used for blank CCI (i.e., invalid CCI)
    CurrentCueInfo(): id(""), name(""), description(""), actions({}) {
    }

    [[nodiscard]] bool isInvalid() const {
        return INTERNAL_ID.empty();
    }

    [[nodiscard]] std::string getInternalID() const {
        return INTERNAL_ID;
    }

private:
    std::string INTERNAL_ID; // This is a unique ID for the CCI, used to identify it in the CCI Vector. Not used for UI, and not user-friendly
};


struct CurrentCueInfoVector {
    std::vector<CurrentCueInfo> vector;

    CurrentCueInfo _blankCCI; // A blank CCI to return when an index is out of range or when no CCIs are available
    // Pre-created to avoid recreating for every invalid CCI access

    CurrentCueInfoVector(const CurrentCueInfoVector& other): vector(other.vector) {
        size = vector.size();
        reconstructCCIToIndexMap();
        reconstructActionCCIMap();
    }

    // Returns true if a CCI is in the vector. Uses the CCIIndexMap.
    bool cciInVector(const std::string &cciInternalID) {
        return cciIDtoIndexMap.find(cciInternalID) != cciIDtoIndexMap.end();
    }

    // CurrentCueInfoVector(const std::vector<CurrentCueInfo>& cciVector): vector(cciVector), size(cciVector.size()) {}


    explicit CurrentCueInfoVector(const std::vector<CurrentCueInfo>& cciVector): vector(cciVector), size(cciVector.size()) {
        reconstructCCIToIndexMap();
        reconstructActionCCIMap();
    }

    // Returns a pointer to the current cue info.
    // When assigning to a variable, ensure the use of auto&, not auto.
    CurrentCueInfo& getCurrentCueInfoByIndex(size_t index) {
        if (size == 0) {
            return _blankCCI;
        }
        if (index >= size) {
            jassertfalse; // Index out of range
            return _blankCCI; // Return a blank CCI
        }
        return vector[index];
    }


    // Just passes index to getCurrentCueInfoByIndex(). Again, ensure use auto&, not auto.
    CurrentCueInfo& operator[](const size_t index) {
        // TODO: Test because i have no clue what i'm doing
        return getCurrentCueInfoByIndex(index);
    }


    // Warning: the returned iterator is invalidated if the vector is modified! Use at your own risk. If you wish to
    // retrieve a CCI before deleting it, use getCurrentCueInfoByIndex() and then delete it.
    std::vector<CurrentCueInfo>::iterator erase(std::vector<CurrentCueInfo>::const_iterator first, std::vector<CurrentCueInfo>::const_iterator last) {
        auto iter = vector.erase(first, last);
        uponCuesDeleted();
        return iter;
    }

    // Warning: the returned iterator is invalidated if the vector is modified! Use at your own risk. If you wish to
    // retrieve a CCI before deleting it, use getCurrentCueInfoByIndex() and then delete it.
    std::vector<CurrentCueInfo>::iterator erase(size_t index) {
        auto iter = vector.erase(vector.begin() + index);
        uponCuesDeleted();
        return iter;
    }

    // Warning: the returned iterator is invalidated if the vector is modified! Use at your own risk. If you wish to
    // retrieve a CCI before deleting it, use getCurrentCueInfoByIndex() and then delete it.
    std::vector<CurrentCueInfo>::iterator erase(const std::vector<CurrentCueInfo>::const_iterator first) {
        auto iter = vector.erase(first);
        uponCuesDeleted();
        return iter;
    }


    void push_back(const CurrentCueInfo& cci) {
        vector.push_back(cci);
        uponCueAdd(cci);
    }

    // Based off https://stackoverflow.com/a/57399634/16571234! Thanks for the algorithm!
    void move(size_t oldIndex, size_t newIndex) {
        if (oldIndex > newIndex)
            std::rotate(vector.rend() - oldIndex - 1, vector.rend() - oldIndex, vector.rend() - newIndex);
        else
            std::rotate(vector.begin() + oldIndex, vector.begin() + oldIndex + 1, vector.begin() + newIndex + 1);
        updateCCIIndexMap(oldIndex, newIndex);
        _notifyListeners(CUE_INDEXS_CHANGED);
    }

    [[nodiscard]] std::vector<CurrentCueInfo>::const_iterator begin() const noexcept {
        return vector.begin();
    }

    [[nodiscard]] size_t getSize() const { return size; }


    void addListener(ShowCommandListener* listener) {
        if (listener != nullptr) {
            listeners.push_back(listener);
        } else {
            jassertfalse; // Listener is null, this should never happen
        }
    }

    void removeListener(ShowCommandListener* listener) {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    size_t getIndexByCCIInternalID(const std::string &cciInternalID) {
        auto it = cciIDtoIndexMap.find(cciInternalID);
        return it != cciIDtoIndexMap.end() ? it->second : sizeTLimit;
    }

    // Gets the parent CCI Internal ID of the CueOSCAction. Expects actionIDtoCCIInternalIDMap to be constructed and valid
    // If not, returns empty string.
    std::string getParentCCIInternalID(const CueOSCAction &action) {
        auto it = actionIDtoCCIInternalIDMap.find(action.ID);
        return it != actionIDtoCCIInternalIDMap.end() ? it->second : "";
    }

    // Gets the parent CCI Internal ID of the CueOSCAction. Expects actionIDtoCCIInternalIDMap to be constructed and valid
    // If not, returns empty string.
    std::string getParentCCIInternalID(const std::string &actionID) {
        auto it = actionIDtoCCIInternalIDMap.find(actionID);
        return it != actionIDtoCCIInternalIDMap.end() ? it->second : "";
    }

    // Sets an actionID as running. Automatically tries to get the cciInternalID from the actionIDtoCCIInternalIDMap
    // If fail, nothing will occur.
    void setAsRunning(const std::string &actionID, std::string cciInternalID = "") {
        if (cciInternalID.empty()) {
            // Figure it out ourselves
            auto it = actionIDtoCCIInternalIDMap.find(actionID);
            if (it == actionIDtoCCIInternalIDMap.end()) {
                jassertfalse; // What do you mean the actionID doesn't have a parent CCI mapped?
                return;
            }
            cciInternalID = it->second;
        }
        cciIDtoRunningActionIDsMap[cciInternalID].insert(actionID); // This will implicitly generate a set for
        // cciIDtoRunningActionIDsMap[cciInternalID] if actionID is not yet in cciIDtoRunningActionIDsMap.
    }

    // Loops through each action in the CCI and calls setAsRunning(std::string actionID, std::string cciInternalID = "")
    // for each action.
    void setAsRunning(const CurrentCueInfo& cci) {
        auto cciInternalID = cci.getInternalID();
        for (const auto& action: cci.actions) {
            setAsRunning(action.ID, cciInternalID);
        }
    }

    // Removes an actionID from the list of running actions for a CCI.
    // Does nothing if the actionID is not in the set of running actions for the CCI.
    // Returns size_t limit (this->sizeTLimit) when failed. Otherwise, returns the number of actionIDs left
    // in the set of running actionIDs for the cciInternalID (i.e., the number of running actions)
    size_t removeFromRunning(const std::string &actionID, std::string cciInternalID = "") {
        if (cciInternalID.empty()) {
            // Figure it out ourselves
            auto it = actionIDtoCCIInternalIDMap.find(actionID);
            if (it == actionIDtoCCIInternalIDMap.end()) {
                jassertfalse; // What do you mean the actionID doesn't have a parent CCI mapped?
                return sizeTLimit;
            }
            cciInternalID = it->second;
        }
        // Now we can remove it from the map
        auto cciIt = cciIDtoRunningActionIDsMap.find(cciInternalID);
        if (cciIt != cciIDtoRunningActionIDsMap.end()) {
            cciIt->second.erase(actionID); // Erase does not throw an exception, even if actionID mysteriously disappears
            return cciIt->second.size();
        }

        jassertfalse; // This should never happen! We should always have a CCI internal ID in the map.
        return sizeTLimit;
    }

    // Loops through all actions in the CCI and calls removeRunningActionID for each.
    void removeFromRunning(const CurrentCueInfo& cci) {
        auto cciInternalID = cci.getInternalID();
        for (const auto& action: cci.actions) {
            removeFromRunning(action.ID, cciInternalID);
        }
    }

    static constexpr size_t sizeTLimit = static_cast<size_t>(-1); // We use this to indicate an invalid index. But... if by some miracle the vector reaches this size, we must not use it.
private:
    // A.k.a. CCIIndexMap
    std::unordered_map<std::string, size_t> cciIDtoIndexMap; // Maps CCI internal ID to index in the vector

    // A.k.a., ActionCCIMap
    // A few notes about this:
    // 1. An actionID's parent CCI should never change; hence its CCI Internal ID should never change
    std::unordered_map<std::string, std::string> actionIDtoCCIInternalIDMap; // Maps actionIDs of each action in cci.actions to its parent's INTERNAL_ID (cci.INTERNAL_ID)

    // A.k.a. RunningActionsMap
    std::unordered_map<std::string, std::set<std::string>> cciIDtoRunningActionIDsMap; // Maps the CCI Internal IDs to the action IDs of the CCI which are still running


    size_t size;
    std::vector<ShowCommandListener*> listeners;

    void uponCueAdd(const CurrentCueInfo& cciAdded) {
        size++;
        if (size == sizeTLimit) {
            jassertfalse; // ...how did you get so many elements inside the vector? The size_t limit is used in this struct to represent an invalid index, so this may break the "static_cast<size_t>(-1)"th element.
        }
        addCCIToIndexMap(cciAdded.getInternalID());
        addToActionCCIMap(cciAdded);
        _notifyListeners(CUES_ADDED);
    }


    void uponCuesDeleted() {
        size = vector.size();
        reconstructCCIToIndexMap();
        reconstructActionCCIMap();
        _notifyListeners(CUES_DELETED);
    }


    void _notifyListeners(const ShowCommand command) const {
        for (auto& listener: listeners) {
            if (listener != nullptr) {
                listener->commandOccurred(command);
            } else {
                jassertfalse; // Listener is null, this should never happen
            }
        }
    }


    // Completely reconstructs cciIDToIndexMap
    void reconstructCCIToIndexMap() {
        std::unordered_map<std::string, size_t> tempMap;
        for (size_t i = 0; i < size; i++) {
            tempMap[vector[i].getInternalID()] = i;
        }
        cciIDtoIndexMap = tempMap;
    }


    size_t manuallyFindIndexOfCCIByInternalID(const std::string& cciInternalID) const {
        // This is a manual search, not using the map
        for (size_t i = 0; i < size; i++) {
            if (vector[i].getInternalID() == cciInternalID) {
                return i;
            }
        }
        return sizeTLimit; // Not found
    }

    /* Updates the cciIDtoIndexMap with an internal ID was not previously in the map
     * The function will find the index of the new CCI by its ID. The index the CCI was added at does not need to be specified.
     */
    void addCCIToIndexMap(const std::string &cciInternalID) {
        // Identify the index of the CCI
        const size_t index = manuallyFindIndexOfCCIByInternalID(cciInternalID);
        if (index == sizeTLimit) {
            jassertfalse; // This should never happen! We should always have a CCI internal ID in the vector.
            cciIDtoIndexMap[cciInternalID] = sizeTLimit;
        }
        cciIDtoIndexMap[cciInternalID] = index;
        // If there's any elements after this index, we also need to update them
        if (size - 1 == index) {
            return; // Added at the end of the vector
        }
        // For every index in the cciIDToIndexMap after the newly added CCI's index, we need to increment it by one
        // This is because we're basically pushing back the vector by one.
        for (auto&[cciID, previousIndex]: cciIDtoIndexMap) {
            if (previousIndex > index) {
                previousIndex++;
            }
        }


    }

    /* Updates the cciIDtoIndexMap based on the oldIndex and newIndex. Figured out after some maths.
     * Useful for single move operations. Much faster than reconstructing the entire map
     * Best for when CurrentCueInfoVector::move has been called and the CCI index map needs to updated accordingly.
     * Only functions when ONE element in the vector has been moved.
     */
    void updateCCIIndexMap(size_t oldIndex, size_t newIndex) {
        if (oldIndex == newIndex) { return; }
        // When moving, the affected range of indexes will be between oldIndex and newIndex, inclusive.
        size_t firstIndexChanged = std::min(oldIndex, newIndex);
        size_t lastIndexChanged = std::max(oldIndex, newIndex);
        bool increment = oldIndex > newIndex; // If the oldIndex is larger, then ++, otherwise --
        if (oldIndex >= size || newIndex >= size) {
            jassertfalse; // Index out of range
            return;
        }


        // Remove the old index from the map
        // No `const auto&` it as we need to modify the map's elements
        for (auto&[cciID, previousIndex] : cciIDtoIndexMap) {
            // If the index matches the old index, changed it to the new index.
            if (previousIndex == oldIndex) {
                previousIndex = newIndex;
                continue;
            }
            if (previousIndex < firstIndexChanged || previousIndex > lastIndexChanged) { continue; }

            if (increment)
                previousIndex++;
            else
                previousIndex--;
        }
    }

    // Reconstruct the actionID: CCI Internal ID map
    void reconstructActionCCIMap() {
        std::unordered_map<std::string, std::string> tempMap;
        for (auto& cci: vector) {
            for (auto& action: cci.actions) {
                tempMap[action.ID] = cci.getInternalID();
            }
        }
        actionIDtoCCIInternalIDMap = tempMap;
    }

    // Add an actionID to the action-CCI map by using a CCI
    void addToActionCCIMap(const CurrentCueInfo& cci) {
        std::string cciInternalID = cci.getInternalID();
        for (const auto& action: cci.actions) {
            actionIDtoCCIInternalIDMap[action.ID] = cciInternalID;
        }
    }

    // Add an actionID to the action-CCI map by using a CueOSCAction and its parent CCI
    void addToActionCCIMap(const CueOSCAction& action, const CurrentCueInfo& parentCCI) {
        actionIDtoCCIInternalIDMap[action.ID] = parentCCI.getInternalID();
    }

    // Add an actionID to the action-CCI map by using a CueOSCAction and its parent CCI ID
    void addToActionCCIMap(const CueOSCAction& action, const std::string &cciID) {
        actionIDtoCCIInternalIDMap[action.ID] = cciID;
    }

    // Add an actionID to the action-CCI map by using a CueOSCAction. Automatically tries to detect action's parent CCI
    // but expects the CCI containing the CueOSCAction to already be added to this CCIVector. Slower than using the
    // other overloads for this function.
    // WARNING: Will jassertfalse and return false if the action is not already in one of the CCIs in the CCIVector.
    // However, this function will NOT add action.ID to the map. It is only guaranteed that action.ID is added to the
    // map when true is returned
    bool addToActionCCIMap(const CueOSCAction& action) {
        // Find the parent CCI
        for (const auto& cci: vector) {
            for (const auto& possibleAction: cci.actions ) {
                if (possibleAction.ID == action.ID) {
                    actionIDtoCCIInternalIDMap[action.ID] = cci.getInternalID();
                    return true;
                }
            }
        }
        jassertfalse; // Nothing found! Not adding the Action
        return false;
    }


    // Remove items from the action-CCI map by using a CueOSCAction
    void removeFromActionCCIMap(const CueOSCAction& action) {
        actionIDtoCCIInternalIDMap.erase(action.ID);
    }

    // Remove items from the action-CCI map by using a CCI
    void removeFromActionCCIMap(const CurrentCueInfo& cci) {
        for (const auto& action : cci.actions) {
            actionIDtoCCIInternalIDMap.erase(action.ID);
        }
    }
};




struct ActiveShowOptions {
    String showName; // Should be max-len of 24. Not Strict.
    String showDescription;
    String currentCueID;
    std::string currentCueInternalID;
    size_t currentCueIndex; // Zero-indexed (0 --> n)
    bool currentCuePlaying;
    size_t numberOfCueItems; // NOT Zero-indexed


    // Modifies currentCueID, currentCueIndex, currentCuePlaying and numberOfCueItems from cciVector.
    // If useIndex is out of range, it will default to 0, not the last element of cciVector.
    void loadCueValuesFromCCIVector(CurrentCueInfoVector& cciVector, size_t useIndex = 0) {
        auto cciVSize = cciVector.getSize();
        if (cciVSize == 0) {
            currentCueIndex = 0;
            currentCuePlaying = false;
            numberOfCueItems = 0;
            currentCueID = "";
            currentCueInternalID = "";
            return;
        }
        if (useIndex >= cciVSize) {
            useIndex = 0;
        }
        currentCueIndex = useIndex;
        auto& cci = cciVector.getCurrentCueInfoByIndex(useIndex);
        currentCuePlaying = cci.currentlyPlaying;
        currentCueID = cci.id;
        numberOfCueItems = cciVSize;
        currentCueInternalID = cci.getInternalID();
    }
};


/* Structure to hold the output of the validation functions.
 * errorMessage is a blank String when isValid is true
 */
struct ValidatorOutput {
    bool isValid;
    String errorMessage;
};


ValidatorOutput isValidIPv4(const String &ip);

ValidatorOutput isValidPort(const String &port);

ValidatorOutput isValidDeviceName(const String &deviceName);


std::pair<bool, double> getDoubleValueFromTextEditor(String text);


class XM32 {
public:
    /* As the XM32 uses approximations for the 20*log(v2/v1) formula for dB, we will use the same.*/
    static double dbToDouble(double db);

    /* The inverse of the dbToDouble function. As the XM32 uses approximations for the 20*log(v2/v1) formula for dB, we will use the same.
     * The maximum dB value is +10, the minimum is -90 (i.e., the same as -inf).
     */
    static double doubleToDb(double v);

    /* The frequency map is a logarithmic scale of frequencies from 20Hz to 20kHz.
     * The function will round the input frequency to the nearest frequency in the map.
     * If the input frequency is outside the range of the map, it will return the closest frequency in the map.
     *
     * Declared in header file because the linker is a b*tch (and doesn't like template typenames)
     */
    template<typename T>
    static T roundToNearest(T in, const std::set<T> &set) {
        auto lower = set.lower_bound(in);

        if (lower == set.begin()) return *lower;
        if (lower == set.end()) return *std::prev(lower);

        T lowerValue = *std::prev(lower);
        T upperValue = *lower;

        return (std::abs(lowerValue - in) <= std::abs(upperValue - in)) ? lowerValue : upperValue;
    }
};

/* Function to map a percentage (0-1) to a value between minVal and maxVal using different algorithms
When percentage is 0, it returns minVal.
When percentage is 1, it returns maxVal.
When percentage is between 0 and 1, it uses the specified algorithm to interpolate between minVal and maxVal.
If an error occurs (e.g., percentage is out of range), it asserts false and fallbacks by returning minVal.
*/
double inferValueFromMinMaxAndPercentage(
    double minVal, double maxVal, double percentage, ParamType algorithm = ParamType::LINF);


/* Basically, inverse of inferValueFromMinMaxAndPercentage. (i.e., normalises a value).
 * When value is out of range, fallbacks to returning 0.0 */
double inferPercentageFromMinMaxAndValue(
    double minVal, double maxVal, double value, ParamType algorithm = ParamType::LINF);


// Generates a NormalisableRange for a logarithmic slider. From https://forum.juce.com/t/logarithmic-slider-for-frequencies-iir-hpf/37569/10
static inline NormalisableRange<double> getNormalisableRangeExp(double min, double max) {
    jassert(min > 0.0);
    jassert(max > 0.0);
    jassert(min < max);

    double logmin = std::log(min);
    double logmax = std::log(max);
    double logrange = (logmax - logmin);

    jassert(logrange > 0.0);

    return {
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
        }};
}


const inline NormalisableRange<double> LEVEL_161_NORMALISABLE_RANGE(
    -90.0, 10.0,
    [](double start, double end, double normalized) {
        normalized = std::max(0.0, std::min(1.0, normalized)); // Ensure normalized is between 0 and 1
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
        normalized = std::max(0.0, std::min(1.0, normalized)); // Ensure normalized is between 0 and 1
        return XM32::doubleToDb(normalized);
    },
    [](double start, double end, double value) {
        value = std::max(start, std::min(end, value)); // Ensure value is within the range
        return XM32::dbToDouble(value); // Return the corresponding dB value
    },
    [](double start, double end, double value) {
        return std::max(start, std::min(end, value));
    });
