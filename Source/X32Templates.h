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
    const NonIter _channelNum = {
        "chNum", "Channel Number", "The number of the channel to send OSC commands to", 1, 1, 32
    };
    const std::vector<std::string> OFF_ON = {"OFF", "ON"};
    const std::vector<std::string> SOURCES = {
        "OFF", "In01", "In02", "In03", "In04", "In05", "In06", "In07", "In08", "In09", "In10", "In11",
        "In12", "In13", "In14", "In15", "In16", "In17", "In18", "In19", "In20", "In21", "In22", "In23",
        "In24", "In25", "In26", "In27", "In28", "In29", "In30", "In31", "In32", "Aux 1", "Aux 2", "Aux 3",
        "Aux 4", "Aux 5", "Aux 6", "USB L", "USB R", "Fx 1L", "Fx 1R", "Fx 2L", "Fx 2R", "Fx 3L", "Fx 3R",
        "Fx 4L", "Fx 4R", "Bus 01", "Bus 02", "Bus 03", "Bus 04", "Bus 05", "Bus 06", "Bus 07", "Bus 08",
        "Bus 09", "Bus 10", "Bus 11", "Bus 12", "Bus 13", "Bus 14", "Bus 15", "Bus 16"
    };

    const NonIter_PathToArgumentPair NAME = {
        {"/ch/", _channelNum, "/config/name"},
        NonIter("chName", "Channel Name", "The name of the channel", "", 0, 12)
    };
    const NonIter_PathToArgumentPair ICON = {
        {"/ch/", _channelNum, "/config/icon"},
        NonIter("chIcon", "Channel Icon", "The index for the channel's icon", 1, INT, 1, 74)
    };
    const EnumParam_PathToArgumentPair COLOUR = {
        {"/ch/", _channelNum, "/config/color"},
        EnumParam("chColour", "Channel Colour", "The index representing the colour of the channel",
                  {
                      "OFF", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE", "OFFi", "REDi", "GREENi",
                      "YELLOWi", "BLUEi", "MAGENTAi", "CYANi", "WHITEi"
                  })
    };

    const EnumParam_PathToArgumentPair SOURCE = {
        {"/ch/", _channelNum, "/config/source"},
        EnumParam("chSource", "Channel Source", "The input source for this channel",
                  SOURCES)
    };
    const EnumParam_PathToArgumentPair DELAY_ON = {
        {"/ch/", _channelNum, "/delay/on"},
        EnumParam("chDelayOn", "Channel Delay On", "Turns the delay on or off for the channel", OFF_ON)
    };
    const NonIter_PathToArgumentPair DELAY_TIME = {
        {"/ch/", _channelNum, "/delay/time"},
        NonIter("chDelayTime", "Channel Delay Time",
                "The amount of delay to apply to the channel's input source (ms)", 0.3f, LINF, 0.3f, 500.f)
    };
    const NonIter_PathToArgumentPair TRIM = {
        {"/ch/", _channelNum, "/preamp/trim"},
        NonIter("chTrim", "Channel Trim", "The digital trim level for the channel. Only for digital sources (dB)", 0.f, LINF, -18.f, 18.f)
    };
    const EnumParam_PathToArgumentPair INVERT = {
        {"/ch/", _channelNum, "/preamp/invert"},
        EnumParam("chInvert", "Channel Invert On", "If the channel's signal should be inverted", OFF_ON)
    };
    const EnumParam_PathToArgumentPair HPF_ON = {
        {"/ch/", _channelNum, "/preamp/hpon"},
        EnumParam("chHPFOn", "Channel High Pass Filter On", "Turns the channel's high pass filter (low cut) on or off", OFF_ON)
    };
    const EnumParam_PathToArgumentPair HPF_SLOPE = {
        {"/ch/", _channelNum, "/preamp/hpslope"},
        EnumParam("chHPFSlope", "Channel High Pass Filter Slope", "The slope for the channel's high pass filter (low cut)", {"12", "18", "24"})
    };
    const NonIter_PathToArgumentPair HPF_FREQ = {
        {"/ch/", _channelNum, "/preamp/hpf"},
        NonIter("chHPFFreq", "Channel High Pass Filter Frequency", "The frequency for the channel's high pass filter (low cut) (Hz)", 20.f, LOGF, 20.f, 400.f)
    };

    const EnumParam_PathToArgumentPair GATE_ON = {
        {"/ch/", _channelNum, "/gate/on"},
        EnumParam("chGateOn", "Channel Gate On", "Turns the channel's gate on or off", OFF_ON)
    };
    const EnumParam_PathToArgumentPair GATE_MODE = {
        {"/ch/", _channelNum, "/gate/mode"},
        EnumParam("chGateMode", "Channel Gate Mode", "The type of gate the channel uses", {"EXP2", "EXP3", "EXP4", "GATE", "DUCK"})
    };
    const NonIter_PathToArgumentPair GATE_THR = {
        {"/ch/", _channelNum, "/gate/thr"},
        NonIter("chGateThr", "Channel Gate Threshold", "The threshold for a channel's gate to activate (dB)", -80.f, LINF, -80.f, 0.f)
    };
    const NonIter_PathToArgumentPair GATE_RANGE = {
        {"/ch/", _channelNum, "/gate/range"},
        NonIter("chGateRange", "Channel Gate Range", "The range for a channel's gate (dB)", 60.f, LINF, 3.f, 60.f)
    };
    // TODO: Test if this is actually LINF or actually LOGF
    const NonIter_PathToArgumentPair GATE_ATTACK = {
        {"/ch/", _channelNum, "/gate/attack"},
        NonIter("chGateAttack", "Channel Gate Attack", "The time for a channel's gate to reach maximum effect (ms)", 0.f, LINF, 0.f, 120.f)
    };
    const NonIter_PathToArgumentPair GATE_HOLD = {
        {"/ch/", _channelNum, "/gate/hold"},
        NonIter("chGateHold", "Channel Gate Hold", "The time for a channel's gate to hold at maximum effect (ms)", 0.02f, LOGF, 0.02f, 2000.f)
    };
    const NonIter_PathToArgumentPair GATE_RELEASE = {
        {"/ch/", _channelNum, "/gate/release"},
        NonIter("chGateRelease", "Channel Gate Release", "The time for a channel's gate to fade to no effect (ms)", 5.f, LOGF, 5.f, 4000.f)
    };
    const EnumParam_PathToArgumentPair GATE_KEYSRC = {
        {"/ch/", _channelNum, "/gate/keysrc"},
        EnumParam("chGateKeySrc", "Channel Gate Key Source", "The key source for a channel's gate", SOURCES)
    };
    const EnumParam_PathToArgumentPair GATE_FILTER_ON = {
        {"/ch/", _channelNum, "/gate/filter/on"},
        EnumParam("chGateFltrOn", "Channel Gate Filter On", "Turns the channel's gate's filter on or off", OFF_ON)
    };
    const EnumParam_PathToArgumentPair GATE_FILTER_TYPE = {
        {"/ch/", _channelNum, "/gate/filter/type"},
        EnumParam("chGateFltrType", "Channel Gate Filter Type", "The type of filter the channel's gate uses (solo/Q)",
            {"LC6", "LC12", "HC6", "HC12", "1.0", "2.0", "3.0", "5.0", "10.0"})
    };
    const NonIter_PathToArgumentPair GATE_FILTER_FREQ = {
        {"/ch/", _channelNum, "/gate/filter/f"},
        NonIter("chGateFltrFreq", "Channel Gate Filter Frequency", "The frequency for a channel's gate filter (Hz)", 20.f, LOGF, 20000.f)
    };

    const EnumParam_PathToArgumentPair DYN_ON = {
        {"/ch/", _channelNum, "/dyn/on"},
        EnumParam("chDynOn", "Channel Dynamics On", "Turns the channel's dynamics processor on or off", OFF_ON)
    };
    const EnumParam_PathToArgumentPair DYN_MODE = {
        {"/ch/", _channelNum, "/dyn/mode"},
        EnumParam("chDynMode", "Channel Dynamics Mode", "The mode for the channel's dynamics processor to operate as", {"COMP", "EXP"})
    };
    const EnumParam_PathToArgumentPair DYN_DET = {
        {"/ch/", _channelNum, "/dyn/det"},
        EnumParam("chDynDet", "Channel Dynamics Detection", "The algorithm for the channel's dynamics processor to detect threshold", {"PEAK", "RMS"})
    };
    const EnumParam_PathToArgumentPair DYN_ENV = {
        {"/ch/", _channelNum, "/dyn/env"},
        EnumParam("chDynEnv", "Channel Dynamics Envelope", "The envelope for the channel's dynamics processor to detect threshold", {"LIN", "LOG"})
    };
    const NonIter_PathToArgumentPair DYN_THR = {
        {"/ch/", _channelNum, "/dyn/thr"},
        NonIter("chDynThr", "Channel Dynamics Threshold", "The threshold for a channel's dynamics processor to activate (dB)", 0.f, LINF, -60.f, 0.f)
    };
    const EnumParam_PathToArgumentPair DYN_RATIO = {
        {"/ch/", _channelNum, "/dyn/ratio"},
        EnumParam("chDynRatio", "Channel Dynamics Ratio", "The ratio for channel's dynamics processor", {"1.1", "1.3", "1.5", "2.0", "2.5", "3.0", "4.0", "5.0", "7.0", "10", "20", "100"})
    };
    const NonIter_PathToArgumentPair DYN_KNEE = {
        {"/ch/", _channelNum, "/dyn/knee"},
        NonIter("chDynKnee", "Channel Dynamics Knee", "The knee for a channel's dynamics processor", 0.f, LINF, 0.f, 5.f)
    };
    const NonIter_PathToArgumentPair DYN_MGAIN = {
        {"/ch/", _channelNum, "/dyn/mgain"},
        NonIter("chDynMGain", "Channel Dynamics Makeup Gain", "The makeup gain to the signal applied post dynamic processing", 0.f, LINF, 0.f, 24.f)
    };
    const NonIter_PathToArgumentPair DYN_ATTACK = {
        {"/ch/", _channelNum, "/dyn/attack"},
        NonIter("chDynAttack", "Channel Dynamics Attack", "The time for a channel's dynamics processor to reach maximum effect (ms)", 0.f, LINF, 0.f, 120.f)
    };
    const NonIter_PathToArgumentPair DYN_HOLD = {
        {"/ch/", _channelNum, "/dyn/hold"},
        NonIter("chDynHold", "Channel Dynamics Hold", "The time for a channel's dynamics processor to hold at maximum effect (ms)", 0.02f, LOGF, 0.02f, 2000.f)
    };
    const NonIter_PathToArgumentPair DYN_RELEASE = {
        {"/ch/", _channelNum, "/dyn/release"},
        NonIter("chDynRelease", "Channel Dynamics Release", "The time for a channel's dynamics processor to fade to no effect (ms)", 5.f, LOGF, 5.f, 4000.f)
    };
    const EnumParam_PathToArgumentPair DYN_POS = {
        {"/ch/", _channelNum, "/dyn/pos"},
        EnumParam("chDynPos", "Channel Dynamics Position", "If the dynamics should be Pre-EQ or Post-EQ", {"PRE", "POST"})
    };
    const EnumParam_PathToArgumentPair DYN_KEYSRC = {
        {"/ch/", _channelNum, "/dyn/keysrc"},
        EnumParam("chDynKeySrc", "Channel Dynamics Key Source", "The key source for a channel's dynamics processor", SOURCES)
    };
    const NonIter_PathToArgumentPair DYN_MIX = {
        {"/ch/", _channelNum, "/dyn/mix"},
        NonIter("chDynMix", "Channel Dynamics Mix", "The percentage of the mix to passthrough the dynamics processor (%)", 100.f, LINF, 0.f, 100.f)
    };
    const EnumParam_PathToArgumentPair DYN_AUTO = {
        {"/ch/", _channelNum, "/dyn/auto"},
        EnumParam("chDynAuto", "Channel Dynamics Auto", "Turns the channel's dynamics processor automatically time on or off", OFF_ON)
    };
    const EnumParam_PathToArgumentPair DYN_FILTER_ON = {
        {"/ch/", _channelNum, "/dyn/filter/on"},
        EnumParam("chDynFltrOn", "Channel Dynamics Filter On", "Turns the channel's dynamics processor filter on or off", OFF_ON)
    };
    const EnumParam_PathToArgumentPair DYN_FILTER_TYPE = {
        {"/ch/", _channelNum, "/dyn/filter/type"},
        EnumParam("chDynFltrType", "Channel Dynamics Filter Type", "The type of filter the channel's dynamics processor uses (solo/Q)",
            {"LC6", "LC12", "HC6", "HC12", "1.0", "2.0", "3.0", "5.0", "10.0"})
    };
    const NonIter_PathToArgumentPair DYN_FILTER_FREQ = {
        {"/ch/", _channelNum, "/dyn/filter/f"},
        NonIter("chDynFltrFreq", "Channel Dynamics Filter Frequency", "The frequency for a channel's dynamics processor filter (Hz)", 20.f, LOGF, 20000.f)
    };





    // TODO: Some more to add!
    const NonIter_PathToArgumentPair FADER = {
        {"/ch/", _channelNum, "/mix/fader"},
        NonIter("chFader", "Channel Fader", "The fader value for the channel", -90.f, LEVEL_1024, -90.f, 10.f)
    };

}
