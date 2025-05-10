#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
    rotaryKnob.setRange (0.0, 1.0);
    rotaryKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    rotaryKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
    rotaryKnob.setBounds (10, 10, 180, 180);
    rotaryKnob.onValueChange = [this] {
        // create and send an OSC message with an address and a float value:
        if (!sender.send ("/ch/01/mix/fader", (float) rotaryKnob.getValue()))
            showConnectionErrorMessage ("Error: could not send OSC message.");
    };
    addAndMakeVisible (rotaryKnob);

    if (!sender.connect ("192.168.1.179", 10023))
        showConnectionErrorMessage ("Error: could not connect to UDP port 10023.");

    // showAlertWindow();
    // OSCDeviceSelectorWindow oscDeviceSelWindow =
    //     OSCDeviceSelectorWindow ("OSC Device Selector");
    // oscDeviceSelWindow.setVisible (true);
    // addAndMakeVisible(new OSCDeviceSelectorComponent());
}

MainComponent::~MainComponent()
{
}


void MainComponent::showAlertWindow() {
    auto *alertWindow = new ResizableWindow("OSC Device", Colours::black, false);
    // alertWindow->addTextBlock("Text block");
    // alertWindow->addTextEditor("Text editor", "Text editor");
    // alertWindow->addTextBlock("Press any button, or the escape key, to close the window");
    //
    // enum AlertWindowResult {
    //        noButtonPressed,
    //        button1Pressed,
    //        button2Pressed
    //    };
    //
    // alertWindow->addButton("Button 1", AlertWindowResult::button1Pressed);
    // alertWindow->addButton("Button 2", AlertWindowResult::button2Pressed);
    // RectanglePlacement placement{ RectanglePlacement::yMid | RectanglePlacement::xLeft | RectanglePlacement::doNotResize };
    // alertWindow->setBounds(placement.appliedTo(alertWindow->getBounds(), getBounds()));
    alertWindow->setVisible(true);
}



//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::FontOptions (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Kewei's bad!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
