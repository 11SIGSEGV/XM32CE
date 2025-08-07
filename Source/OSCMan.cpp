/*
  ==============================================================================

    OSCMan.cpp
    Created: 11 May 2025 6:46:13pm
    Author:  anony

  ==============================================================================
*/

#include "OSCMan.h"


OSCDeviceSender::OSCDeviceSender(const String &ipAddress, const String &port, const String &deviceName) {
    auto ipv4AddrValidatorOut = isValidIPv4(ipAddress);
    if (!ipv4AddrValidatorOut.isValid) {
        throw std::invalid_argument(
            (String("IP address is not valid: ") + ipv4AddrValidatorOut.errorMessage).toStdString());
    }
    auto portValidatorOut = isValidPort(port);
    if (!portValidatorOut.isValid) {
        throw std::invalid_argument(
            (String("Port is not valid: ") + portValidatorOut.errorMessage).toStdString());
    }
    auto deviceNameValidatorOut = isValidDeviceName(deviceName);
    if (!deviceNameValidatorOut.isValid) {
        throw std::invalid_argument(
            (String("Device name is not valid: ") + deviceNameValidatorOut.errorMessage).toStdString());
    }
    this->ipAddress = ipAddress;
    this->port = port.getIntValue();
    this->deviceName = deviceName;

    connect();
}

/* Assumes device is valid*/
OSCDeviceSender::OSCDeviceSender(const OSCDevice &device) {
    if (device.deviceName.isEmpty()) {
        this->ipAddress = "127.0.0.1";
        this->port = 10023;
        this->deviceName = "LocalX32";
        connect();
        return;
    }
    this->ipAddress = device.ipAddress;
    this->port = device.port;
    this->deviceName = device.deviceName;
    connect();
}


OSCDeviceSender::OSCDeviceSender(const String &ipAddress, const int port, const String &deviceName) {
    String properIP;
    int properPort;
    String properDeviceName;
    auto ipv4AddrValidatorOut = isValidIPv4(ipAddress);
    if (!ipv4AddrValidatorOut.isValid) {
        jassertfalse; // IP address is not valid.
        properIP = "127.0.0.1";
    } else {
        properIP = ipAddress;
    }
    if (port < 0 || port > 65535) {
        properPort = 10023;
    } else {
        properPort = port;
    }
    auto deviceNameValidatorOut = isValidDeviceName(deviceName);
    if (!deviceNameValidatorOut.isValid) {
        jassertfalse; // Invalid device name
        properDeviceName = "LocalX32";
    } else {
        properDeviceName = deviceName;
    }
    this->ipAddress = properIP;
    this->port = properPort;
    this->deviceName = properDeviceName;
}



