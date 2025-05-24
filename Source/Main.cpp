/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>

#include "Helpers.h"
#include "MainComponent.h"

//==============================================================================
class XM32CEApplication  : public JUCEApplication
{
public:

    //==============================================================================
    XM32CEApplication() {
        tests();
    }

    void tests() {
        testTooManyArguments();
        testOSCMessageArgumentCompiler();
        /*
        ArgumentEmbeddedPath sampleArgumentEmbeddedPath = {"/ch/", NonIter("chNum", "Channel Number", "Number of the Channel", 1, 1, 32), "/mix/fader"};
        ValueStorerArray sampleArgumentValues = {ValueStorer(2)};
        std::vector<OSCMessageArguments> test = {NonIter("chFdr", "Channel Fader", "Fader Level for a Channel", 0.f, ParamType::LEVEL_1024, 0.f, 1.f),};
        PathToArgumentMap samplePathToArgumentMap = {{&sampleArgumentEmbeddedPath, test}};
        ValueStorerArray sampleValueStorerArray = {ValueStorer(.5f)};
        auto tetCompiliation = testOscDevice.compileOSCArguments(test, sampleValueStorerArray);
        auto m = tetCompiliation[0];
        auto testPathComp = testOscDevice.fillInArgumentsOfEmbeddedPath(sampleArgumentEmbeddedPath, sampleArgumentValues);
        */
    }

    void testOSCMessageArgumentCompiler() {
        if (true) {
            try {
                std::vector<OSCMessageArguments> testArguments = {
                    NonIter("chFdr", "Channel Fader", "Fader Level for a Channel", 0.f, ParamType::LEVEL_1024, 0.f, 1.f),};
                ValueStorerArray sampleValueStorerArray = {ValueStorer(2)}; // Incorrect type, but no exception expected
                auto testCompiliation = testOscDevice.compileOSCArguments(testArguments, sampleValueStorerArray);

            } catch (std::out_of_range &e) {
                DBG("Caught exception: " << e.what());
            } catch (std::invalid_argument &e) {
                DBG("Caught exception: " << e.what());
            } catch (...) {
                DBG("Caught unknown exception");
            }
        }
    }

    void testTooManyArguments() {
        if (false) {
            try {
                ArgumentEmbeddedPath sampleArgumentEmbeddedPath = {"/ch/", NonIter("chNum", "Channel Number", "Number of the Channel", 1, 1, 32), "/mix/fader"};
                ValueStorerArray sampleArgumentValues = {}; // Too little arguments
                auto pathString = testOscDevice.fillInArgumentsOfEmbeddedPath(sampleArgumentEmbeddedPath, sampleArgumentValues);
                // This should throw an exception
            } catch (std::out_of_range &e) {
                DBG("Caught exception: " << e.what());
            } catch (std::invalid_argument &e) {
                DBG("Caught exception: " << e.what());
            } catch (...) {
                DBG("Caught unknown exception");
            }
        }
        if (false) {
            try {
                ArgumentEmbeddedPath sampleArgumentEmbeddedPath = {"/ch/", NonIter("chNum", "Channel Number", "Number of the Channel", 1, 1, 32), "/mix/fader"};
                ValueStorerArray sampleArgumentValues = {ValueStorer(2), ValueStorer(8)}; // Too many arguments
                auto pathString = testOscDevice.fillInArgumentsOfEmbeddedPath(sampleArgumentEmbeddedPath, sampleArgumentValues);
                // This should throw an exception
            } catch (std::out_of_range &e) {
                DBG("Caught exception: " << e.what());
            } catch (std::invalid_argument &e) {
                DBG("Caught exception: " << e.what());
            } catch (...) {
                DBG("Caught unknown exception");
            }
        }
        if (true) {
            try {
                std::vector<OSCMessageArguments> testArgumentTemplates = {NonIter("chFdr", "Channel Fader", "Fader Level for a Channel", 0.f, ParamType::LEVEL_1024, 0.f, 1.f),};
                ValueStorerArray sampleArgumentValues = {}; // Too little arguments
                auto pathString = testOscDevice.compileOSCArguments(testArgumentTemplates, sampleArgumentValues);
                // This should throw an exception
            } catch (std::out_of_range &e) {
                DBG("Caught exception: " << e.what());
            } catch (std::invalid_argument &e) {
                DBG("Caught exception: " << e.what());
            } catch (...) {
                DBG("Caught unknown exception");
            }
        }




    }

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        oscDevSelWin.reset(new OSCDeviceSelectorWindow("OSC Device Selector"));
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setFullScreen(true);
            setContentOwned (new MainComponent(), true);
            setResizable (true, true);
            setResizeLimits (600, 400, 10000, 10000);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);

        }


        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need."OSC Device Selector"
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<OSCDeviceSelectorWindow> oscDevSelWin;
    OSCDeviceSender testOscDevice = OSCDeviceSender {String("192.168.1.100"), 20023, String("Test")};

};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (XM32CEApplication)
