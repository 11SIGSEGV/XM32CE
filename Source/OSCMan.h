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
using util::lang::indices;

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

    ~OSCDeviceSender();


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
    static std::vector<OSCArgument> compileOSCArguments(std::vector<OSCMessageArguments> &args,
                                                        ValueStorerArray &argVals);

    void send(OSCMessage &message) {
        oscSender.send(message);
    }


private:
    OSCSender oscSender;
    // OSCMessage
    String ipAddress;
    int port;
    String deviceName;
};



class OSCSingleActionDispatcher : public ThreadPoolJob {
public:
    /* Constructor for OSCSingleActionDispatcher.
     * cueAction - The CueOSCAction to dispatch.
     * oscDevice - The OSCDeviceSender to use for sending messages.free(): invalid size
     * jobName - The name of the job, used for debugging and logging.
     * oatFadeMillisecondsMinimumIterationDuration - The minimum duration for each iteration of the fade in milliseconds.
     *  Basically acts like a frame limiter.
     */
    OSCSingleActionDispatcher(CueOSCAction &cueAction, OSCDeviceSender &oscDevice, String jobName = "",
        int oatFadeMillisecondsMinimumIterationDuration = 50):
    ThreadPoolJob(jobName), oscSender(oscDevice), cueAction(cueAction), FMMID(oatFadeMillisecondsMinimumIterationDuration) {
    }

    JobStatus runJob() override;

    ~OSCSingleActionDispatcher() override {
        // Try end the job
        signalJobShouldExit();
    };


private:
    const unsigned int FMMID; // The minimum duration passed before next increment for OAT_FADE actions (ms)
    CueOSCAction &cueAction;
    OSCDeviceSender &oscSender; // The OSC Device Sender to use for sending messages

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCSingleActionDispatcher)
};


class OSCCueDispatcherManager : public Thread, public Thread::Listener {
public:
    OSCCueDispatcherManager(OSCDeviceSender &oscDevice, unsigned int maximumSimultaneousMessageThreads = 100,
                            unsigned int waitFormsWhenActionQueueIsEmpty = 50);



    ~OSCCueDispatcherManager() override {
        stopThread(5000); // Wait 5s to finish any ongoing tasks
        messageDispatcherPool.removeAllJobs(true, 5000);
    }


    void exitSignalSent() override {
        actionQueue.push(_stopThread);
        stopThread(5000);
        messageDispatcherPool.removeAllJobs(true, 5000);
    }

    void run() override;

    void addCueToMessageQueue(CueOSCAction cueAction);

    void addCueToMessageQueue(CurrentCueInfo cueInfo);

private:
    const CueOSCAction _stopThread {true}; // run() only checks _stopThread.oat == action.oat
    ThreadPool messageDispatcherPool;
    TSQueue<CueOSCAction> actionQueue;
    const unsigned int maximumSimultaneousMessageThreads;
    const unsigned int waitFormsWhenActionQueueIsEmpty; // Time to wait when action queue is empty
    OSCDeviceSender &oscSender; // The OSC Device Sender to use for sending messages
    ThreadPool singleActionDispatcherPool; // Pool for single action dispatchers
    ThreadID threadID; // ID of the thread running the dispatcher manager

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCCueDispatcherManager)
};
