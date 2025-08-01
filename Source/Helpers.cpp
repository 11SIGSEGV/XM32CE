/*
  ==============================================================================

    Helpers.cpp
    Created: 10 May 2025 4:19:51pm
    Author:  anony

  ==============================================================================
*/

#include "Helpers.h"


void ShowCommandListener::cueCommandOccurred(ShowCommand, std::string cciInternalID, size_t cciCurrentIndex) {}


String formatValueUsingUnit(const Units unit, double value) {
    switch (unit) {
        case DB: {
            if (value <= -90.0) {
                return "-inf";
            }
            auto val = roundTo(value, ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT.at(DB));
            if (val > 0) {
                return "+" + String(val) + "dB";
            }
            return String(val) + "dB";
        }
        case HERTZ: {
            bool useKhz = value >= 1000.0;
            double val = roundTo((useKhz ? value / 1000.0: value), ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT.at(HERTZ));
            return String(val) + (useKhz ? "kHz": "Hz");
        }
        case MS: {
            auto val = roundTo(value, ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT.at(MS));
            return String(val) + "ms";
        }
        case NONE: {
            return String(value);
        }
    }
    jassertfalse; // How... how did you end.. bu.. but... ?????
    return "";
}

Image getIconImageFile(int iconID, bool getDisabledVersion) {
    if (ICON_FILE_MAP.find(iconID) != ICON_FILE_MAP.end()) {
        auto file = FileInfo::ICONS_DIRECTORY.getChildFile(ICON_FILE_MAP.at(iconID));
        if (!file.existsAsFile()) {
            jassertfalse; // Icon file does not exist
            return {};
        }
        Image fileImage = PNGImageFormat::loadFrom(file);
        // Fun fact, the implementation for isNull is just !isValid()
        if (fileImage.isNull()) {
            jassertfalse; // Failed to load image from file
            return {};
        }
        if (getDisabledVersion) {
            fileImage.desaturate();
            fileImage.multiplyAllAlphas(0.5);
        }
        return fileImage; // Return the file if it exists
    } else {
        jassertfalse; // Icon ID not found in the map
        return {};
    }
}



/*
OSCMessage CurrentCueInfo::constructMessage(bool ignoreAndReconstructCache) {
    if (!ignoreAndReconstructCache && _constructedMessageCacheIsValid) {
        return _constructedMessageCache;
    }
    if (finalOSCAddress.isEmpty()) {
        jassertfalse; // Address needs to be finalised to construct message
        return {"/"};
    }
    try {
        OSCMessage msg { finalOSCAddress };
        for (const auto& valueStore: finalOSCValues) {
            switch (valueStore._meta_PARAMTYPE) {
                case INT:
                    msg.addInt32(valueStore.intValue);
                    break;
                case _GENERIC_FLOAT:
                    msg.addFloat32(valueStore.floatValue);
                    break;
                case STRING:
                    msg.addString(valueStore.stringValue);
                    break;
                default:
                    jassertfalse; // No other ParamTypes are valid for ValueStorer.
            }
        }
        _constructedMessageCache = msg;
        _constructedMessageCacheIsValid = true;
        return msg;
    } catch (const OSCFormatError& e) {
        jassertfalse;
        DBG(e.description + '\n' + e.what());
        return {"/"};
    }
}
*/



ValidatorOutput isValidIPv4(const String &ip) {
    auto parts = StringArray::fromTokens(ip, ".", "");

    if (parts.size() != 4)
        return ValidatorOutput{false, "IPv4 Address Requires 4 Octets"};

    for (auto &part: parts) {
        // Check if the part is all digits
        if (part.isEmpty())
            return ValidatorOutput{false, "IPv4 Address requires all octets to be filled"};
        if (!part.containsOnly("0123456789"))
            return ValidatorOutput{false, "IPv4 Address only accepts digits"};

        // No leading zeros allowed (except for "0")
        if (part.length() > 1 && part.startsWithChar('0'))
            return ValidatorOutput{false, "IPv4 Address does not allow leading zeros"};

        int value = part.getIntValue();
        if (value < 0 || value > 255)
            return {false, "Each octet must be between 0 and 255"};
    }
    return {true, ""};
}


ValidatorOutput isValidPort(const String &port) {
    if (port.isEmpty())
        return ValidatorOutput{false, "Port cannot be empty"};

    if (!port.containsOnly("0123456789"))
        return ValidatorOutput{false, "Port only allows digits"};

    int portValue = port.getIntValue();
    if (portValue < 0 || portValue > 65535)
        return ValidatorOutput{false, "Port must be between 0 and 65535"};

    return ValidatorOutput{true, ""};
}


