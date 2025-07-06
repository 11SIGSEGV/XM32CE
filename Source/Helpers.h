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
    FULL_SHOW_RESET, // Reset all UI and local variables

    CURRENT_CUE_ID_CHANGE,
    CUE_INDEXS_CHANGED,
    _BROADCAST_TO_ALL_CUE_STOPPED
};


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


inline UUIDGenerator uuidGen;


namespace UICfg {
    const String DEFAULT_SANS_SERIF_FONT_NAME = Font::getDefaultSansSerifFontName();
    const String DEFAULT_SERIF_FONT_NAME = Font::getDefaultSerifFontName();
    const String DEFAULT_MONOSPACE_FONT_NAME = Font::getDefaultMonospacedFontName();
    constexpr int DEFAULT_TEXT_HEIGHT = 100;
    const Font DEFAULT_FONT = FontOptions(DEFAULT_SANS_SERIF_FONT_NAME, static_cast<float>(DEFAULT_TEXT_HEIGHT),
                                          Font::plain);
    const Font DEFAULT_MONOSPACE_FONT = FontOptions(DEFAULT_MONOSPACE_FONT_NAME,
                                                    static_cast<float>(DEFAULT_TEXT_HEIGHT), Font::plain);

    const Colour BG_COLOUR(34, 34, 34);
    const Colour LIGHT_BG_COLOUR(100, 100, 100);
    const Colour TEXT_COLOUR(238, 238, 238);
    const Colour TEXT_ACCENTED_COLOUR(212, 235, 255);
    const Colour TEXT_COLOUR_DARK(100, 100, 100);
    const Colour STRONG_BORDER_COLOUR(212, 212, 212);
    const Colour HEADER_BG_COLOUR(25, 23, 43);
    const Colour HEADER_BTN_DISABLED_BG_COLOUR(15, 14, 27);

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
    const Colour ROTARY_ENABLED_COLOUR(242u, 194u, 63u);
    const Colour ROTARY_DISABLED_COLOUR(87u, 76u, 48u);

    constexpr int ROUND_TO_WHEN_IN_DOUBT = 2; // Round to 2 decimal places when in doubt (e.g., no unit for the value)


    // For UI element sizes use relative sizes
    constexpr float STD_PADDING = 1.f / 30.f;

    // Look and feel Configs
    const Colour TEXT_EDITOR_BG_COLOUR(0.f, 0.f, 0.f, 0.f);
}


namespace IconID {
    constexpr int DOWN_ARROW = 1;
    constexpr int UP_ARROW = 2;
    constexpr int OCTAGON = 3;
    constexpr int STOP = OCTAGON;
    constexpr int PLAY = 4;
}


inline const std::unordered_map<int, std::string> ICON_FILE_MAP = {
    {IconID::DOWN_ARROW, "down.png"},
    {IconID::UP_ARROW, "up.png"},
    {IconID::PLAY, "play.png"},
    {IconID::OCTAGON, "octagon.png"},
};


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
Image getIconImageFile(int iconID);


// Abbreviated as OAT
enum OSCActionType {
    OAT_COMMAND,
    OAT_FADE,
    _EXIT_THREAD
};



struct CueOSCAction {
    CueOSCAction(bool exitThread): oat(_EXIT_THREAD), oscAddress("/") {
    }

    // For OAT_COMMAND, the arguments are used to fill in the OSC Message.
    CueOSCAction(OSCAddressPattern oscAddress, std::vector<OSCMessageArguments> argumentTemplates, ValueStorerArray arguments):
    oat(OAT_COMMAND), arguments(arguments), oatCommandOSCArgumentTemplate(argumentTemplates),
                                                                     oscAddress(oscAddress), ID(uuidGen.generate()) {
    }
    // A lot of OAT_COMMANDs only have one OSCMessageArgument, so let's allow a single argument template
    CueOSCAction(OSCAddressPattern oscAddress, OSCMessageArguments argumentTemplate, ValueStorer argument): oat(OAT_COMMAND),
        oatCommandOSCArgumentTemplate({argumentTemplate}), arguments({std::move(argument)}), oscAddress(oscAddress), ID(uuidGen.generate()) {
    }


