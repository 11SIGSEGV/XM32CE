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


enum TemplateCategory {
                // Category Name                (IDPrefix)
    CH,         // Channel                      (C), C for Channel
    AUX,        // Aux Ins                      (A), A for Aux
    FXRTN,      // Effects Returns              (R), R for Return
    BUS,        // Mixbuses                     (B), B for Bus
    MTX,        // Matricies                    (X), X for ...idk, i guess for matrix?
    MAIN,       // Stereo Mains                 (S), S for Stereo
    MMONO,      // Mono Main                    (M), M fr Mono
    DCA,        // Digitally Controlled
                //  Amplifiers (DCAs)           (D), D for DCA
    FX,         // Effects                      (F), F for Fx
    OUT,        // Outputs                      (O), O for Output
    HDAMP,      // Headamps                     (H), H for Headamp
    INSRT,      // Effects Inserts              (I), I for Insert
    SHOW,       // Show, Cue, Scene, Snippet
                //  and Preset Management       (P), P for Preset
    NUL         // Null Template                (N), N for Null. Used for blank XM32Templates.
};


// Used by XM32Templates to determine which NonIters assume fading available
inline const std::set<ParamType> DEFAULT_FADING_ENABLED = {LINF, LOGF, INT, LEVEL_161, LEVEL_1024};


// OptionParam not here! This is because X/M32 OSC never actually requires an option parameter.
// It only exists for compatability with non-X/M32 devices.
struct XM32Template {
    const std::string NAME;
    const std::string ID;
    // The first letter of the ID refers to the template category. See above. The ID is recommended to be 5 characters long.
    const bool FADE_ENABLED;
    // Defaults to true for NonIter templates when not bitset or string... but can be overwritten. Should NEVER be true for EnumParams. Overwrites should be respected.
    const TemplateCategory CATEGORY;
    const ArgumentEmbeddedPath PATH;
    const NonIter NONITER;
    const EnumParam ENUMPARAM;
    const bool _META_UsesNonIter;

    // Blank Template
    XM32Template(): CATEGORY(NUL), NONITER(nullNonIter), ENUMPARAM(nullEnum), FADE_ENABLED(false),
                    _META_UsesNonIter(false) {
    }

    // Uses nonIter.verboseName as name for template and automatically determine if fading is enabled
    XM32Template(const std::string &id, const TemplateCategory category, const ArgumentEmbeddedPath &path,
                 const NonIter &nonIter): NAME(nonIter.verboseName), ID(id), CATEGORY(category), PATH(path),
                                          NONITER(nonIter),
                                          ENUMPARAM(nullEnum), _META_UsesNonIter(true),
                                          FADE_ENABLED(DEFAULT_FADING_ENABLED.count(nonIter._meta_PARAMTYPE)) {
    }

    // Uses nonIter.verboseName as name for template
    XM32Template(const std::string &id, const TemplateCategory category, const ArgumentEmbeddedPath &path,
                 const NonIter &nonIter, const bool fadeEnabled): NAME(nonIter.verboseName), ID(id),
                                                                  FADE_ENABLED(fadeEnabled), CATEGORY(category),
                                                                  PATH(path), NONITER(nonIter), ENUMPARAM(nullEnum),
                                                                  _META_UsesNonIter(true) {
    }

    // Manually specify name
    XM32Template(const std::string &id, const std::string &name, const TemplateCategory category,
                 const ArgumentEmbeddedPath &path, const NonIter &nonIter,
                 const bool fadeEnabled = true): NAME(name), ID(id), FADE_ENABLED(fadeEnabled), CATEGORY(category),
                                                 PATH(path), NONITER(nonIter), ENUMPARAM(nullEnum),
                                                 _META_UsesNonIter(true) {
    }

