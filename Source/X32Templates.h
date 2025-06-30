/*
  ==============================================================================

    X32Templates.h
    Created: 29 Jun 2025 3:07:26pm
    Author:  anony
    Welcome to this shitshow of a file! This file will contain HUNDREDS of templates for each and every supported
    X/M32 OSC Command! I hope you don't have to recompile this file often ;)

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "XM32Maps.h"
#include "Helpers.h"

namespace Channel {
    const NonIter _channelNum = {"chNum", "Channel Number", "The number of the channel to send OSC commands to", 1, 1, 32};
    const NonIter_PathToArgumentPair NAME = {
        {"/ch/", _channelNum, "/config/name"},
        {NonIter("chName", "Channel Name", "The name of the channel", "", 0, 12)}
    };
    const NonIter_PathToArgumentPair ICON = {
        {"/ch/", _channelNum, "/config/icon"},
        {NonIter("chIcon", "Channel Icon", "The index for the channel's icon", 1, INT, 1, 74)}
    };
    const EnumParam_PathToArgumentPair COLOUR = {
        {"/ch/", _channelNum, "/config/color"},
        {EnumParam("chColour", "Channel Colour", "The index representing the colour of the channel",
            {"OFF", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE", "OFFi", "REDi", "GREENi", "YELLOWi", "BLUEi", "MAGENTAi", "CYANi", "WHITEi"})}
    };
    // TODO: Some more to add!
    const NonIter_PathToArgumentPair FADER = {
        {"/ch/", _channelNum, "/mix/fader"},
        {NonIter("chFader", "Channel Fader", "The fader value for the channel", -90.f, LEVEL_1024, -90.f, 10.f)}
    };

}
