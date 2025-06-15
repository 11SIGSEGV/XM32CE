#include "MainComponent.h"

//==============================================================================


MainComponent::MainComponent() {
    headerBar.registerListener(this);
    setSize(600, 400);



    for (auto *comp: getComponents()) {
        addAndMakeVisible(*comp);
    }
}

MainComponent::~MainComponent() {
    removeAllChildren();
    // for (auto *comp: activeComps) {
    //     if (comp != nullptr)
    //         comp->setLookAndFeel(nullptr);
    // }
    delete this;
}


//==============================================================================
void MainComponent::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(juce::FontOptions(16.0f));
    g.setColour(juce::Colours::white);
    g.drawText("Kewei's bad!", getLocalBounds(), juce::Justification::centred, true);
}


void MainComponent::resized() {
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    auto bounds = getLocalBounds();
    headerBar.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.05f));

}


void MainComponent::localShowCommandReciever(ShowCommand command) {
    switch (command) {
        case ShowCommand::SHOW_STOP:
            // Handle stop command
            DBG("Stop!");;
            break;
        case ShowCommand::SHOW_START:
            // Handle play command
            DBG("Start!");
            break;
        case ShowCommand::SHOW_PREVIOUS_CUE:
            // Handle up command
            DBG("Previous Cue!");
            break;
        case ShowCommand::SHOW_NEXT_CUE:
            // Handle down command
            DBG("Next Cue!");
            break;
        default:
            jassertfalse; // Unknown command
    }
}



void HeaderBar::reconstructImage() {
    auto bounds = getLocalBounds();
    auto boundWidthTenths = bounds.getWidth() / 10;
    float fontSize = bounds.getHeight() * 0.6f;

    auto showNameTextBox = showNameBox.toFloat();
    showNameTextBox.removeFromLeft(boundWidthTenths * 0.05f);
    showNameTextBox.removeFromRight(boundWidthTenths * 0.05f);

    auto cueIDTextBox = cueIDBox.toFloat();
    cueIDTextBox.removeFromLeft(boundWidthTenths * 0.05f);
    cueIDTextBox.removeFromRight(boundWidthTenths * 0.05f);

    auto cueNoTextBox = cueNoBox.toFloat();
    cueNoTextBox.removeFromLeft(boundWidthTenths * 0.05f);
    cueNoTextBox.removeFromRight(boundWidthTenths * 0.05f);

    // Let's start drawing!
    borderImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(borderImage);
    // Box Borders
    g.fillAll(UICfg::HEADER_BG_COLOUR);
    g.setColour(UICfg::STRONG_BORDER_COLOUR);
    g.drawRect(showNameBox, 1);
    g.drawRect(cueIDBox, 1);
    g.drawRect(cueNoBox, 1);
    g.drawRect(stopBox, 1);
    g.drawRect(downBox, 1);
    g.drawRect(upBox, 1);
    g.drawRect(playBox, 1);
    g.drawRect(timeBox, 1);

    // Now, let's draw the Showname, CueID and CueNo.
    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    g.setFont(fontSize);
    g.setColour(UICfg::TEXT_COLOUR);
    g.drawFittedText(activeShowOptions.showName, showNameTextBox.toNearestInt(), Justification::centredLeft, 1);
    g.drawFittedText("ID: " + String(activeShowOptions.currentCueID), cueIDTextBox.toNearestInt(), Justification::centredLeft, 1);
    g.drawFittedText("Cue " + String(activeShowOptions.currentCueIndex) + "/" + String(activeShowOptions.numberOfCueItems),
                     cueNoTextBox.toNearestInt(), Justification::centredLeft, 1);

}


void HeaderBar::resized() {
    auto bounds = getLocalBounds();
    // The bounds passed should be the bounds of ONLY the intended header bar.
    auto boundWidthTenths = bounds.getWidth() / 10;
    float fontSize = bounds.getHeight() * 0.6f;

    showNameBox = bounds.removeFromLeft(boundWidthTenths * 2);
    cueIDBox = bounds.removeFromLeft(boundWidthTenths * 1.5f);
    cueNoBox = bounds.removeFromLeft(boundWidthTenths * 1.5f);
    stopBox = bounds.removeFromLeft(boundWidthTenths);
    downBox = bounds.removeFromLeft(boundWidthTenths * 0.5f);
    upBox = bounds.removeFromLeft(boundWidthTenths * 0.5f);
    playBox = bounds.removeFromLeft(boundWidthTenths);
    timeBox = bounds;  // Simply use remaining bounds.
    timeTextBox = timeBox.toFloat();
    timeTextBox.removeFromLeft(boundWidthTenths * 0.05f);
    timeTextBox.removeFromRight(boundWidthTenths * 0.05f);

    // Let's set the button bounds
    stopButton.setBounds(stopBox);
    downButton.setBounds(downBox);
    upButton.setBounds(upBox);
    playButton.setBounds(playBox);

    reconstructImage();
}


void HeaderBar::paint(Graphics &g) {
    g.drawImage(borderImage, getLocalBounds().toFloat());
    // We'll need to manually draw the time.
    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    g.setFont(getLocalBounds().getHeight() * 0.8f); // Larger font size for the time text
    g.setColour(UICfg::TEXT_COLOUR);
    g.drawFittedText(getCurrentTimeAsFormattedString(), timeTextBox.toNearestInt(),
                     Justification::centred, 1);

}