    // Manually specify name and automatically determine if fading is enabled
    XM32Template(const std::string &id, const std::string &name, const TemplateCategory category,
                 const ArgumentEmbeddedPath &path, const NonIter &nonIter): NAME(name), ID(id), CATEGORY(category),
                                                                            PATH(path), NONITER(nonIter),
                                                                            ENUMPARAM(nullEnum),
                                                                            _META_UsesNonIter(true),
                                                                            FADE_ENABLED(
                                                                                DEFAULT_FADING_ENABLED.count(
                                                                                    nonIter._meta_PARAMTYPE)) {
    }

    // Uses enumParam.verboseName as name for template
    XM32Template(const std::string &id, const TemplateCategory category, const ArgumentEmbeddedPath &path,
                 const EnumParam &enumParam): NAME(enumParam.verboseName), ID(id), FADE_ENABLED(false),
                                              CATEGORY(category), PATH(path), NONITER(nullNonIter),
                                              ENUMPARAM(enumParam), _META_UsesNonIter(false) {
    }

    // Manually specify name
    XM32Template(const std::string &id, const std::string &name, const TemplateCategory category,
                 const ArgumentEmbeddedPath &path, const EnumParam &enumParam): NAME(name), ID(id), FADE_ENABLED(false),
        CATEGORY(category), PATH(path), NONITER(nullNonIter), ENUMPARAM(enumParam), _META_UsesNonIter(false) {
    }


    // Not recommended. For performance, the type of the template should be checked first, then a control path should be entered depending
    // on the appropriate type.
    [[nodiscard]] OSCMessageArguments getRawMessageArgument() const {
        if (_META_UsesNonIter) {
            return NONITER;
        }
        return ENUMPARAM;
    }
};


struct XM32TemplateGroup {
    const TemplateCategory category;
    const std::vector<XM32Template> &templates;
    const std::string name;
    const int size;

    XM32TemplateGroup(const TemplateCategory category, const std::string &name,
                      const std::vector<XM32Template> &templates): category(category), templates(templates), name(name),
                                                                   size(templates.size()) {
    }
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



namespace Channel {
    const NonIter _channelNum = {
        "chNum", "Channel Number", "The number of the channel to send OSC commands to", 1, 1, 32
    };

    const XM32Template NAME = {
        "CNAME", CH, {"/ch/", _channelNum, "/config/name"},
        NonIter("chName", "Name", "The name of the channel", "", 0, 12),
    };
    const XM32Template ICON = {
        "CICON", CH, {"/ch/", _channelNum, "/config/icon"},
        NonIter("chIcon", "Icon", "The index for the channel's icon", 1, 1, 74)
    };
    const XM32Template COLOUR = {
        "CCOLR", CH, {"/ch/", _channelNum, "/config/color"},
        EnumParam("chColour", "Colour", "The index representing the colour of the channel",
                  {
                      "OFF", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE", "OFFi", "REDi", "GREENi",
                      "YELLOWi", "BLUEi", "MAGENTAi", "CYANi", "WHITEi"
                  })
    };