/* Requirements are:
 * 1. Device name must be between 1 and 30 characters
 * 2. Device name must not contain any special characters
*/
ValidatorOutput isValidDeviceName(const String &deviceName) {
    if (deviceName.isEmpty())
        return ValidatorOutput{false, "Device name cannot be empty"};
    if (deviceName.length() > 30)
        return {false, "Device name must be less than 30 characters"};
    if (!deviceName.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ "))
        return {false, "Device name only allows alphanumeric characters and underscores"};
    return ValidatorOutput{true, ""};
}

// First value indicates if value is valid, second value is the parsed double value.
std::pair<bool, double> getDoubleValueFromTextEditor(String text) {
    // Remove whitespace, characters and convert to lowercase
    text = text.trim().toLowerCase().retainCharacters("1234567890.-");
    if (text.isEmpty()) {
        return {false, 0.0}; // Invalid value
    }
    return {true, text.getDoubleValue()};
}


double XM32::doubleToDb(double v) {
    double db;
    if (v >= 0.5)
        db = v * 40.0 - 30.0;
    else if (v >= 0.25)
        db = v * 80.0 - 50.0;
    else if (v >= 0.0625)
        db = v * 160.0 - 70.0;
    else if (v >= 0.0)
        db = v * 480.0 - 90.0;
    else {
        jassertfalse;
        return 0.0;
    }
    return db;
}

double XM32::dbToDouble(double db) {
    double v;
    if (db < -60.0)
        v = (db + 90.0) / 480.0;
    else if (db < -30.0)
        v = (db + 70.0) / 160.0;
    else if (db < -10.0)
        v = (db + 50.0) / 80.0;
    else if (db <= 10.0)
        v = (db + 30.0) / 40.0;
    else {
        jassertfalse;
        return 0.0;
    }
    // Optional, round value to X32 known value
    v = std::round(v * 1023) / 1023;
    return v;
}


// This function (and its oppsite) are both actually quite fast. They can be used for realtime applications
double inferValueFromMinMaxAndPercentage(double minVal, double maxVal, double percentage, const ParamType algorithm) {
    if (percentage == 0.0) {
        // If default is 0, then we can assume that the value is just the min.
        return minVal;
    }
    if (percentage == 1.0) {
        // If default is 1, then we can assume that the value is just the max.
        return maxVal;
    }
    if (percentage > 0.0 && percentage < 1.0) {
        switch (algorithm) {
            case LINF:
                // Linear interpolation
                return jmap(percentage, minVal, maxVal);
            case LOGF:
                // Logarithmic interpolation
                return mapToLog10(percentage, minVal, maxVal);
            case LEVEL_161: {
                // Level 161 interpolation - for XM32
                // First, find the level equivalent of the percentage
                if (minVal != -90.0 || maxVal != 10.0) {
                    jassertfalse; // Level 161 is only defined for -90 to 10 dB
                }
                float targetLevel = jmap(percentage, minVal, maxVal);
                // Then, find the closest level 161 value.
                return XM32::roundToNearest(targetLevel, levelValues_161);
            }
            case LEVEL_1024: {
                // Level 1024 interpolation - for XM32. Much simpler! This uses Music Tribe's approximation for float to dB log scale
                // NOTE: Here, if minVal and maxVal are not -90 and 10, respectively, it does not matter. It will still assume -90 and 10.
                if (minVal != -90.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                if (maxVal != 10.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                return XM32::doubleToDb(percentage);
            }
            case INT: {
                // Round to nearest.
                return std::round(jmap(percentage, minVal, maxVal));
            }
            default:
                jassertfalse;
                return minVal; // Fallback to min value if algorithm is not recognised
        }
    }
    jassertfalse; // Value is out of range from 0-1
    return minVal;
}


double inferPercentageFromMinMaxAndValue(const double minVal, const double maxVal, const double value, const ParamType algorithm) {
    if (value == minVal) {
        // If default value is the minimum, the normalised percentage is 0.
        return 0.0;
    }
    if (value == maxVal) {
        // If default value is the maximum, the normalised percentage is 1.
        return 1.0;
    }
    if (value > minVal && value < maxVal) {
        switch (algorithm) {
            case LINF:
                // Linear interpolation
                return (value - minVal) / (maxVal - minVal);
            case INT:
                // Same, but round
                return std::round((value - minVal) / (maxVal - minVal));
            case LOGF:
                // Logarithmic interpolation
                return mapFromLog10(value, minVal, maxVal);
            case LEVEL_161: {
                // Level 161 'un'-interpolation - for XM32
                if (minVal != -90.0 || maxVal != 10.0) {
                    jassertfalse; // Level 161 is only defined for -90 to 10 dB
                }
                auto val = levelToFloat_161.find(value);
                if (val == levelToFloat_161.end()) {
                    jassertfalse; // Value is not a valid level 161 value
                    return 0.0; // Fallback to 0.0 if value is not a valid level 161 value
                }
                return val->second;
            }
            case LEVEL_1024: {
                // Level 1024 interpolation - for XM32.
                // NOTE: Here, if minVal and maxVal are not -90 and 10, respectively, it does not matter. It will still assume -90 and 10.
                if (minVal != -90.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                if (maxVal != 10.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                if (value < -90.0 || value > 10.0) {
                    jassertfalse; // Value is out of range for level 1024
                    return 0.0;
                    // Fallback to 0.0 if value is out of range. We don't want to reach XM32::dbToDouble with an out of range value, as it will throw an exception
                }
                return XM32::dbToDouble(value);
            }
            default:
                jassertfalse;
                return 0.0; // Fallback to min value if algorithm is not recognised
        }
    }
    jassertfalse; // Value is out of range from minVal to maxVal
    return 0.0;
}
