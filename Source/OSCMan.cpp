/*
  ==============================================================================

    OSCMan.cpp
    Created: 11 May 2025 6:46:13pm
    Author:  anony

  ==============================================================================
*/

#include "OSCMan.h"

#include <complex>

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
OSCDeviceSender::OSCDeviceSender(OSCDevice device) {
    if (device.deviceName.isEmpty()) {
        throw std::invalid_argument("Empty OSCDevice");
    }
    this->ipAddress = device.ipAddress;
    this->port = device.port;
    this->deviceName = device.deviceName;
}

OSCDeviceSender::OSCDeviceSender(const String &ipAddress, int port, const String &deviceName) {
    auto ipv4AddrValidatorOut = isValidIPv4(ipAddress);
    if (!ipv4AddrValidatorOut.isValid) {
        throw std::invalid_argument(
            (String("IP address is not valid: ") + ipv4AddrValidatorOut.errorMessage).toStdString());
    }
    if (port < 0 || port > 65535) {
        throw std::invalid_argument("Port is within 0 and 65535");
    }
    auto deviceNameValidatorOut = isValidDeviceName(deviceName);
    if (!deviceNameValidatorOut.isValid) {
        throw std::invalid_argument(
            (String("Device name is not valid: ") + deviceNameValidatorOut.errorMessage).toStdString());
    }
    this->ipAddress = ipAddress;
    this->port = port;
    this->deviceName = deviceName;
}

OSCMessage OSCDeviceSender::compileMessageFromArgumentEmbeddedPathAndOSCMessageArguments(
    ArgumentEmbeddedPath &path, std::vector<ValueStorer> &pathArgumentValues, std::vector<OSCArgument> &arguments,
    std::vector<ValueStorer> &argumentValues) {
    /*This function will first form the path from the ArgumentEmbeddedPath using the values provided by the vector of ValueStorers
     *Then, using the list of message arguments expected, the function will format and correctly cast the appropriate
     *type to prepare for the OSC messsage to be sent.
     */
}


std::vector<OSCArgument> OSCDeviceSender::compileOSCArguments(
    std::vector<OSCMessageArguments> &args,
    ValueStorerArray &argVals) {
    std::vector<OSCArgument> oscArguments;
    if (args.size() != argVals.size()) {
        throw std::invalid_argument("Size of argument template and argument value vectors must be the same");
    }
    for (size_t i = 0; i < args.size(); ++i) {
        if (auto *optVal = std::get_if<OptionParam>(&args[i])) {
            // If it's an OptionParam, the value from the ValueStorer will be the string.
            // Do a quick check that the stringValue is a valid option
            if (std::find(optVal->value.begin(), optVal->value.end(), argVals[i].stringValue)
                == optVal->value.end()) {
                // Value not found in the options
                throw std::invalid_argument("Invalid option value");
            }
            oscArguments.emplace_back(String(argVals[i].stringValue));
        } else if (auto *enumVal = std::get_if<EnumParam>(&args[i])) {
            if (enumVal->value.size() < argVals[i].intValue) {
                throw std::out_of_range("Enum value out of range");
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
                        throw std::out_of_range("Integer value out of range");
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
                        throw std::out_of_range("Float value out of range");
                    }
                    oscArguments.emplace_back(argVals[i].floatValue);
                    break;
                }
                case STRING: {
                    // Check string length limitations and append
                    // Remember, intMax and intMin are the max and min length of the string
                    if (argVals[i].stringValue.length() < nonIter->intMin ||
                        argVals[i].stringValue.length() > nonIter->intMax) {
                        throw std::out_of_range("String value out of size range");
                    }
                    oscArguments.emplace_back(argVals[i].stringValue);
                    // TODO: Test if String() required to convert to juce::String instead of std::string
                    break;
                }
                case BITSET: {
                    // Check the size of the bitset
                    if (nonIter->intMax != argVals[i].stringValue.size()) {
                        throw std::out_of_range("Bitset value is not expected size");
                    }

                    // Convert string to bitset, then to int
                    oscArguments.emplace_back(std::stoi(argVals[i].stringValue, nullptr, 2));
                    break;
                }
                default:
                    throw std::invalid_argument("Unsupported ParamType for NonIter Parameter Template");
            }
        } else {
            throw std::invalid_argument("Unable to cast variant to valid parameter type for OSC Message arguments");
        }
    }
    return oscArguments;
}


