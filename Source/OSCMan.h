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
    OSCDeviceSender(const String &ipAddress, const String &port, const String &deviceName);

    OSCDeviceSender(const String &ipAddress, int port, const String &deviceName);

    explicit OSCDeviceSender(OSCDevice device);


    /* Attempts to connect to OSC Device. If the connection is successful, it returns true. Otherwise, returns false.*/
    bool connect();

    /* Attempts to disconnect from OSC Device. If the disconnection is successful, it returns true. Otherwise, returns false.*/
    bool disconnect();

    // template <typename Arg1, typename ... Args>
    // bool sendMessage(String &path, Arg1 &&arg1, Args &&... args) {
    //     OSCMessage message(path, std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    //     return oscSender.send(message);
    // }

    // Converts argument embedded path and arguments into actual message
    static OSCMessage compileMessageFromArgumentEmbeddedPathAndOSCMessageArguments(
        ArgumentEmbeddedPath &path,
        ValueStorerArray &pathArgumentValues,
        std::vector<OSCArgument> &arguments,
        ValueStorerArray &argumentValues);

    /* Accepts path with embedded arguments (X32Maps::ArgumentEmbeddedPath), then using provided argument values
     * (X32Maps::ValueStorerArray), it will fill in the path with the values provided.
     * Does NOT do type checking for NonIter types (e.g., int vs string) - if the ValueStorer values provided are not
     * the same type as the NonIter type in the embedded argument path, it will NOT throw an exception...
     */
    static String fillInArgumentsOfEmbeddedPath(ArgumentEmbeddedPath &path, ValueStorerArray &pthArgVal);

    // Accepts Vector of Expected Arguments (i.e., templates) and Vector of ValueStore.
    static std::vector<OSCArgument> compileOSCArguments(std::vector<OSCMessageArguments> &args, ValueStorerArray &argVals);


    ~OSCDeviceSender();

private:
    OSCSender oscSender;
    // OSCMessage
    String ipAddress;
    int port;
    String deviceName;
};