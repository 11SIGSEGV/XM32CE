/*
  ==============================================================================

    Helpers.cpp
    Created: 10 May 2025 4:19:51pm
    Author:  anony

  ==============================================================================
*/

#include "Helpers.h"


bool isValidIPv4(const String &ip) {
    auto parts = StringArray::fromTokens(ip, ".", "");

    if (parts.size() != 4)
        return false;

    for (auto &part: parts) {
        // Check if the part is all digits
        if (part.isEmpty())
            return false;
        if (!part.containsOnly("0123456789"))
            return false;

        // No leading zeros allowed (except for "0")
        if (part.length() > 1 && part.startsWithChar('0'))
            return false;

        int value = part.getIntValue();
        if (value < 0 || value > 255)
            return false;
    }
    return true;
}
