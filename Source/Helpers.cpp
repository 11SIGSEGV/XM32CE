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