std::vector<OSCArgument> OSCDeviceSender::compileOSCArguments(
    std::vector<OSCMessageArguments> &args,
    ValueStorerArray &argVals) {
    std::vector<OSCArgument> oscArguments;
    if (args.size() != argVals.size()) {
        jassertfalse; // Argument values and templates must be the same size.
        return {};
    }
    for (size_t i = 0; i < args.size(); ++i) {
        if (auto *optVal = std::get_if<OptionParam>(&args[i])) {
            // If it's an OptionParam, the value from the ValueStorer will be the string.
            // Do a quick check that the stringValue is a valid option
            if (std::find(optVal->value.begin(), optVal->value.end(), argVals[i].stringValue)
                == optVal->value.end()) {
                jassertfalse; // Value not found in the options
                return {};
            }
            oscArguments.emplace_back(String(argVals[i].stringValue));
        } else if (auto *enumVal = std::get_if<EnumParam>(&args[i])) {
            if (enumVal->value.size() < argVals[i].intValue) {
                jassertfalse; // Enum value out of range
                return {};
            }
            // As this is not a OPTIONS, we only need the index of the ENUM as the value.
            oscArguments.emplace_back(argVals[i].intValue);
        } else if (auto *nonIter = std::get_if<NonIter>(&args[i])) {
            // Let's first determine if the value is int, float, string or bitset
            // The NonIter will indicate the type (_meta_PARAMTYPE)
            switch (nonIter->_meta_PARAMTYPE) {
                case INT: {
                    // Get the value from the ValueStorer and append it to the finalString, after checking constraints
                    if (nonIter->intMin > argVals[i].intValue ||
                        nonIter->intMax < argVals[i].intValue) {
                        jassertfalse; // Integer value out of range
                        return {};
                    }
                    oscArguments.emplace_back(argVals[i].intValue);
                    break;
                }
                case LINF:
                case LOGF:
                case LEVEL_161:
                case LEVEL_1024: {
                    if (nonIter->floatMin > argVals[i].floatValue ||
                        nonIter->floatMax < argVals[i].floatValue) {
                        jassertfalse; // Float value out of range
                        return {};
                    }
                    oscArguments.emplace_back(argVals[i].floatValue);
                    break;
                }
                case STRING: {
                    // Check string length limitations and append
                    // Remember, intMax and intMin are the max and min length of the string
                    if (argVals[i].stringValue.length() < nonIter->intMin ||
                        argVals[i].stringValue.length() > nonIter->intMax) {
                        jassertfalse; // String value out of size range
                    }
                    oscArguments.emplace_back(argVals[i].stringValue);
                    break;
                }
                case BITSET: {
                    // Check the size of the bitset
                    if (nonIter->intMax != argVals[i].stringValue.size()) {
                        jassertfalse; // Bitset value is not expected size
                    }
                    // Convert string to bitset, then to int
                    oscArguments.emplace_back(std::stoi(argVals[i].stringValue, nullptr, 2));
                    break;
                }
                default:
                    jassertfalse; // Unsupported ParamType for NonIter Parameter Template
                    return {};
            }
        } else {
            jassertfalse; // Unable to cast variant to valid parameter type for OSC Message arguments
        }
    }
    return oscArguments;
}


String OSCDeviceSender::fillInArgumentsOfEmbeddedPath(const ArgumentEmbeddedPath &path, const ValueStorerArray &pthArgVal) {
    String finalString;

    // Loop through each element of path. If it's a string, directly append it to finalString. Otherwise,
    // fetch a value from pathArgumentValues and convert it to a string appendable format

    // Being by checks. Find number of in-path arguments.
    int argIndx{0};
    const int pathArgValsSize = pthArgVal.size();
    for (auto &segment : path) {
        if (const auto *strVal = std::get_if<std::string>(&segment)) {
            // If it's a string, append it directly
            finalString += String(*strVal);
        }
        // Enum and Option ParamTypes in ArgumentEmbeddedPaths have been deprecated.
        /*else if (const auto *optVal = std::get_if<OptionParam>(&segment)) {
            if (argIndx < pthArgVal.size()) {
                // If it's an OptionParam, the value from the ValueStorer will be the string.
                // Do a quick check that the stringValue is a valid option
                if (std::find(optVal->value.begin(), optVal->value.end(), pthArgVal[argIndx].stringValue)
                    == optVal->value.end()) {
                    // Value not found in the options
                    jassertfalse; // Invalid option value provided for the path
                }
                finalString += String(pthArgVal[argIndx].stringValue);
                ++argIndx;
            } else
                jassertfalse; // Not enough path arguments provided for the path
        } else if (const auto *enumVal = std::get_if<EnumParam>(&segment)) {
            // When the value is an enum, the ValueStore will have an int which the vector of the enum options will have
            // To check for this, check the int in the ValueStore is smaller than the size of the vector
            if (enumVal->value.size() < pthArgVal[argIndx].intValue) {
                jassertfalse; // Enum value out of range
            }
            // As this is not a OPTIONS, we only need the index of the ENUM as the value.
            finalString += String(pthArgVal[argIndx].intValue);
            ++argIndx;
        } */
        else if (const auto *nonIter = std::get_if<NonIter>(&segment)) {
            // First, check if path[i] exists, or if it's out of range
            if (argIndx >= pathArgValsSize) {
                jassertfalse; // Path argument index out of range
                continue;
            }

            // Let's first determine if the value is int, float, string or bitset
            // The NonIter will indicate the type (_meta_PARAMTYPE)
            switch (nonIter->_meta_PARAMTYPE) {
                case INT: {
                    // Get the value from the ValueStorer and append it to the finalString, after checking constraints
                    if (nonIter->intMin > pthArgVal[argIndx].intValue ||
                        nonIter->intMax < pthArgVal[argIndx].intValue) {
                        jassertfalse; // Integer value out of range
                        break;
                    }
                    finalString += String(pthArgVal[argIndx].intValue);
                    break;
                }
                case STRING: {
                    // Check string length limitations and append
                    // Remember, intMax and intMin are the max and min length of the string
                    if (pthArgVal[argIndx].stringValue.length() < nonIter->intMin ||
                        pthArgVal[argIndx].stringValue.length() > nonIter->intMax) {
                        jassertfalse; // String value out of size range
                        break;
                    }
                    finalString += String(pthArgVal[argIndx].stringValue);
                    break;
                }
                // Bitset support for in-path arguments have been deprecated.
                /*
                case BITSET: {
                    // Basically, it's already formatted 💀
                    // Just append it to the finalString...
                    // But first, check the size of the bitset
                    if (nonIter->intMax != pthArgVal[argIndx].stringValue.size()) {
                        jassertfalse; // Bitset size incorrect
                        break;
                    }
                    // Append the bitset to the finalString
                    finalString += String(pthArgVal[argIndx].stringValue);
                    break;
                }*/
                default:
                    jassertfalse; // Float not included in switch-case - should NEVER be used in Embedded Path Argument
                    break;
            }
            ++argIndx;
        } else
            jassertfalse; // Unable to cast variant to valid parameter type for in-path arguments
    }

    // Check number of arguments was the same as the number of arguments in the path
    // No need for argIndx + 1, as it should have been incremented in the loop (i.e., incremented after it was used, even if never used again)
    if (argIndx != pathArgValsSize) {
        jassertfalse; // Number of arguments in path and number of arguments provided do not match
    }

    return finalString;
}