    // For OAT_FADE, the fadeTime is used to determine the fade time in seconds.
    CueOSCAction(OSCAddressPattern oscAddress, float fadeTime, NonIter oscArgumentTemplate, ValueStorer startValue,
                 ValueStorer endValue): oscAddress(oscAddress), oat(OAT_FADE), fadeTime(fadeTime),
                                        oscArgumentTemplate(oscArgumentTemplate),
                                        startValue(startValue), endValue(endValue), ID(uuidGen.generate()) {
        _checks();
    }


    void _checks() {
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

        if (oat == OAT_COMMAND) {
            if (arguments.size() != oatCommandOSCArgumentTemplate.size()) {
                jassertfalse; // The number of arguments must match the number of argument templates
            }
        }
    }

    const std::string ID;

    OSCActionType oat;
    OSCAddressPattern oscAddress;

    // For OAT_COMMAND
    // Let's sacrifice the extra memory for the sake of reliability and stability.
    std::vector<OSCMessageArguments> oatCommandOSCArgumentTemplate;
    ValueStorerArray arguments; // The arguments to send with the OSC Message, only used for OAT_COMMAND


    // For OAT_FADE
    float fadeTime{0.f}; // The fade time in seconds, only used for OAT_FADE
    NonIter oscArgumentTemplate = _nullNonIter; // Used to find algorithm and type for parameter
    ValueStorer startValue;
    ValueStorer endValue;
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
    CurrentCueInfo(): id(""), name(""), description(""), actions({}), INTERNAL_ID("") {
    }

    bool isInvalid() {
        return INTERNAL_ID.empty();
    }

    std::string getInternalID() const {
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
        reconstructCCIToIndexMap();
        size = vector.size();
    }


    // CurrentCueInfoVector(const std::vector<CurrentCueInfo>& cciVector): vector(cciVector), size(cciVector.size()) {}


    explicit CurrentCueInfoVector(const std::vector<CurrentCueInfo>& cciVector): vector(cciVector), size(cciVector.size()) {
        reconstructCCIToIndexMap();
    }


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


    // Just passes index to getCurrentCueInfoByIndex().
    CurrentCueInfo& operator[](size_t index) {
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

    // Based off https://stackoverflow.com/a/57399634/16571234
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
        if (it == cciIDtoIndexMap.end()) {
            return sizeTLimit;
        }
        return it->second;
    }

    static constexpr size_t sizeTLimit = static_cast<size_t>(-1); // We use this to indicate an invalid index. But... if by some miracle the vector reaches this size, we must not use it.
private:
    std::unordered_map<std::string, size_t> cciIDtoIndexMap; // Maps CCI internal ID to index in the vector

    size_t size = 0;
    std::vector<ShowCommandListener*> listeners;

    void uponCueAdd(const CurrentCueInfo& cciAdded) {
        size++;
        if (size == sizeTLimit) {
            jassertfalse; // ...how did you get so many elements inside the vector? The size_t limit is used in this struct to represent an invalid index, so this may break the "static_cast<size_t>(-1)"th element.
        }
        addCCIToIndexMap(cciAdded.getInternalID());
        _notifyListeners(CUES_ADDED);
    }

    void uponCuesDeleted() {
        size = vector.size(); // Yeah... we could go size -= internalIDsOfCCIsDeleted.size(), but like... at that rate just recalculate the size of the vector.
        // yes... we can calculate the new cciIDtoIndexMap... but depending on how many CCIs were deleted... the performance really does not matter in the end.
        // So we'll just reconstruct it.
        reconstructCCIToIndexMap();
        _notifyListeners(CUES_DELETED);
    }

