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
    const EnumParam_PathToArgumentPair SOURCE = {
        {"/ch/", _channelNum, "/config/source"},
        {EnumParam("chSource", "Channel Source", "The input source for this channel",
            {"OFF", "In01", "In02", "In03", "In04", "In05", "In06", "In07", "In08", "In09", "In10", "In11", "In12", "In13", "In14", "In15", "In16", "In17", "In18", "In19", "In20", "In21", "In22", "In23", "In24", "In25", "In26", "In27", "In28", "In29", "In30", "In31", "In32", "Aux 1", "Aux 2", "Aux 3", "Aux 4", "Aux 5", "Aux 6", "USB L", "USB R", "Fx 1L", "Fx 1R", "Fx 2L", "Fx 2R", "Fx 3L", "Fx 3R", "Fx 4L", "Fx 4R", "Bus 01", "Bus 02", "Bus 03", "Bus 04", "Bus 05", "Bus 06", "Bus 07", "Bus 08", "Bus 09", "Bus 10", "Bus 11", "Bus 12", "Bus 13", "Bus 14", "Bus 15", "Bus 16"})
        }
    };
    const EnumParam_PathToArgumentPair DELAY_ON = {
        {"/ch/", _channelNum, "/delay/on"},
        {EnumParam("chDelayOn", "Channel Delay On", "Turns the delay on or off for the channel", {"OFF", "ON"})}
    };
    const NonIter_PathToArgumentPair DELAY_TIME = {
        {"/ch/", _channelNum, "/delay/time"},
    {NonIter("chDelayTime", "Channel Delay Time", "The amount of delay to apply to the channel's input source in milliseconds", 0.3f, LINF, 0.3f, 500.f)}
    };
    // TODO: Some more to add!
    const NonIter_PathToArgumentPair FADER = {
        {"/ch/", _channelNum, "/mix/fader"},
        {NonIter("chFader", "Channel Fader", "The fader value for the channel", -90.f, LEVEL_1024, -90.f, 10.f)}
    };

}