// Sends actual message. For performance’s sake, no checks are done here, so ensure the message is valid before calling this function.
ThreadPoolJob::JobStatus OSCSingleActionDispatcher::runJob() {
    if (cueAction.oat == OAT_COMMAND) {
        OSCMessage msg{cueAction.oscAddress};
        if (std::get_if<OptionParam>(&cueAction.oatCommandOSCArgumentTemplate)) {
            // If it's an OptionParam, the value from the ValueStorer will be the string.
            msg.addString(cueAction.argument.stringValue);
        } else if (std::get_if<EnumParam>(&cueAction.oatCommandOSCArgumentTemplate)) {
            // As this is not a OPTIONS, we only need the index of the ENUM as the value.
            msg.addInt32(cueAction.argument.intValue);
        } else if (auto *nonIter = std::get_if<NonIter>(&cueAction.oatCommandOSCArgumentTemplate)) {
            // Let's first determine if the value is int, float, string or bitset
            // The NonIter will indicate the type (_meta_PARAMTYPE)
            switch (nonIter->_meta_PARAMTYPE) {
                case INT: {
                    // Don't try "lin-f" it. Let it be.
                    msg.addInt32(cueAction.argument.intValue);
                    break;
                }
                case LINF:
                case LOGF:
                case LEVEL_161:
                case LEVEL_1024: {
                    // Adjust to normalised range (0.f-1.f)
                    if (nonIter->normalisedInverted) {
                        msg.addFloat32(1 - inferPercentageFromMinMaxAndValue(nonIter->floatMin, nonIter->floatMax, cueAction.argument.floatValue, nonIter->_meta_PARAMTYPE));
                    } else {
                        msg.addFloat32(inferPercentageFromMinMaxAndValue(nonIter->floatMin, nonIter->floatMax, cueAction.argument.floatValue, nonIter->_meta_PARAMTYPE));
                    }
                    break;
                }
                case STRING: {
                    msg.addString(cueAction.argument.stringValue);
                    break;
                }
                case BITSET: {
                    msg.addInt32(std::stoi(cueAction.argument.stringValue, nullptr, 2));
                    break;
                }
                default:
                    jassertfalse;
                    // Unsupported ParamType for NonIter Parameter Template. Is it a template ValueStorer (i.e., ParamType blank?)
            }

        } else {
            jassertfalse; // Invalid OSCMessageArguments type in the action
        }
        oscSender.send(msg);
    } else if (cueAction.oat == OAT_FADE) {
        // This is where it gets exponentially more complicated exponentially fast.
        // For OAT_FADE, we need to construct a message with the fade time and the start and end values.
        // We will have to manually construct the message, as the fade time is not integrated into most OSC devices.

        // Fortunately, OSC Cue Actions only support NonIter types, so we are using oscArgumentTemplate.
        // We'll have to normalised startValue and endValue to the range of the NonIter type.

        double normalisedPercentage{0.0};
        double normalisedEndPercentage{0.0};
        int totalIncrements = static_cast<int>(std::ceil(cueAction.fadeTime * 1000 / FMMID));
        // Total increments based on fade time and minimum iteration duration


        // First, we have to find the correct normalised start and end values based on the oscArgumentTemplate.
        if (cueAction.oscArgumentTemplate._meta_PARAMTYPE == INT) {
            // Also assumes LINEAR
            // Normalise the start and end values to the range of the NonIter type
            normalisedPercentage = inferPercentageFromMinMaxAndValue(
                cueAction.oscArgumentTemplate.intMin, cueAction.oscArgumentTemplate.intMax,
                cueAction.startValue.intValue, LINF);
            normalisedEndPercentage = inferPercentageFromMinMaxAndValue(cueAction.oscArgumentTemplate.intMin,
                                                                        cueAction.oscArgumentTemplate.intMax,
                                                                        cueAction.endValue.intValue, LINF);
        } else if (cueAction.oscArgumentTemplate._meta_PARAMTYPE == LINF ||
                   cueAction.oscArgumentTemplate._meta_PARAMTYPE == LOGF ||
                   cueAction.oscArgumentTemplate._meta_PARAMTYPE == LEVEL_161 ||
                   cueAction.oscArgumentTemplate._meta_PARAMTYPE == LEVEL_1024) {
            normalisedPercentage = inferPercentageFromMinMaxAndValue(
                cueAction.oscArgumentTemplate.floatMin, cueAction.oscArgumentTemplate.floatMax,
                cueAction.startValue.floatValue, cueAction.oscArgumentTemplate._meta_PARAMTYPE);
            normalisedEndPercentage = inferPercentageFromMinMaxAndValue(
                cueAction.oscArgumentTemplate.floatMin, cueAction.oscArgumentTemplate.floatMax,
                cueAction.endValue.floatValue, cueAction.oscArgumentTemplate._meta_PARAMTYPE);
            if (cueAction.oscArgumentTemplate.normalisedInverted) {
                normalisedPercentage = 1 - normalisedPercentage;
                normalisedEndPercentage = 1 - normalisedEndPercentage;
            }
        } else {
            jassertfalse; // Unsupported ParamType for NonIter Parameter Template in OAT_FADE
            return JobStatus::jobHasFinished; // Exit the job if the ParamType is unsupported
        }


        // Manually convert min and max values to double - the type inferValueFromMinMaxAndPercentage accepts to prevent
        // type conversion with every call
        const double minVal = cueAction.oscArgumentTemplate._meta_PARAMTYPE == INT
                                  ? cueAction.oscArgumentTemplate.intMin
                                  : cueAction.oscArgumentTemplate.floatMin;
        const double maxVal = cueAction.oscArgumentTemplate._meta_PARAMTYPE == INT
                                  ? cueAction.oscArgumentTemplate.intMax
                                  : cueAction.oscArgumentTemplate.floatMax;


        // Now we have set how much each increment should be.
        double normalisedPercentageIncrement = (normalisedEndPercentage - normalisedPercentage) / totalIncrements;

        // Now for each increment, we will construct the message and send it.
        for (int i = 0; (i < totalIncrements && !shouldExit()); ++i) {
            auto messageStart = std::chrono::high_resolution_clock::now();

            // Construct the message for each increment
            OSCMessage incrementedMsg{cueAction.oscAddress};
            normalisedPercentage += normalisedPercentageIncrement; // Increment the normalised start value

            // The argument will have to be un-normalised to the original type.
            switch (cueAction.oscArgumentTemplate._meta_PARAMTYPE) {
                case INT:
                    incrementedMsg.addInt32(static_cast<int>(inferValueFromMinMaxAndPercentage(
                        minVal, maxVal, normalisedPercentage, LINF)));
                    break;
                default:
                    incrementedMsg.addFloat32(normalisedPercentage);
                    break;
            }

            // Send the message
            oscSender.send(incrementedMsg);


            std::chrono::duration<double, std::milli> elapsed =
                    std::chrono::high_resolution_clock::now() - messageStart;

            // Check if minimum iteration duration has passed
            if (elapsed.count() < FMMID) {
                std::this_thread::sleep_for(std::chrono::milliseconds(FMMID) - elapsed);
            } else {
                DBG("Warning: OAT_FADE iteration took longer than minimum duration. Consider increasing FMMID.");
            }
        }

        // If exited because of shouldExit(), we will not send the end message.
        if (shouldExit()) {
            return jobHasFinished; // Exit the job if the thread should exit
        }

        // Finally, we need to send the end value to ensure the fade is complete... but to be safe, let's check if the
        // normalisedPercentage is not already at the end value.
        if (normalisedPercentage < normalisedEndPercentage) {
            OSCMessage endMsg{cueAction.oscAddress};
            switch (cueAction.oscArgumentTemplate._meta_PARAMTYPE) {
                case INT:
                    endMsg.addInt32(static_cast<int>(inferValueFromMinMaxAndPercentage(
                        minVal, maxVal, normalisedPercentage, LINF)));
                    break;
                default:
                    endMsg.addFloat32(normalisedPercentage);
                    break;
            }
            oscSender.send(endMsg);
        }
    } else {
        jassertfalse; // Unsupported OSC Action Type
        return jobHasFinished; // Exit the job if the OSC Action Type is unsupported
    }
    return jobHasFinished; // Indicate that the job has finished successfully
}