    const XM32Template SOURCE = {
        "CSRCE", CH, {"/ch/", _channelNum, "/config/source"},
        EnumParam("chSource", "Source", "The input source for this channel",
                  SOURCES)
    };
    const XM32Template DELAY_ON = {
        "CDYON", CH, {"/ch/", _channelNum, "/delay/on"},
        EnumParam("chDelayOn", "Delay On", "Turns the delay on or off for the channel", OFF_ON)
    };
    const XM32Template DELAY_TIME = {
        "CDYTM", CH, {"/ch/", _channelNum, "/delay/time"},
        NonIter("chDelayTime", "Delay Time",
                "The amount of delay to apply to the channel's input source (ms)", 0.3f, LINF, 0.3f, 500.f, MS)
    };
    const XM32Template TRIM = {
        "CTRIM", CH, {"/ch/", _channelNum, "/preamp/trim"},
        NonIter("chTrim", "Trim", "The digital trim level for the channel. Only for digital sources (dB)", 0.f, LINF, -18.f, 18.f, DB)
    };
    const XM32Template INVERT = {
        "CIVRT", CH, {"/ch/", _channelNum, "/preamp/invert"},
        EnumParam("chInvert", "Invert On", "If the channel's signal should be inverted", OFF_ON)
    };
    const XM32Template HPF_ON = {
        "CHFON", CH, {"/ch/", _channelNum, "/preamp/hpon"},
        EnumParam("chHPFOn", "High Pass Filter On", "Turns the channel's high pass filter (low cut) on or off", OFF_ON)
    };
    const XM32Template HPF_SLOPE = {
        "CHFSP", CH, {"/ch/", _channelNum, "/preamp/hpslope"},
        EnumParam("chHPFSlope", "High Pass Filter Slope", "The slope for the channel's high pass filter (low cut)", {"12", "18", "24"})
    };
    const XM32Template HPF_FREQ = {
        "CHFFQ", CH, {"/ch/", _channelNum, "/preamp/hpf"},
        NonIter("chHPFFreq", "High Pass Filter Frequency", "The frequency for the channel's high pass filter (low cut) (Hz)", 20.f, LOGF, 20.f, 400.f, HERTZ)
    };

    const XM32Template GATE_ON = {
        "CGTON", CH, {"/ch/", _channelNum, "/gate/on"},
        EnumParam("chGateOn", "Gate On", "Turns the channel's gate on or off", OFF_ON)
    };
    const XM32Template GATE_MODE = {
        "CGTMD", CH, {"/ch/", _channelNum, "/gate/mode"},
        EnumParam("chGateMode", "Gate Mode", "The type of gate the channel uses", {"EXP2", "EXP3", "EXP4", "GATE", "DUCK"})
    };
    const XM32Template GATE_THR = {
        "CGTTR", CH, {"/ch/", _channelNum, "/gate/thr"},
        NonIter("chGateThr", "Gate Threshold", "The threshold for a channel's gate to activate (dB)", -80.f, LINF, -80.f, 0.f, DB)
    };
    const XM32Template GATE_RANGE = {
        "CGTRG", CH, {"/ch/", _channelNum, "/gate/range"},
        NonIter("chGateRange", "Gate Range", "The range for a channel's gate (dB)", 60.f, LINF, 3.f, 60.f, DB)
    };
    // TODO: Test if this is actually LINF or actually LOGF
    const XM32Template GATE_ATTACK = {
        "CGTAK", CH, {"/ch/", _channelNum, "/gate/attack"},
        NonIter("chGateAttack", "Gate Attack", "The time for a channel's gate to reach maximum effect (ms)", 0.f, LINF, 0.f, 120.f, MS)
    };
    const XM32Template GATE_HOLD = {
        "CGTHD", CH, {"/ch/", _channelNum, "/gate/hold"},
        NonIter("chGateHold", "Gate Hold", "The time for a channel's gate to hold at maximum effect (ms)", 0.02f, LOGF, 0.02f, 2000.f, MS)
    };
    const XM32Template GATE_RELEASE = {
        "CGTRS", CH, {"/ch/", _channelNum, "/gate/release"},
        NonIter("chGateRelease", "Gate Release", "The time for a channel's gate to fade to no effect (ms)", 5.f, LOGF, 5.f, 4000.f, MS)
    };
    const XM32Template GATE_KEYSRC = {
        "CGTKS", CH, {"/ch/", _channelNum, "/gate/keysrc"},
        EnumParam("chGateKeySrc", "Gate Key Source", "The key source for a channel's gate", SOURCES)
    };
    const XM32Template GATE_FILTER_ON = {
        "CGTFO", CH, {"/ch/", _channelNum, "/gate/filter/on"},
        EnumParam("chGateFltrOn", "Gate Filter On", "Turns the channel's gate's filter on or off", OFF_ON)
    };
    const XM32Template GATE_FILTER_TYPE = {
        "CGTFT", CH, {"/ch/", _channelNum, "/gate/filter/type"},
        EnumParam("chGateFltrType", "Gate Filter Type", "The type of filter the channel's gate uses (solo/Q)",
            {"LC6", "LC12", "HC6", "HC12", "1.0", "2.0", "3.0", "5.0", "10.0"})
    };
    const XM32Template GATE_FILTER_FREQ = {
        "CGTFF", CH, {"/ch/", _channelNum, "/gate/filter/f"},
        NonIter("chGateFltrFreq", "Gate Filter Frequency", "The frequency for a channel's gate filter (Hz)", 20.f, LOGF, 20.f, 20000.f, HERTZ)
    };

