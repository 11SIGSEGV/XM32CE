#include "MainComponent.h"

//==============================================================================


MainComponent::MainComponent() {
    headerBar.registerListener(this);


    for (auto *comp: getComponents()) {
        addAndMakeVisible(*comp);
    }
}

MainComponent::~MainComponent() {
    removeAllChildren();
}


//==============================================================================
void MainComponent::paint(Graphics &g) {;
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

void MainComponent::commandOccured(ShowCommand cmd) {
    switch (cmd) {
        case ShowCommand::SHOW_NEXT_CUE:
            std::cout << "Next cue" << std::endl;
            jassertfalse; // NOT IMPLEMENTED
            break;
        case ShowCommand::SHOW_PREVIOUS_CUE:
            std::cout << "Previous cue" << std::endl;
            jassertfalse; // NOT IMPLEMENTED
            break;
        case ShowCommand::SHOW_PLAY:
            std::cout << "Play" << std::endl;
            activeShowOptions.currentCuePlaying = true;
            break;
        case ShowCommand::SHOW_STOP:
            std::cout << "Stop" << std::endl;
            activeShowOptions.currentCuePlaying = false;
            break;
        case ShowCommand::SHOW_NAME_CHANGE:
            std::cout << "Show Name Change" << std::endl;
            break;
        case ShowCommand::SHOW_CUE_INDEX_CHANGE:
            std::cout << "Show Cue Index Change" << std::endl;
            break;

        default:
            jassertfalse; // Invalid ShowCommand
    }
    for (auto *comp: callbackCompsUponActiveShowOptionsChanged) {
        comp->commandOccured(cmd);
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

    // Now draw icon images
    g.drawImage(stopIcon, stopButtonIconBox, RectanglePlacement::centred);
    g.drawImage(playIcon, playButtonIconBox, RectanglePlacement::centred);
    g.drawImage(upIcon, upButtonIconBox, RectanglePlacement::centred);
    g.drawImage(downIcon, downButtonIconBox, RectanglePlacement::centred);

    // For STOP and PLAY buttons, we need text labels
    g.setFont(stopButtonTextBox.getHeight());;
    g.drawFittedText("STOP", stopButtonTextBox.toNearestInt(), Justification::centredLeft, 1);
    g.drawFittedText("PLAY", playButtonTextBox.toNearestInt(), Justification::centredLeft, 1);



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
    // For each button, we also have to set the image bounds.
    stopButtonIconBox = stopBox.toFloat();
    // Padding of 0.1 of the width tenths for padding
    auto boundWidth0005 = boundWidthTenths * 0.05f;


    stopButtonIconBox.removeFromBottom(boundWidth0005);
    stopButtonIconBox.removeFromTop(boundWidth0005);
    stopButtonIconBox.removeFromLeft(boundWidth0005);
    stopButtonTextBox = stopButtonIconBox.removeFromRight(stopBox.getWidth() - 0.1f * boundWidthTenths - stopButtonIconBox.getHeight());
    stopButtonTextBox.removeFromLeft(boundWidth0005);
    stopButtonTextBox.removeFromRight(boundWidth0005);

    downButton.setBounds(downBox);
    downButtonIconBox = downBox.toFloat();
    // No need to set side padding for the down button as no text beside it. Using RectanglePlacement::centred will
    // auto-center it to the box
    downButtonIconBox.removeFromTop(boundWidth0005);
    downButtonIconBox.removeFromBottom(boundWidth0005);

    upButton.setBounds(upBox);
    upButtonIconBox = upBox.toFloat();
    upButtonIconBox.removeFromTop(boundWidth0005);
    upButtonIconBox.removeFromBottom(boundWidth0005);

    playButton.setBounds(playBox);
    playButtonIconBox = playBox.toFloat();
    playButtonIconBox.removeFromTop(boundWidth0005);
    playButtonIconBox.removeFromBottom(boundWidth0005);
    playButtonIconBox.removeFromLeft(boundWidth0005);
    playButtonTextBox = playButtonIconBox.removeFromRight(playButton.getWidth() - 0.1f * boundWidthTenths - playButtonIconBox.getHeight());
    playButtonTextBox.removeFromLeft(boundWidth0005);
    playButtonTextBox.removeFromRight(boundWidth0005);

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


void HeaderBar::commandOccured(ShowCommand command) {
    if (_showCommandsRequiringImageReconstruction.find(command) != _showCommandsRequiringImageReconstruction.end()) {
        reconstructImage();
    }
}
