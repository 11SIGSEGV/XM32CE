/*
  ==============================================================================

    OSCMan.h
    Created: 11 May 2025 6:46:13pm
    Author:  anony

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Helpers.h"

struct OSCDevice {
    String ipAddress;
    int port;
    String deviceName;
};





class OSCDeviceSender {
public:
    OSCDeviceSender(OSCDevice device);
    OSCDeviceSender(String &ipAddress, String &port, String &deviceName);
    OSCDeviceSender(String &ipAddress, int port, String &deviceName);

    /* Attempts to connect to OSC Device. If the connection is successful, it returns true. Otherwise, returns false.*/
    bool connect();

    /* Attempts to disconnect from OSC Device. If the disconnection is successful, it returns true. Otherwise, returns false.*/
    bool disconnect();

    // template <typename Arg1, typename ... Args>
    // bool sendMessage(String &path, Arg1 &&arg1, Args &&... args) {
    //     OSCMessage message(path, std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    //     return oscSender.send(message);
    // }


    ~OSCDeviceSender();

private:
    OSCSender oscSender;
    // OSCMessage
    String ipAddress;
    int port;
    String deviceName;
};