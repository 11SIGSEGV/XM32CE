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

OSCMessage OSCDeviceSender::messageFromArgumentEmbeddedPathAndOSCMessageArguments(ArgumentEmbeddedPath &path, std::vector<ValueStorer> &pathArgumentValues, std::vector<OSCArgument> &arguments, std::vector<ValueStorer> &argumentValues) {
    /*This function will first form the path from the ArgumentEmbeddedPath using the values provided by the vector of ValueStorers
     *Then, using the list of message arguments expected, the function will format and correctly cast the appropriate
     *type to prepare for the OSC messsage to be sent.
     */
}


std::vector<OSCArgument> OSCDeviceSender::compileOSCArguments(std::vector<OSCMessageArguments> &args, ValueStorerArray &argVals) {
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
        }
        else if (auto *enumVal = std::get_if<EnumParam>(&args[i])) {
            if (enumVal->value.size() < argVals[i].intValue) {
                throw std::out_of_range("Enum value out of range");
            }
            // As this is not a OPTIONS, we only need the index of the ENUM as the value.
            oscArguments.emplace_back(argVals[i].intValue);
        }

        else if (auto *nonIter = std::get_if<NonIter>(&args[i])) {
            // Let's first determine if the value is int, float, string or bitset
            // The NonIter will indicate the type (_meta_PARAMTYPE)
            switch (nonIter->_meta_PARAMTYPE) {
                case ParamType::INT: {
                    // Get the value from the ValueStorer and append it to the finalString, after checking constraints
                    if (nonIter->intMin > argVals[i].intValue ||
                        nonIter->intMax < argVals[i].intValue) {
                        throw std::out_of_range("Integer value out of range");
                        }
                    oscArguments.emplace_back(argVals[i].intValue);
                    break;
                }
                case ParamType::LINF: case ParamType::LOGF: case ParamType::LEVEL_161: case ParamType::LEVEL_1024: {
                    if (nonIter->floatMin > argVals[i].floatValue ||
                        nonIter->floatMax < argVals[i].floatValue) {
                        throw std::out_of_range("Float value out of range");
                        }
                    oscArguments.emplace_back(argVals[i].floatValue);
                    break;
                }
                case ParamType::STRING: {
                    // Check string length limitations and append
                    // Remember, intMax and intMin are the max and min length of the string
                    if (argVals[i].stringValue.length() < nonIter->intMin ||
                        argVals[i].stringValue.length() > nonIter->intMax) {
                        throw std::out_of_range("String value out of size range");
                        }
                    oscArguments.emplace_back(argVals[i].stringValue); // TODO: Test if String() required to convert to juce::String instead of std::string
                    break;
                }
                case ParamType::BITSET: {
                    // Check the size of the bitset
                    if (nonIter->intMax != argVals[i].stringValue.size()) {
                        throw std::out_of_range("Bitset value is not expected size");
                    }
                    // Convert string to bitset, then to int
                    oscArguments.emplace_back(std::stoi(argVals[i].stringValue));
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
    int argIndx { 0 };
    for (size_t i = 0; i < path.size(); ++i) {
        if (auto *strVal = std::get_if<std::string>(&path[i])) {
            // If it's a string, append it directly
            finalString += String(*strVal);
        }

        else if (auto *optVal = std::get_if<OptionParam>(&path[i])) {
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
        }

        else if (auto *enumVal = std::get_if<EnumParam>(&path[i])) {
            // When the value is an enum, the ValueStore will have an int which the vector of the enum options will have
            // To check for this, check the int in the ValueStore is smaller than the size of the vector
            if (enumVal->value.size() < pthArgVal[argIndx].intValue) {
                throw std::out_of_range("Enum value out of range");
            }
            // As this is not a OPTIONS, we only need the index of the ENUM as the value.
            finalString += String(pthArgVal[argIndx].intValue);
            ++argIndx;
        }

        else if (auto *nonIter = std::get_if<NonIter>(&path[i])) {
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
                case ParamType::LINF: case ParamType::LOGF: {
                    if (nonIter->floatMin > pthArgVal[argIndx].floatValue ||
                        nonIter->floatMax < pthArgVal[argIndx].floatValue) {
                        throw std::out_of_range("Float value out of range");
                    }
                    finalString += String(pthArgVal[argIndx].floatValue);
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
                    throw std::invalid_argument("Unsupported ParamType for NonIter Parameter Template in Embedded Path");
            }
            ++argIndx;
        }

        else
            throw std::invalid_argument("Unable to cast variant to valid parameter type for in-path arguments");
    }
    return finalString;
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