    void _notifyListeners(ShowCommand command) {
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
        cciIDtoIndexMap.clear();
        for (size_t i = 0; i < size; i++) {
            cciIDtoIndexMap[vector[i].getInternalID()] = i;
        }
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
        size_t index = manuallyFindIndexOfCCIByInternalID(cciInternalID);
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
        for (auto& it: cciIDtoIndexMap) {
            if (it.second > index) {
                it.second++;
            }
        }


    }

    /* Updates the cciIDtoIndexMap based on the oldIndex and newIndex. Figured out after some maths.
     * Useful for single move operations. Much faster than reconstructing the entire map
     * Best for when CurrentCueInfoVector::move has been called and the CCI index map needs to updated accordingly.
     * Only functions when ONE element in the vector has been moved.
     */
    void updateCCIIndexMap(size_t oldIndex, size_t newIndex) {
        // When moving, the affected range of indexes will be between oldIndex and newIndex, inclusive.
        size_t firstIndexChanged = std::min(oldIndex, newIndex);
        size_t lastIndexChanged = std::max(oldIndex, newIndex);
        if (firstIndexChanged >= size || oldIndex >= size || newIndex >= size) {
            jassertfalse; // Index out of range
            return;
        }

        // Remove the old index from the map
        // No `const auto&` it as we need to modify the map's elements
        for (auto& it : cciIDtoIndexMap) {
            // If the index matches the old index, changed it to the new index.
            if (it.second == oldIndex) {
                it.second = newIndex;
                continue;
            }
            if (it.second < firstIndexChanged || it.second > lastIndexChanged) { continue; }

            // Simply increment one to the index. This will reflect the movement of ONE element.
            it.second++;
        }
    }
};





struct ActiveShowOptions {
    String showName; // Should be max-len of 24. Not Strict.
    String showDescription;
    String currentCueID;
    int currentCueIndex; // Zero-indexed (0 --> n)
    bool currentCuePlaying;
    int numberOfCueItems; // NOT Zero-indexed


    // Modifies currentCueID, currentCueIndex, currentCuePlaying and numberOfCueItems from cciVector.
    // If useIndex is out of range, it will default to 0, not the last element of cciVector.
    void loadCueValuesFromCCIVector(CurrentCueInfoVector& cciVector, unsigned int useIndex = 0) {
        auto cciVSize = cciVector.getSize();
        if (cciVSize == 0) {
            currentCueIndex = 0;
            currentCuePlaying = false;
            numberOfCueItems = 0;
            currentCueID = "";
            return;
        }
        if (useIndex >= cciVSize) {
            useIndex = 0;
        }
        currentCueIndex = useIndex;
        auto cci = cciVector.getCurrentCueInfoByIndex(useIndex);
        currentCuePlaying = cci.currentlyPlaying;
        currentCueID = cci.id;
        numberOfCueItems = cciVSize;
    }
};


enum Units {
    HERTZ,
    DB,
    NONE
};

const std::unordered_map<Units, int> ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT = {
    {HERTZ, 2},
    {NONE, UICfg::ROUND_TO_WHEN_IN_DOUBT},
    {DB, 2}
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
    static float dbToFloat(float db);

    /* The inverse of the dbToFloat function. As the XM32 uses approximations for the 20*log(v2/v1) formula for dB, we will use the same.
     * The maximum dB value is +10, the minimum is -90 (i.e., the same as -inf).
     */
    static float floatToDb(float v);

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
const double inferValueFromMinMaxAndPercentage(
    double minVal, double maxVal, double percentage, ParamType algorithm = ParamType::LINF);


/* Basically, inverse of inferValueFromMinMaxAndPercentage. (i.e., normalises a value).
 * When value is out of range, fallbacks to returning 0.0 */
const double inferPercentageFromMinMaxAndValue(
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
        return XM32::floatToDb(normalized);
    },
    [](double start, double end, double value) {
        value = std::max(start, std::min(end, value)); // Ensure value is within the range
        return XM32::dbToFloat(value); // Return the corresponding dB value
    },
    [](double start, double end, double value) {
        return std::max(start, std::min(end, value));
    });
