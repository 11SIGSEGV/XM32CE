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
        if (!sender.send ("/ch/01/mix/fader", static_cast<float>(rotaryKnob.getValue())))
            showConnectionErrorMessage ("Error: could not send OSC message.");
    };

    if (!sender.connect ("127.0.0.1", 10023))
        showConnectionErrorMessage ("Error: could not connect to UDP port  o10023.");
    testRotary.setBounds(100, 300, 180, 180);
    testRotary2.setBounds(300, 300, 180, 180);
    testRotary3.setBounds(500, 300, 180, 180);
    testRotary4.setBounds(700, 300, 180, 180);
    testRotary5.setBounds(100, 500, 180, 180);

    for (auto *comp : getComponents())
    {
        addAndMakeVisible(*comp);
    }
    TextEditor textEditor;


}

MainComponent::~MainComponent() {
    // for (auto *comp : getComponents())
    // {
        // removeAllChildren();
    // }
    removeAllChildren();
    for (auto *comp : activeComps)
    {
        if (comp != nullptr)
            comp->setLookAndFeel(nullptr);
    }
    delete this;
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