String OSCDeviceSender::fillInArgumentsOfEmbeddedPath(ArgumentEmbeddedPath &path, std::vector<ValueStorer> &pthArgVal) {
    String finalString;

    // Loop through each element of path. If it's a string, directly append it to finalString. Otherwise,
    // fetch a value from pathArgumentValues and convert it to a string appendable format

    // Being by checks. Find number of in-path arguments.
    int argIndx{0};
    const int pathArgValsSize = pthArgVal.size();
    for (size_t i = 0; i < path.size(); ++i) {
        // First, check if path[i] exists, or if it's out of range
        if (argIndx >= pathArgValsSize) {
            throw std::out_of_range("Path argument index out of range");
        }
        if (auto *strVal = std::get_if<std::string>(&path[i])) {
            // If it's a string, append it directly
            finalString += String(*strVal);
        } else if (auto *optVal = std::get_if<OptionParam>(&path[i])) {
            if (argIndx < pthArgVal.size()) {
                // If it's an OptionParam, the value from the ValueStorer will be the string.
                // Do a quick check that the stringValue is a valid option
                if (std::find(optVal->value.begin(), optVal->value.end(), pthArgVal[argIndx].stringValue)
                    == optVal->value.end()) {
                    // Value not found in the options
                    throw std::invalid_argument("Invalid option value provided for the path");
                }
                finalString += String(pthArgVal[argIndx].stringValue);
                ++argIndx;
            } else
                throw std::out_of_range("Not enough path arguments provided for the path");
        } else if (auto *enumVal = std::get_if<EnumParam>(&path[i])) {
            // When the value is an enum, the ValueStore will have an int which the vector of the enum options will have
            // To check for this, check the int in the ValueStore is smaller than the size of the vector
            if (enumVal->value.size() < pthArgVal[argIndx].intValue) {
                throw std::out_of_range("Enum value out of range");
            }
            // As this is not a OPTIONS, we only need the index of the ENUM as the value.
            finalString += String(pthArgVal[argIndx].intValue);
            ++argIndx;
        } else if (auto *nonIter = std::get_if<NonIter>(&path[i])) {
            // Let's first determine if the value is int, float, string or bitset
            // The NonIter will indicate the type (_meta_PARAMTYPE)
            switch (nonIter->_meta_PARAMTYPE) {
                case ParamType::INT: {
                    // Get the value from the ValueStorer and append it to the finalString, after checking constraints
                    if (nonIter->intMin > pthArgVal[argIndx].intValue ||
                        nonIter->intMax < pthArgVal[argIndx].intValue) {
                        throw std::out_of_range("Integer value out of range");
                    }
                    finalString += String(pthArgVal[argIndx].intValue);
                    break;
                }
                case ParamType::STRING: {
                    // Check string length limitations and append
                    // Remember, intMax and intMin are the max and min length of the string
                    if (pthArgVal[argIndx].stringValue.length() < nonIter->intMin ||
                        pthArgVal[argIndx].stringValue.length() > nonIter->intMax) {
                        throw std::out_of_range("String value out of size range");
                    }
                    finalString += String(pthArgVal[argIndx].stringValue);
                    break;
                }
                case ParamType::BITSET: {
                    // Basically, it's already formatted 💀
                    // Just append it to the finalString...
                    // But first, check the size of the bitset
                    if (nonIter->intMax != pthArgVal[argIndx].stringValue.size()) {
                        throw std::out_of_range("Bitset value is not expected size");
                    }
                    // Append the bitset to the finalString
                    finalString += String(pthArgVal[argIndx].stringValue);
                    break;
                }
                default:
                    // Float not included in switch-case - should NEVER be used in Embedded Path Argument
                    throw std::invalid_argument(
                        "Unsupported ParamType for NonIter Parameter Template in Embedded Path");
            }
            ++argIndx;
        } else
            throw std::invalid_argument("Unable to cast variant to valid parameter type for in-path arguments");
    }

    // Check number of arguments was the same as the number of arguments in the path
    // No need for argIndx + 1, as it should have been incremented in the loop (i.e., incremeted after it was used, even if never used again)
    if (argIndx != pathArgValsSize) {
        DBG("Number of arguments in path and number of arguments provided do not match");
    }

    return finalString;
}