    const XM32Template DYN_ON = {
        "CDYON", CH, {"/ch/", _channelNum, "/dyn/on"},
        EnumParam("chDynOn", "Dynamics On", "Turns the channel's dynamics processor on or off", OFF_ON)
    };
    const XM32Template DYN_MODE = {
        "CDYMD", CH, {"/ch/", _channelNum, "/dyn/mode"},
        EnumParam("chDynMode", "Dynamics Mode", "The mode for the channel's dynamics processor to operate as", {"COMP", "EXP"})
    };
    const XM32Template DYN_DET = {
        "CDYDT", CH, {"/ch/", _channelNum, "/dyn/det"},
        EnumParam("chDynDet", "Dynamics Detection", "The algorithm for the channel's dynamics processor to detect threshold", {"PEAK", "RMS"})
    };
    const XM32Template DYN_ENV = {
        "CDYEV", CH, {"/ch/", _channelNum, "/dyn/env"},
        EnumParam("chDynEnv", "Dynamics Envelope", "The envelope for the channel's dynamics processor to detect threshold", {"LIN", "LOG"})
    };
    const XM32Template DYN_THR = {
        "CDYTR", CH, {"/ch/", _channelNum, "/dyn/thr"},
        NonIter("chDynThr", "Dynamics Threshold", "The threshold for a channel's dynamics processor to activate (dB)", 0.f, LINF, -60.f, 0.f, DB)
    };
    const XM32Template DYN_RATIO = {
        "CDYRT", CH, {"/ch/", _channelNum, "/dyn/ratio"},
        EnumParam("chDynRatio", "Dynamics Ratio", "The ratio for channel's dynamics processor", {"1.1", "1.3", "1.5", "2.0", "2.5", "3.0", "4.0", "5.0", "7.0", "10", "20", "100"})
    };
    const XM32Template DYN_KNEE = {
        "CDYKN", CH, {"/ch/", _channelNum, "/dyn/knee"},
        NonIter("chDynKnee", "Dynamics Knee", "The knee for a channel's dynamics processor", 0.f, LINF, 0.f, 5.f)
    };
    const XM32Template DYN_MGAIN = {
        "CDYMG", CH, {"/ch/", _channelNum, "/dyn/mgain"},
        NonIter("chDynMGain", "Dynamics Makeup Gain", "The makeup gain to the signal applied post dynamic processing (dB)", 0.f, LINF, 0.f, 24.f, DB)
    };
    const XM32Template DYN_ATTACK = {
        "CDYAK", CH, {"/ch/", _channelNum, "/dyn/attack"},
        NonIter("chDynAttack", "Dynamics Attack", "The time for a channel's dynamics processor to reach maximum effect (ms)", 0.f, LINF, 0.f, 120.f, MS)
    };
    const XM32Template DYN_HOLD = {
        "CDYHD", CH, {"/ch/", _channelNum, "/dyn/hold"},
        NonIter("chDynHold", "Dynamics Hold", "The time for a channel's dynamics processor to hold at maximum effect (ms)", 0.02f, LOGF, 0.02f, 2000.f, MS)
    };
    const XM32Template DYN_RELEASE = {
        "CDYRS", CH, {"/ch/", _channelNum, "/dyn/release"},
        NonIter("chDynRelease", "Dynamics Release", "The time for a channel's dynamics processor to fade to no effect (ms)", 5.f, LOGF, 5.f, 4000.f, MS)
    };
    const XM32Template DYN_POS = {
        "CDYPS", CH, {"/ch/", _channelNum, "/dyn/pos"},
        EnumParam("chDynPos", "Dynamics Position", "If the dynamics should be Pre-EQ or Post-EQ", {"PRE", "POST"})
    };
    const XM32Template DYN_KEYSRC = {
        "CDYKS", CH, {"/ch/", _channelNum, "/dyn/keysrc"},
        EnumParam("chDynKeySrc", "Dynamics Key Source", "The key source for a channel's dynamics processor", SOURCES)
    };
    const XM32Template DYN_MIX = {
        "CDYMX", CH, {"/ch/", _channelNum, "/dyn/mix"},
        NonIter("chDynMix", "Dynamics Mix", "The percentage of the mix to passthrough the dynamics processor (%)", 100.f, LINF, 0.f, 100.f)
    };
    const XM32Template DYN_AUTO = {
        "CDYAT", CH, {"/ch/", _channelNum, "/dyn/auto"},
        EnumParam("chDynAuto", "Dynamics Auto", "Turns the channel's dynamics processor automatically time on or off", OFF_ON)
    };
    const XM32Template DYN_FILTER_ON = {
        "CDYFO", CH, {"/ch/", _channelNum, "/dyn/filter/on"},
        EnumParam("chDynFltrOn", "Dynamics Filter On", "Turns the channel's dynamics processor filter on or off", OFF_ON)
    };
    const XM32Template DYN_FILTER_TYPE = {
        "CDYFT", CH, {"/ch/", _channelNum, "/dyn/filter/type"},
        EnumParam("chDynFltrType", "Dynamics Filter Type", "The type of filter the channel's dynamics processor uses (solo/Q)",
            {"LC6", "LC12", "HC6", "HC12", "1.0", "2.0", "3.0", "5.0", "10.0"})
    };
    const XM32Template DYN_FILTER_FREQ = {
        "CDYFF", CH, {"/ch/", _channelNum, "/dyn/filter/f"},
        NonIter("chDynFltrFreq", "Dynamics Filter Frequency", "The frequency for a channel's dynamics processor filter (Hz)", 20.f, LOGF, 20.f, 20000.f, HERTZ)
    };





