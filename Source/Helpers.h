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
    CueOSCAction(bool exitThread): oat(_EXIT_THREAD), oscAddress("/"), oatCommandOSCArgumentTemplate(_nullNonConstOSCMessageArguments), arguments(_nullNonConstValueStorerArray) {
    }

    // For OAT_COMMAND, the arguments are used to fill in the OSC Message.
    CueOSCAction(OSCAddressPattern oscAddress, std::vector<OSCMessageArguments>& argumentTemplates, ValueStorerArray arguments):
    oat(OAT_COMMAND), arguments(arguments), oatCommandOSCArgumentTemplate(argumentTemplates),
                                                                     oscAddress(oscAddress) {
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

    // For OAT_FADE, the fadeTime is used to determine the fade time in seconds.
    CueOSCAction(OSCAddressPattern oscAddress, float fadeTime, NonIter oscArgumentTemplate, ValueStorer startValue,
                 ValueStorer endValue): oscAddress(oscAddress), oat(OAT_FADE), fadeTime(fadeTime),
                                        oscArgumentTemplate(oscArgumentTemplate),
                                        startValue(startValue), endValue(endValue), arguments(_nullNonConstValueStorerArray),
    oatCommandOSCArgumentTemplate(_nullNonConstOSCMessageArguments) {
        _checks();
    }

    OSCActionType oat;
    OSCAddressPattern oscAddress;

    // For OAT_COMMAND
    std::vector<OSCMessageArguments>& oatCommandOSCArgumentTemplate;
    // We're not using a pointer to a ValueStorerArray because the values should be stored in the CueOSCAction as they
    // should be unique to this action. The vector above on the other hand can be shared between multiple actions.
    ValueStorerArray arguments; // The arguments to send with the OSC Message, only used for OAT_COMMAND


    // For OAT_FADE
    float fadeTime{0.f}; // The fade time in seconds, only used for OAT_FADE
    NonIter oscArgumentTemplate = _nullNonIter; // Used to find algorithm and type for parameter
    ValueStorer startValue;
    ValueStorer endValue;
};


struct CurrentCueInfo {
    String id; // When ID is empty, it is implied the CCI is not valid. This can be used when no CCIs are in a CCI Vector, so a blank CCI can be used.
    String name;
    String description;
    std::vector<CueOSCAction> actions;
    bool currentlyPlaying {false};

    /*
    ValueStorerArray finalOSCValues; // Later unpacked into positional arguments for OSCMessage constructor,
    OSCMessage _constructedMessageCache{"/"};
    bool _constructedMessageCacheIsValid{false};


    // WARNING: A blank OSC Message Address String WILL RAISE AN EXCEPTION! PROCEED AT YOUR OWN RISK
    CurrentCueInfo(String id, String name, String description, String finalOSCAddress = "",
                   ValueStorerArray finalOSCValues = {}): id(id), name(name), description(description),
                                                          finalOSCAddress(finalOSCAddress),
                                                          finalOSCValues(finalOSCValues) {
    }


    // When finalOSCAddress and finalOSCValues are set and ready to be constructed into a valid OSCMessage,
    // this function can be used.
    // Use ignoreAndReconstructCache when variables in CurrentCueInfo have changed.
    OSCMessage constructMessage(bool ignoreAndReconstructCache = false);
    */

    CurrentCueInfo(const String &id, const String &name, const String &description,
                   const std::vector<CueOSCAction>& actions): id(id), name(name), description(description),
                                                       actions(actions) {
    }

    // Used for blank CCI (i.e., invalid CCI)
    CurrentCueInfo(): id(""), name(""), description(""), actions({}) {
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
    void loadCueValuesFromCCIVector(const std::vector<CurrentCueInfo>& cciVector, unsigned int useIndex = 0) {
        auto cciVSize = cciVector.size();
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
        currentCuePlaying = cciVector[useIndex].currentlyPlaying;
        currentCueID = cciVector[useIndex].id;
        numberOfCueItems = cciVSize;
    }
};


enum Units {
    HERTZ,
    DB,
    NONE
};

enum ShowCommand {
    SHOW_PLAY,
    SHOW_STOP,
    SHOW_NEXT_CUE,
    SHOW_PREVIOUS_CUE,
    SHOW_NAME_CHANGE,
    FULL_SHOW_RESET, // Reset all UI and local variables

    CURRENT_CUE_ID_CHANGE
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
