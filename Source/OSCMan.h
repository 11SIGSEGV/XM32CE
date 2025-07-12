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
#include "AppComponents.h"



struct OSCDevice {
    String ipAddress;
    int port{};
    String deviceName;
};




class OSCDispatcherListener {
public:
    virtual ~OSCDispatcherListener() = default;

    /* Called when event from OSCDispatchManager needs to be relayed to the caller.
    */
    virtual void actionFinished(std::string) = 0;
};


class OSCDeviceSender {
public:
    OSCDeviceSender(const String &ipAddress, const String &port, const String &deviceName);

    OSCDeviceSender(const String &ipAddress, int port, const String &deviceName);

    explicit OSCDeviceSender(const OSCDevice &device);

    ~OSCDeviceSender();


    String getIPAddress() { return ipAddress; }

    /* Attempts to connect to OSC Device. If the connection is successful, it returns true. Otherwise, returns false.*/
    bool connect();

    /* Attempts to disconnect from OSC Device. If the disconnection is successful, it returns true. Otherwise, returns false.*/
    bool disconnect();

    // template <typename Arg1, typename ... Args>
    // bool sendMessage(String &path, Arg1 &&arg1, Args &&... args) {
    //     OSCMessage message(path, std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    //     return oscSender.send(message);
    // }


    /* Accepts path with embedded arguments (X32Maps::ArgumentEmbeddedPath), then using provided argument values
     * (X32Maps::ValueStorerArray), it will fill in the path with the values provided.
     * Does NOT do type checking for NonIter types (e.g., int vs string) - if the ValueStorer values provided are not
     * the same type as the NonIter type in the embedded argument path, it will NOT throw an exception...
     */
    static String fillInArgumentsOfEmbeddedPath(const ArgumentEmbeddedPath &path, const ValueStorerArray &pthArgVal);


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
    OSCSingleActionDispatcher(CueOSCAction cueAction, OSCDeviceSender &oscDevice, const String &jobName = "",
                              int oatFadeMillisecondsMinimumIterationDuration = 50): ThreadPoolJob(jobName),
        oscSender(oscDevice), cueAction(cueAction), FMMID(oatFadeMillisecondsMinimumIterationDuration) {
    }

    JobStatus runJob() override;

    ~OSCSingleActionDispatcher() override {
        // Try end the job
        signalJobShouldExit();
    };

private:
    const unsigned int FMMID; // The minimum duration passed before next increment for OAT_FADE actions (ms)
    CueOSCAction cueAction;
    OSCDeviceSender &oscSender; // The OSC Device Sender to use for sending messages

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCSingleActionDispatcher)
};


class OSCCueDispatcherManager : public Thread, public Thread::Listener {
public:
    explicit OSCCueDispatcherManager(OSCDeviceSender &oscDevice, unsigned int maximumSimultaneousMessageThreads = 100,
                                     unsigned int waitFormsWhenActionQueueIsEmpty = 50);




    ~OSCCueDispatcherManager() override {
        singleActionDispatcherPool.removeAllJobs(true, 1000);
    }


    void exitSignalSent() override {
        removeListener(this);
        actionQueue.emplace(true);
        singleActionDispatcherPool.removeAllJobs(true, 5000);
    }

    void run() override;

    // Registers a listener for OSCDispatcherManager events. These listeners will receive a callback when an event
    // occurs from the OSCCueDispatcherManager.
    void registerListener(OSCDispatcherListener *lstnr) { dispatchListeners.push_back(lstnr); }
    void unregisterListener(OSCDispatcherListener *lstnr) {
        dispatchListeners.erase(
            std::remove(dispatchListeners.begin(), dispatchListeners.end(), lstnr),
            dispatchListeners.end());
    }

    void addCueToMessageQueue(const CueOSCAction &cueAction);

    void addCueToMessageQueue(const CurrentCueInfo &cueInfo);

    void stopAction(const std::string &actionID, bool jassertWhenNotFound = false);

    void stopAction(const CueOSCAction& cueAction, bool jassertWhenNotFound = false) {
        stopAction(cueAction.ID, jassertWhenNotFound);
    }

    void stopAllActionsInCCI(const CurrentCueInfo &cueInfo, bool jassertWhenNotFound = false);

private:
    std::vector<OSCDispatcherListener*> dispatchListeners;
    std::unordered_map<std::string, OSCSingleActionDispatcher*> actionIDToJobMap; // Maps action ID to the job pointer
    std::queue<CueOSCAction> actionQueue;
    const unsigned int maximumSimultaneousMessageThreads;
    const unsigned int waitMSFromWhenActionQueueIsEmpty; // Time to wait when action queue is empty
    OSCDeviceSender &oscSender; // The OSC Device Sender to use for sending messages
    ThreadPool singleActionDispatcherPool; // Pool for single action dispatchers

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCCueDispatcherManager)
};


class OSCDeviceSelectorComponent : public Component, public TextEditor::Listener, public TextButton::Listener {
public:
    // UI for selecting OSC Device. Allows user to input IP and port.
    OSCDeviceSelectorComponent() {
        setOpaque(true);
        setSize(600, 400);
        initaliseComponents();
        Component::setVisible(true);
    }

    // Does a lot of repetitive initalisations.
    void initaliseComponents();