OSCCueDispatcherManager::OSCCueDispatcherManager(OSCDeviceSender &oscDevice,
                                                 unsigned int maximumSimultaneousMessageThreads,
                                                 unsigned int waitMSFromWhenActionQueueIsEmpty): oscSender(oscDevice),
    maximumSimultaneousMessageThreads(maximumSimultaneousMessageThreads), Thread("oscCueDispatcherManager"),
    waitMSFromWhenActionQueueIsEmpty(waitMSFromWhenActionQueueIsEmpty) {
    if (maximumSimultaneousMessageThreads == 0 || maximumSimultaneousMessageThreads > 511) {
        jassertfalse; // Maximum simultaneous message threads must be above 1 and should be below 512.
    }
    addListener(this);
    // setPriority(Priority::high); // Set a higher priority for the thread to ensure it processes messages quickly
};


void OSCCueDispatcherManager::addCueToMessageQueue(const CurrentCueInfo &cueInfo) {
    for (const auto &action: cueInfo.actions) {
        addCueToMessageQueue(action);
    }
}


void OSCCueDispatcherManager::addCueToMessageQueue(const CueOSCAction &cueAction) {
    actionQueue.push(cueAction);
}


void OSCCueDispatcherManager::run() {
    while (!threadShouldExit()) {

        while (actionQueue.empty()) {
            auto waitStart = std::chrono::high_resolution_clock::now();
            // Also check if a cue job has finished. This is a non-realtime operation, so we can shove it into this loop
            // Create a copy of the actionIDToJobMap to avoid modifying the map while iterating
            auto actionIDToJobMapCopy = actionIDToJobMap; // Copy the map to avoid modifying while iterating
            for (const auto& [id, singleActionDispatcher]: actionIDToJobMapCopy) {
                // Check if the singleActionDispatcher is in the pool (i.e., it has not been removed yet)
                if (!singleActionDispatcherPool.contains(singleActionDispatcher)) {
                    actionIDToJobMap.erase(id);
                    // Notify listeners that the action has finished
                    for (auto *listener: dispatchListeners) {
                        listener->actionFinished(id);
                    }
                }
            }
            std::chrono::duration<double, std::milli> elapsed =
                    std::chrono::high_resolution_clock::now() - waitStart;
            if (elapsed.count() < waitMSFromWhenActionQueueIsEmpty) {
                // Sleep for the remaining time to wait
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(waitMSFromWhenActionQueueIsEmpty) - elapsed);
            } else {
                DBG("Warning: OSC Cue Dispatcher Manager exceeded waitMSFromWhenActionQueueIsEmpty. This means each "
                    "iteration checking if the queue is empty took longer than the allowed wait time. "
                    "Consider increasing wait time or running the thread with realtime priority or more "
                    "system resources.");
            }
        }

        auto action = actionQueue.front();
        actionQueue.pop();
        if (action.oat == EXIT_THREAD) {
            return;
        }
        auto *dispatcher = new OSCSingleActionDispatcher(action, oscSender);
        singleActionDispatcherPool.addJob(dispatcher, true);
        actionIDToJobMap[action.ID] = dispatcher;

    }
}


