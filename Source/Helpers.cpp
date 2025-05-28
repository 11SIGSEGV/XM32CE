/*
  ==============================================================================

    Helpers.cpp
    Created: 10 May 2025 4:19:51pm
    Author:  anony

  ==============================================================================
*/

#include "Helpers.h"


ValidatorOutput isValidIPv4(const String &ip) {
    auto parts = StringArray::fromTokens(ip, ".", "");

    if (parts.size() != 4)
        return ValidatorOutput {false, "IPv4 Address Requires 4 Octets"};

    for (auto &part: parts) {
        // Check if the part is all digits
        if (part.isEmpty())
            return ValidatorOutput { false , "IPv4 Address requires all octets to be filled" };
        if (!part.containsOnly("0123456789"))
            return ValidatorOutput { false , "IPv4 Address only accepts digits" };

        // No leading zeros allowed (except for "0")
        if (part.length() > 1 && part.startsWithChar('0'))
            return ValidatorOutput { false, "IPv4 Address does not allow leading zeros" };

        int value = part.getIntValue();
        if (value < 0 || value > 255)
            return { false, "Each octet must be between 0 and 255" };
    }
    return {true , ""};
}


ValidatorOutput isValidPort(const String &port) {
    if (port.isEmpty())
        return ValidatorOutput { false, "Port cannot be empty" };

    if (!port.containsOnly("0123456789"))
        return ValidatorOutput { false, "Port only allows digits" };

    int portValue = port.getIntValue();
    if (portValue < 0 || portValue > 65535)
        return ValidatorOutput { false, "Port must be between 0 and 65535" };

    return ValidatorOutput { true, "" };
}


/* Requirements are:
 * 1. Device name must be between 1 and 30 characters
 * 2. Device name must not contain any special characters
*/
ValidatorOutput isValidDeviceName(const String &deviceName) {
    if (deviceName.isEmpty())
        return ValidatorOutput { false, "Device name cannot be empty" };
    if (deviceName.length() > 30)
        return { false, "Device name must be less than 30 characters" };
    if (!deviceName.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ "))
        return { false, "Device name only allows alphanumeric characters and underscores" };
    return ValidatorOutput { true, "" };
}



float XM32::floatToDb(float v) {
    float db;
    if (v >= 0.5)
        db = v * 40.f - 30.f;
    else if (v >= 0.25)
        db = v * 80.f - 50.f;
    else if (v >= 0.0625)
        db = v * 160.f - 70.f;
    else if (v >= 0.0)
        db = v * 480.f - 90.f;
    else
        throw std::out_of_range("floatToDb: v value out of range");
    return db;
}

float XM32::dbToFloat(float db) {
    float v;
    if (db < -60.f)
        v = ( db+90.f ) / 480.f;
    else if (db < -30.f)
        v = (db+70.f) / 160.f;
    else if (db < -10.f)
        v = (db+50.f) / 80.f;
    else if (db <= 10.f)
        v = (db+30.f) / 40.f;
    else
        throw std::out_of_range("dbToFloat: db value out of range");
    // Optional, round value to X32 known value
    v = roundf(v*1023)/1023;
    return v;
}


const double inferValueFromMinMaxAndPercentage(double minVal, double maxVal, double percentage, ParamType algorithm) {
    if (percentage == 0.0) {
        // If default is 0, then we can assume that the value is just the min.
        return minVal;
    } if (percentage == 1.0) {
        // If default is 1, then we can assume that the value is just the max.
        return maxVal;
    } if (percentage > 0.0 && percentage < 1.0) {
        switch (algorithm) {
            case ParamType::LINF:
                // Linear interpolation
                return jmap(percentage, minVal, maxVal);
            case ParamType::LOGF:
                // Logarithmic interpolation
                return mapToLog10(percentage, minVal, maxVal);
            case ParamType::LEVEL_161: {
                // Level 161 interpolation - for XM32
                // First, find the level equivalent of the percentage
                if (minVal != -90.0 || maxVal != 10.0) {
                    jassertfalse; // Level 161 is only defined for -90 to 10 dB
                }
                float targetLevel = jmap(percentage, minVal, maxVal);
                // Then, find the closest level 161 value.
                return XM32::roundToNearest(targetLevel, levelValues_161);
            }
            case ParamType::LEVEL_1024: {
                // Level 1024 interpolation - for XM32. Much simpler! This uses Music Tribe's approximation for float to dB log scale
                // NOTE: Here, if minVal and maxVal are not -90 and 10, respectively, it does not matter. It will still assume -90 and 10.
                if (minVal != -90.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                } if (maxVal != 10.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                return XM32::floatToDb(percentage);
            }
            default:
                jassertfalse;
                return minVal; // Fallback to min value if algorithm is not recognised
        }
    }
    jassertfalse; // Value is out of range from 0-1
    return minVal;
}


const double inferPercentageFromMinMaxAndValue(double minVal, double maxVal, double value, ParamType algorithm) {
    if (value == minVal) {
        // If default value is the minimum, the normalised percentage is 0.
        return 0.0;
    } if (value == maxVal) {
        // If default value is the maximum, the normalised percentage is 1.
        return 1.0;
    } if (value > minVal && value < maxVal) {
        switch (algorithm) {
            case ParamType::LINF:
                // Linear interpolation
                return (value-minVal)/(maxVal-minVal);
            case ParamType::LOGF:
                // Logarithmic interpolation
                return mapFromLog10(value, minVal, maxVal);
            case ParamType::LEVEL_161: {
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
            case ParamType::LEVEL_1024: {
                // Level 1024 interpolation - for XM32.
                // NOTE: Here, if minVal and maxVal are not -90 and 10, respectively, it does not matter. It will still assume -90 and 10.
                if (minVal != -90.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                } if (maxVal != 10.0) {
                    jassertfalse; // Level 1024 is only defined for -90 to 10 dB
                }
                if (value < -90.0 || value > 10.0) {
                    jassertfalse; // Value is out of range for level 1024
                    return 0.0; // Fallback to 0.0 if value is out of range. We don't want to reach XM32::dbToFloat with an out of range value, as it will throw an exception
                }
                return XM32::dbToFloat(value);
            }
            default:
                jassertfalse;
                return 0.0; // Fallback to min value if algorithm is not recognised
        }
    }
    jassertfalse; // Value is out of range from minVal to maxVal
    return 0.0;
}