// Sends actual message. For performance sake, no checks are done here, so ensure the message is valid before calling this function.
ThreadPoolJob::JobStatus OSCSingleActionDispatcher::runJob() {
    if (cueAction.oat == OAT_COMMAND) {
        OSCMessage msg{cueAction.oscAddress};
        for (unsigned i: indices(cueAction.oatCommandOSCArgumentTemplate)) {
            auto argTemplate = cueAction.oatCommandOSCArgumentTemplate[i];
            auto valueStorer = cueAction.arguments[i];
            if (auto *optVal = std::get_if<OptionParam>(&argTemplate)) {
                // If it's an OptionParam, the value from the ValueStorer will be the string.
                msg.addString(cueAction.arguments[0].stringValue);
            } else if (auto *enumVal = std::get_if<EnumParam>(&argTemplate)) {
                // As this is not a OPTIONS, we only need the index of the ENUM as the value.
                msg.addInt32(valueStorer.intValue);
            } else if (auto *nonIter = std::get_if<NonIter>(&argTemplate)) {
                // Let's first determine if the value is int, float, string or bitset
                // The NonIter will indicate the type (_meta_PARAMTYPE)
                switch (nonIter->_meta_PARAMTYPE) {
                    case INT: {
                        msg.addInt32(valueStorer.intValue);
                        break;
                    }
                    case LINF:
                    case LOGF:
                    case LEVEL_161:
                    case LEVEL_1024: {
                        msg.addFloat32(valueStorer.floatValue);
                        break;
                    }
                    case STRING: {
                        msg.addString(valueStorer.stringValue);
                        break;
                    }
                    case BITSET: {
                        msg.addInt32(std::stoi(valueStorer.stringValue, nullptr, 2));
                        break;
                    }
                    default:
                        jassertfalse;
                        // Unsupported ParamType for NonIter Parameter Template. Is it a template ValueStorer (i.e., ParamType blank?)
                }
            } else {
                jassertfalse; // Invalid OSCMessageArguments type in the action
            }
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
                case LEVEL_161: case LEVEL_1024:
                    incrementedMsg.addFloat32(normalisedPercentage);
                    break;
                default:
                    incrementedMsg.addFloat32(static_cast<float>(inferValueFromMinMaxAndPercentage(
                        minVal, maxVal, normalisedPercentage, cueAction.oscArgumentTemplate._meta_PARAMTYPE)));
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
        // Finally, we need to send the end value to ensure the fade is complete... but to be safe, let's check if the
        // normalisedPercentage is not already at the end value.
        if (normalisedPercentage < normalisedEndPercentage) {
            OSCMessage endMsg{cueAction.oscAddress};
            switch (cueAction.oscArgumentTemplate._meta_PARAMTYPE) {
                case INT:
                    endMsg.addInt32(static_cast<int>(inferValueFromMinMaxAndPercentage(
                        minVal, maxVal, normalisedPercentage, LINF)));
                    break;
                case LEVEL_161: case LEVEL_1024:
                    endMsg.addFloat32(normalisedPercentage);
                    break;
                default:
                    endMsg.addFloat32(static_cast<float>(inferValueFromMinMaxAndPercentage(
                        minVal, maxVal, normalisedPercentage, cueAction.oscArgumentTemplate._meta_PARAMTYPE)));
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
                                                 unsigned int waitFormsWhenActionQueueIsEmpty): oscSender(oscDevice),
    maximumSimultaneousMessageThreads(maximumSimultaneousMessageThreads), Thread("oscCueDispatcherManager"),
    waitFormsWhenActionQueueIsEmpty(waitFormsWhenActionQueueIsEmpty) {
    if (maximumSimultaneousMessageThreads == 0 || maximumSimultaneousMessageThreads > 511) {
        jassertfalse; // Maximum simultaneous message threads must be above 1 and should be below 512.
    }
    startThread();

    setPriority(Priority::high); // Set a higher priority for the thread to ensure it processes messages quickly
}


void OSCCueDispatcherManager::addCueToMessageQueue(CurrentCueInfo cueInfo) {
    for (auto action: cueInfo.actions) {
        addCueToMessageQueue(action);
    }
}


void OSCCueDispatcherManager::addCueToMessageQueue(CueOSCAction cueAction) {
    actionQueue.push(cueAction);
}


void OSCCueDispatcherManager::run() {
    while (!threadShouldExit()) {
        auto action = actionQueue.pop();
        if (action.oat == _stopThread.oat) {
            break;
        }
        // Process the action here
        // For example, send the OSC message
        // oscSender.send(action.constructMessage());
        // Note: You need to implement the send method in your OSCDeviceSender class
        OSCSingleActionDispatcher singleActionDispatcher(action, oscSender);
        singleActionDispatcherPool.addJob(&singleActionDispatcher, true);
        // If no operations, wait for a short period to avoid busy waiting
        // ReSharper disable once CppExpressionWithoutSideEffects
        wait(waitFormsWhenActionQueueIsEmpty);
    }
    // Exit signal sent, clean up
    singleActionDispatcherPool.removeAllJobs(true, 5000); // Wait for all jobs to finish
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