// Tries to stop the action with the given actionID. If the action is not found, it does nothing unless
// jassertWhenNotFound is true, in which case it asserts.
void OSCCueDispatcherManager::stopAction(const std::string &actionID, bool jassertWhenNotFound) {
    // Find the pointer if it exists
    const auto job = actionIDToJobMap.find(actionID);
    if (job == actionIDToJobMap.end()) {
        if (jassertWhenNotFound) { jassertfalse; }  // ID not found.
        return; // Action not found, nothing to stop
    }
    // See if this works...
    singleActionDispatcherPool.removeJob(job->second, true, 1000);
}


void OSCCueDispatcherManager::stopAllActionsInCCI(const CurrentCueInfo &cueInfo, bool jassertWhenNotFound) {
    for (const auto& action: cueInfo.actions) {
        stopAction(action.ID, jassertWhenNotFound);
    }
}


bool OSCDeviceSender::connect() {
    return oscSender.connect(ipAddress, port);
}

bool OSCDeviceSender::disconnect() {
    return oscSender.disconnect();
}

OSCDeviceSender::~OSCDeviceSender() {
    disconnect();
}


void OSCDeviceSelectorComponent::initaliseComponents() {
    ipAddressTextEditor.setTextToShowWhenEmpty("XXX.XXX.XXX.XXX", UICfg::TEXT_COLOUR_DARK);
    ipAddressTextEditor.addListener(this);
    portTextEditor.setTextToShowWhenEmpty("0-65535", UICfg::TEXT_COLOUR_DARK);
    portTextEditor.addListener(this);
    deviceNameTextEditor.setTextToShowWhenEmpty("Device Name", UICfg::TEXT_COLOUR_DARK);
    deviceNameTextEditor.addListener(this);

    inputErrors.setReadOnly(true);
    inputErrors.setMultiLine(true);
    inputErrors.setColour(TextEditor::backgroundColourId, UICfg::TEXT_EDITOR_BG_COLOUR);

    // Don't use Look and Feel for colours as applyButton and cancelButton use different colours
    applyButton.setColour(TextButton::buttonColourId, UICfg::POSITIVE_BUTTON_COLOUR);
    applyButton.setColour(TextButton::buttonOver, UICfg::POSITIVE_OVER_BUTTON_COLOUR);
    applyButton.addListener(this);
    cancelButton.setColour(TextButton::buttonColourId, UICfg::NEGATIVE_BUTTON_COLOUR);
    cancelButton.setColour(TextButton::buttonOver, UICfg::NEGATIVE_OVER_BUTTON_COLOUR);
    cancelButton.addListener(this);

    ipAddressTextEditor.setJustification(Justification::centredLeft);
    portTextEditor.setJustification(Justification::centredLeft);
    deviceNameTextEditor.setJustification(Justification::centredLeft);

    addAndMakeVisible(applyButton);
    addAndMakeVisible(cancelButton);
    addAndMakeVisible(ipAddressTextEditor);
    addAndMakeVisible(portTextEditor);
    addAndMakeVisible(deviceNameTextEditor);
    addAndMakeVisible(inputErrors);
}