    // Remove all the references to this class as a listener
    ~OSCDeviceSelectorComponent() override {
        ipAddressTextEditor.removeListener(this);
        portTextEditor.removeListener(this);
        deviceNameTextEditor.removeListener(this);
        applyButton.removeListener(this);
        cancelButton.removeListener(this);
    }


    // Painting is mostly handled by child components and image draw upon resize
    void paint(Graphics &g) override {
        waitForResize();
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
        inputErrors.setText(inputErrorsString);
    }


    // Called upon window resize. Handles bounding boxes and draws an image of the window to
    // avoid paint() from having to redraw everything.
    void resized() override;


    // Listener for when text editor is changed.
    void textEditorTextChanged(TextEditor &editor) override {
        if (&editor == &ipAddressTextEditor)
            ipAddressString = ipAddressTextEditor.getText();
        else if (&editor == &portTextEditor)
            portString = portTextEditor.getText();
        else if (&editor == &deviceNameTextEditor)
            deviceNameString = deviceNameTextEditor.getText();
    }


    /* Returns true when inputs are valid, false when not.*/
    bool validateTextEditorOutputs();


    void textEditorReturnKeyPressed(TextEditor &editor) override {
        // If the ipAddressTextEditor is returned, then check if it's valid
        if (&editor == &ipAddressTextEditor)
            ipAddressString = ipAddressTextEditor.getText();
        else if (&editor == &portTextEditor)
            portString = portTextEditor.getText();
        else if (&editor == &deviceNameTextEditor)
            deviceNameString = deviceNameTextEditor.getText();
        else
            jassertfalse; // This should never happen

        // Now, call the validator
        validateTextEditorOutputs();
    }


    void buttonClicked(Button *button) override {
        if (button == &applyButton) {
            if (validateTextEditorOutputs())
                exitPopup();
        } else if (button == &cancelButton)
            exitPopup();
        else
            jassertfalse; // This should never happen
    }


    OSCDevice getDevice() {
        if (!validateTextEditorOutputs()) {
            return {};
        }
        OSCDevice device;
        device.ipAddress = ipAddressString;
        device.port = portString.getIntValue();
        device.deviceName = deviceNameString;
        return device;
    }


    void exitPopup() {
        exitModalState();
        getParentComponent()->userTriedToCloseWindow();
    }

private:
    TextEditor ipAddressTextEditor;
    TextEditor portTextEditor;
    TextEditor deviceNameTextEditor;

    String ipAddressString{};
    String portString{};
    String deviceNameString{};

    // This will be a read-only text editor that will show invalid inputs and reasons.
    TextEditor inputErrors;
    String inputErrorsString{};

    TextButton applyButton {"Apply", "Apply changes to OSC Device"};
    TextButton cancelButton {"Cancel", "Cancel changes to OSC Device"};

    /*
    std::vector<juce::Component *> getComps() {
        return {
            &ipAddressTextEditor, &portTextEditor, &deviceNameTextEditor,
            &inputErrors, &applyButton, &cancelButton
        };
    }
    */

    // As the window is resized, we need to resize the components. When size has been called, then we can paint.
    bool resizeReady = false;

    // This is where we will list all the bounding boxes, declared in resized().
    Image backgroundImage;


    void waitForResize(int waitLoops = 100) const {
        if (!resizeReady) {
            bool naturalExit = true;
            // Loop 100 times to wait for the resize to be ready. If not ready by end of for loop, return
            for (int i = 0; i < waitLoops; i++) {
                // ReSharper disable once CppDFAConstantConditions
                if (resizeReady) {
                    naturalExit = false;
                    break;
                }
            }
            // ReSharper disable once CppDFAConstantConditions
            if (naturalExit)
                DBG("Exited paint() due to resize not being ready");
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCDeviceSelectorComponent)
};


class OSCDeviceSelectorWindow : public DocumentWindow {
public:
    explicit OSCDeviceSelectorWindow(const String &name = "OSC Device Selector")
        : DocumentWindow(name,
                         Desktop::getInstance().getDefaultLookAndFeel()
                         .findColour(ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons) {
        oscDeviceSelectorComponent.reset(new OSCDeviceSelectorComponent());
        setUsingNativeTitleBar(true);
        setFullScreen(false);
        setSize(1280, 540);
        setContentOwned(oscDeviceSelectorComponent.get(), true);
        setResizable(true, true);
        setResizeLimits(640, 270, 1920, 1080);
        setAlwaysOnTop(true);


        #if JUCE_IOS || JUCE_ANDROID
                        setFullScreen (true);
        #else
                setResizable(true, true);
                centreWithSize(getWidth(), getHeight());
        #endif

        setVisible(true);
    }


    void closeButtonPressed() override {
        returnedOSCDev = oscDeviceSelectorComponent->getDevice();
        if (!returnedOSCDev.deviceName.isEmpty()) {
            // Means valid OSC Device was inputted
            validOSCDevice = true;
        }
        setVisible(false);
        if (oscDeviceSelectorComponent != nullptr) {
            removeChildComponent(oscDeviceSelectorComponent.get());
            oscDeviceSelectorComponent.release();
        }
    }

private:
    String returnedIPAddr{};
    String returnedPort{};
    String returnedDeviceName{};

    std::unique_ptr<OSCDeviceSelectorComponent> oscDeviceSelectorComponent;
    OSCDevice returnedOSCDev{};
    bool validOSCDevice{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCDeviceSelectorWindow)
};
