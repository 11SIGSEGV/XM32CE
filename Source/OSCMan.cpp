/*
  ==============================================================================

    OSCMan.cpp
    Created: 11 May 2025 6:46:13pm
    Author:  anony

  ==============================================================================
*/

#include "OSCMan.h"

OSCDeviceSender::OSCDeviceSender(String &ipAddress, String &port, String &deviceName) {
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

OSCDeviceSender::OSCDeviceSender(String &ipAddress, int port, String &deviceName) {
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


bool OSCDeviceSender::connect() {
    return oscSender.connect(ipAddress, port);
}

bool OSCDeviceSender::disconnect() {
    return oscSender.disconnect();
}

OSCDeviceSender::~OSCDeviceSender() {
    disconnect();

}