void OSCDeviceSelectorComponent::resized() {
    resizeReady = false;

    auto winBounds = getLocalBounds();

    // Ok, let's go one by one.
    // First, let's do the area of the window title. Let's also use relative sizes
    // to make it easier to resize the window.
    backgroundImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(backgroundImage);
    g.fillAll(UICfg::BG_COLOUR);
    g.setColour(UICfg::TEXT_COLOUR);

    Rectangle<int> tempBox;

    // Deal with Padding
    auto contentBounds = winBounds;
    auto standardPaddingAsPixels = UICfg::STD_PADDING * winBounds.getHeight();
    contentBounds.removeFromBottom(standardPaddingAsPixels);
    contentBounds.removeFromTop(standardPaddingAsPixels);
    contentBounds.removeFromLeft(standardPaddingAsPixels);
    contentBounds.removeFromRight(standardPaddingAsPixels);

    // One tenth of the window width and height
    int widthTenth = contentBounds.getWidth() / 10;
    int heightTenth = contentBounds.getHeight() / 10;


    // Title
    auto titleArea = contentBounds.removeFromTop(heightTenth);
    auto titleFont = FontOptions(
        UICfg::DEFAULT_SANS_SERIF_FONT_NAME,
        titleArea.getHeight() / 2.f,
        Font::bold);
    g.setFont(titleFont);
    g.drawFittedText(
        "OSC Device Selector", titleArea.toNearestInt(),
        Justification::centredLeft, 1);

    // Input boxes fonts
    auto inputBoxFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, heightTenth * 0.9, Font::plain);
    auto errorBoxFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, heightTenth * 0.4, Font::plain);

    ipAddressTextEditor.setFont(inputBoxFont);
    portTextEditor.setFont(inputBoxFont);
    deviceNameTextEditor.setFont(inputBoxFont);
    inputErrors.setFont(errorBoxFont);


    // There is a bug where the font size is not set correctly when the window is resized for text already painted.
    // So, let's clear the text editor text, and set the text again
    ipAddressTextEditor.setText("");
    ipAddressTextEditor.setText(ipAddressString);
    portTextEditor.setText("");
    portTextEditor.setText(portString);
    deviceNameTextEditor.setText("");
    deviceNameTextEditor.setText(deviceNameString);


    // Now IP address bar and Port bar
    // The height of these bars are 2/10 of the window heightFont
    // The width for IP Addr. is 7/10, the width for Port is 3/10
    // Labels
    tempBox = contentBounds.removeFromTop(heightTenth);
    auto ipAddrLabelBox = tempBox.removeFromLeft(widthTenth * 7);
    auto portLabelBox = tempBox;

    g.drawFittedText(
        "IP Address",
        ipAddrLabelBox.toNearestInt(), Justification::centredLeft, 1);
    g.drawFittedText("Port",
        portLabelBox.toNearestInt(), Justification::centredLeft, 1);


    // Boxes
    tempBox = contentBounds.removeFromTop(heightTenth * 2);
    auto ipAddrBox = tempBox.removeFromLeft(widthTenth * 7);
    auto portBox = tempBox;
    ipAddressTextEditor.setBounds(ipAddrBox);
    portTextEditor.setBounds(portBox);


    // Device name bar
    auto deviceNameLabelBox = contentBounds.removeFromTop(heightTenth);
    auto deviceNameBox = contentBounds.removeFromTop(heightTenth * 2);
    g.drawFittedText(
        "Device Name",
        deviceNameLabelBox.toNearestInt(), Justification::centredLeft, 1);
    deviceNameTextEditor.setBounds(deviceNameBox);

    // 1/10 of the window height for padding
    contentBounds.removeFromTop(heightTenth);

    // Apply and Cancel buttons
    applyButton.setBounds(contentBounds.removeFromLeft(widthTenth * 2));
    contentBounds.removeFromLeft(widthTenth);
    cancelButton.setBounds(contentBounds.removeFromLeft(widthTenth * 2));
    contentBounds.removeFromLeft(widthTenth);

    // Input errors box
    inputErrors.setBounds(contentBounds.removeFromTop(heightTenth * 2));


    resizeReady = true;
}


bool OSCDeviceSelectorComponent::validateTextEditorOutputs() {
    inputErrorsString = String();
    bool noError = true;

    // Check if IP is valid
    auto validatorOut = isValidIPv4(ipAddressString);
    if (!validatorOut.isValid) {
        inputErrorsString << "Invalid IP Address: " << validatorOut.errorMessage << "\n";
        noError = false;
    }

    // Check if the port is valid
    validatorOut = isValidPort(portString);
    if (!validatorOut.isValid) {
        inputErrorsString << "Invalid Port: " << validatorOut.errorMessage << "\n";
        noError = false;
    }

    // Check if the device name is valid
    validatorOut = isValidDeviceName(deviceNameString);
    if (!validatorOut.isValid) {
        inputErrorsString << "Invalid Device Name: " << validatorOut.errorMessage << "\n";
        noError = false;
    }

    return noError;
}
