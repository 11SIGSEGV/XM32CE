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
using namespace juce;

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
     */
    static float roundToNearestFrequency(float inputFreq, const std::vector<float> &frequencyMap = logScaleFreq_201);
};