    // TODO: Some more to add!
    const XM32Template FADER = {
        "CFADR", CH, {"/ch/", _channelNum, "/mix/fader"},
        NonIter("chFader", "Fader", "The fader value for the channel", -90.f, LEVEL_1024, -90.f, 10.f)
    };

    const std::vector<XM32Template> ALL_TEMPLATES = {
        NAME, ICON, COLOUR, SOURCE, DELAY_ON, DELAY_TIME, TRIM, INVERT, HPF_ON, HPF_SLOPE, HPF_FREQ,
        GATE_ON, GATE_MODE, GATE_THR, GATE_RANGE, GATE_ATTACK, GATE_HOLD, GATE_RELEASE, GATE_KEYSRC,
        GATE_FILTER_ON, GATE_FILTER_TYPE, GATE_FILTER_FREQ,
        DYN_ON, DYN_MODE, DYN_DET, DYN_ENV, DYN_THR, DYN_RATIO, DYN_KNEE, DYN_MGAIN, DYN_ATTACK,
        DYN_HOLD, DYN_RELEASE, DYN_POS, DYN_KEYSRC, DYN_MIX, DYN_AUTO, DYN_FILTER_ON, DYN_FILTER_TYPE, DYN_FILTER_FREQ,
        FADER
    };

}





const XM32TemplateGroup CHANNEL_GROUP {CH, "Channel", Channel::ALL_TEMPLATES};


const std::map<TemplateCategory, XM32TemplateGroup> TEMPLATE_CATEGORY_MAP = {
    {CH, CHANNEL_GROUP}
};
