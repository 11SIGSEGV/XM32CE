#include "MainComponent.h"

//==============================================================================


MainComponent::MainComponent() {
    activeShowOptions.loadCueValuesFromCCIVector(cuesInfo);
    headerBar.registerListener(this);
    commandOccurred(FULL_SHOW_RESET);

    for (auto *comp: getComponents()) {
        addAndMakeVisible(*comp);
    }
}

MainComponent::~MainComponent() {
    dispatcher.stopThread(5000);
    removeAllChildren();
}


void MainComponent::paint(Graphics &g) {;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));


    g.setFont(FontOptions(16.0f));
    g.setColour(Colours::white);
    g.drawText("Kewei's bad!", getLocalBounds(), Justification::centred, true);
}



void MainComponent::resized() {
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    auto bounds = getLocalBounds();
    headerBar.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.05f));
    sidePanel.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.2f));
}


void MainComponent::updateActiveShowOptionsFromCCIIndex(int newIndex) {
    auto ccisInfoSize = cuesInfo.size();
    if (ccisInfoSize == 0) {
        // Set cue info to default
        activeShowOptions.currentCueIndex = 0;
        activeShowOptions.currentCueID = "";
        activeShowOptions.currentCuePlaying = false;
        activeShowOptions.numberOfCueItems = 0;
        return;
    }
    if (newIndex < 0 || newIndex >= ccisInfoSize) {
        return; // Nothing to update - max index
    }
    auto cci = cuesInfo[newIndex];

    activeShowOptions.currentCueIndex = newIndex;
    activeShowOptions.currentCueID = cci.id;
    activeShowOptions.currentCuePlaying = cci.currentlyPlaying;
    activeShowOptions.numberOfCueItems = ccisInfoSize;
}


void MainComponent::setPlayStatusForCurrentCue(bool isPlaying) {
    if (activeShowOptions.numberOfCueItems == 0) {
        return;
    }
    activeShowOptions.currentCuePlaying = isPlaying;
    cuesInfo[activeShowOptions.currentCueIndex].currentlyPlaying = isPlaying;
}



void MainComponent::commandOccurred(ShowCommand cmd) {
    switch (cmd) {
        case SHOW_NEXT_CUE:
            updateActiveShowOptionsFromCCIIndex(activeShowOptions.currentCueIndex + 1);
            break;
        case SHOW_PREVIOUS_CUE:
            updateActiveShowOptionsFromCCIIndex(activeShowOptions.currentCueIndex - 1);
            break;
        case SHOW_PLAY:
            setPlayStatusForCurrentCue(true);
            break;
        case SHOW_STOP:
            setPlayStatusForCurrentCue(false);
            break;
        case SHOW_NAME_CHANGE:
            std::cout << "Show Name Change" << std::endl;
            break;
        case FULL_SHOW_RESET:
            break; // Don't need to do anything, but we still need to broadcast it to all callbacks.'
        default:
            jassertfalse; // Invalid ShowCommand
    }
    for (auto *comp: callbackCompsUponActiveShowOptionsChanged) {
        comp->commandOccurred(cmd);
    }
}

// ==========================================================================


CCISidePanel::CCISidePanel(ActiveShowOptions &activeShowOptions, std::vector<CurrentCueInfo>& currentCueInfos):
    activeShowOptions(activeShowOptions), currentCueInfos(currentCueInfos) {
}


void CCISidePanel::constructImage() {
    panelImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(panelImage);

    g.fillAll(UICfg::BG_COLOUR);

    g.setColour(UICfg::TEXT_COLOUR);
    g.setFont(UICfg::DEFAULT_FONT);

    if (activeShowOptions.numberOfCueItems == 0) {
        return; // Don't draw nothing!
     }

    // Assumes currentCueIndex is valid!
    auto currentCueInfo = currentCueInfos[activeShowOptions.currentCueIndex];

    // Cue Name
    g.setFont(cueNameBox.getHeight()*0.7);
    // Figure out string width
    // If the name is too long, then scale down and use two lines
    if (GlyphArrangement::getStringWidthInt(g.getCurrentFont(), currentCueInfo.name) <= cueNameBox.getWidth()) {
        g.drawFittedText(currentCueInfo.name, cueNameBox.toNearestInt(), Justification::bottomLeft, 1);
    } else {
        g.setFont(cueNameBox.getHeight()*0.4);
        g.drawFittedText(currentCueInfo.name, cueNameBox.toNearestInt(), Justification::bottomLeft, 2);
    }


    // Cue Description
    g.setFont(cueDescriptionBox.getHeight()*0.15);
    g.drawFittedText(currentCueInfo.description, cueDescriptionBox.toNearestInt(), Justification::topLeft, 6);


    auto stoppedPlayingIndicatorBoxWidth = stoppedPlayingIndicatorBox.getWidth();
    auto stoppedPlayingIndicatorBoxHeight = stoppedPlayingIndicatorBox.getHeight();
    // Draw for Playing
    playingIndicatorImage = Image(Image::ARGB, stoppedPlayingIndicatorBoxWidth,
        stoppedPlayingIndicatorBoxHeight, true);
    Graphics pII(playingIndicatorImage);

    pII.fillAll(UICfg::POSITIVE_BUTTON_COLOUR);
    pII.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    pII.setFont(stoppedPlayingIndicatorBoxHeight * 0.7);
    pII.setColour(UICfg::TEXT_COLOUR);
    pII.drawFittedText("PLAYING",
        0, 0, stoppedPlayingIndicatorBoxWidth, stoppedPlayingIndicatorBoxHeight,
        Justification::centred, 1);

    // Draw for Stopped
    stoppedIndicatorImage = Image(Image::ARGB, stoppedPlayingIndicatorBoxWidth,
    stoppedPlayingIndicatorBoxHeight, true);
    Graphics sII(stoppedIndicatorImage);

    sII.fillAll(UICfg::NEGATIVE_BUTTON_COLOUR);
    sII.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    sII.setFont(stoppedPlayingIndicatorBoxHeight * 0.7);
    sII.setColour(UICfg::TEXT_COLOUR);
    sII.drawFittedText("STOPPED",
        0, 0, stoppedPlayingIndicatorBoxWidth, stoppedPlayingIndicatorBoxHeight,
        Justification::centred, 1);

}



void CCISidePanel::resized() {
    if (localBoundsIsInvalid())
        return;

    auto bounds = getLocalBounds();
    auto padding = UICfg::STD_PADDING * bounds.getWidth();
    bounds.removeFromTop(padding);
    bounds.removeFromLeft(padding);
    bounds.removeFromBottom(padding);
    bounds.removeFromRight(padding);


    auto heightTenths = bounds.getHeight() * 0.1f;
    auto widthTenths = bounds.getWidth() * 0.1f;

    stoppedPlayingIndicatorBox = Rectangle<double>(
        bounds.getRight() - widthTenths * 2.5, bounds.getY(), widthTenths * 2, heightTenths * 0.3).toNearestInt();


    cueNameBox = bounds.removeFromTop(heightTenths);
    // Padding
    bounds.removeFromTop(padding);
    // Description
    cueDescriptionBox = bounds.removeFromTop(heightTenths * 2);
    // Padding
    bounds.removeFromTop(padding);

    constructImage();
}


void CCISidePanel::paint(Graphics &g) {
    g.drawImage(panelImage, getLocalBounds().toFloat());
    if (activeShowOptions.currentCuePlaying) {
        g.drawImage(playingIndicatorImage, stoppedPlayingIndicatorBox.toFloat());
    } else {
        g.drawImage(stoppedIndicatorImage, stoppedPlayingIndicatorBox.toFloat());
    }
}


void CCISidePanel::commandOccurred(ShowCommand command) {
    switch (command) {
        case SHOW_PLAY: case SHOW_STOP:
            repaint();
            break;
        case SHOW_NEXT_CUE: case SHOW_PREVIOUS_CUE:
            constructImage();
            repaint();
            break;
        case SHOW_NAME_CHANGE:
            break;
        case FULL_SHOW_RESET:
            break;
        case CURRENT_CUE_ID_CHANGE:
            break;
    }
}


// ==========================================================================

void HeaderBar::reconstructButtonBackgroundImage() {
    if (localBoundsIsInvalid())
        return;

    buttonsBGImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);

    Graphics gBG(buttonsBGImage);


    gBG.setColour(stopButton.isEnabled() ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(stopBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(stopBox, 1);

    gBG.setColour(downButton.isEnabled() ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(downBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(downBox, 1);

    gBG.setColour(upButton.isEnabled() ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(upBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(upBox, 1);

    gBG.setColour(playButton.isEnabled() ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(playBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(playBox, 1);
}



void HeaderBar::reconstructImage() {
    if (localBoundsIsInvalid())
        return;

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


    // Fill boxes manually
    g.setColour(UICfg::HEADER_BG_COLOUR);
    g.fillRect(showNameBox);
    g.fillRect(cueIDBox);
    g.fillRect(cueNoBox);
    g.fillRect(timeBox);

    // Box Borders
    g.setColour(UICfg::STRONG_BORDER_COLOUR);
    g.drawRect(showNameBox, 1);
    g.drawRect(cueIDBox, 1);
    g.drawRect(cueNoBox, 1);
    g.drawRect(timeBox, 1);



    // Buttons - we use a separate method for this.
    reconstructButtonBackgroundImage();


    // Now, let's draw the Showname, CueID and CueNo.
    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    g.setFont(fontSize);
    g.setColour(UICfg::TEXT_COLOUR);

    g.drawFittedText(activeShowOptions.showName, showNameTextBox.toNearestInt(), Justification::centredLeft, 1);
    g.drawFittedText("ID: " + String(activeShowOptions.currentCueID), cueIDTextBox.toNearestInt(), Justification::centredLeft, 1);
    g.drawFittedText("Cue " + String(activeShowOptions.currentCueIndex + 1) + "/" + String(activeShowOptions.numberOfCueItems),
                     cueNoTextBox.toNearestInt(), Justification::centredLeft, 1);



    // Button Foregrounds
    buttonsFGImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics gFG(buttonsFGImage);
    // Now draw icon images
    gFG.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    gFG.setFont(fontSize);;
    gFG.setColour(UICfg::TEXT_COLOUR);
    gFG.drawImage(stopIcon, stopButtonIconBox, RectanglePlacement::centred);
    gFG.drawImage(playIcon, playButtonIconBox, RectanglePlacement::centred);
    gFG.drawImage(upIcon, upButtonIconBox, RectanglePlacement::centred);
    gFG.drawImage(downIcon, downButtonIconBox, RectanglePlacement::centred);

    // For STOP and PLAY buttons, we need text labels
    gFG.setFont(stopButtonTextBox.getHeight());;
    gFG.drawFittedText("STOP", stopButtonTextBox.toNearestInt(), Justification::centredLeft, 1);
    gFG.drawFittedText("PLAY", playButtonTextBox.toNearestInt(), Justification::centredLeft, 1);

}


void HeaderBar::resized() {
    if (localBoundsIsInvalid())
        return;

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


    buttonsBox = Rectangle<int>(stopBox.getX(), stopBox.getY(),
        boundWidthTenths * 3, getLocalBounds().getHeight());


    // Let's set the button bounds
    stopButton.setBounds(stopBox);
    downButton.setBounds(downBox);
    upButton.setBounds(upBox);
    playButton.setBounds(playBox);


    // For each button, we also have to set the image bounds.
    // Padding of total 0.1 of the width tenths (i.e., 0.05 each side) for padding
    auto boundWidth0005 = boundWidthTenths * 0.05f;

    stopButtonIconBox = stopBox.toFloat();
    stopButtonIconBox.removeFromBottom(boundWidth0005);
    stopButtonIconBox.removeFromTop(boundWidth0005);
    stopButtonIconBox.removeFromLeft(boundWidth0005);
    stopButtonTextBox = stopButtonIconBox.removeFromRight(
        stopBox.getWidth() - 0.1f * boundWidthTenths - stopButtonIconBox.getHeight());
    stopButtonTextBox.removeFromLeft(boundWidth0005);
    stopButtonTextBox.removeFromRight(boundWidth0005);

    downButtonIconBox = downBox.toFloat();
    // No need to set side padding for the down button as no text beside it. Using RectanglePlacement::centred will
    // auto-center it to the box
    downButtonIconBox.removeFromTop(boundWidth0005);
    downButtonIconBox.removeFromBottom(boundWidth0005);

    upButtonIconBox = upBox.toFloat();
    upButtonIconBox.removeFromTop(boundWidth0005);
    upButtonIconBox.removeFromBottom(boundWidth0005);

    playButtonIconBox = playBox.toFloat();
    playButtonIconBox.removeFromTop(boundWidth0005);
    playButtonIconBox.removeFromBottom(boundWidth0005);
    playButtonIconBox.removeFromLeft(boundWidth0005);
    playButtonTextBox = playButtonIconBox.removeFromRight(
        playButton.getWidth() - 0.1f * boundWidthTenths - playButtonIconBox.getHeight());
    playButtonTextBox.removeFromLeft(boundWidth0005);
    playButtonTextBox.removeFromRight(boundWidth0005);

    reconstructImage();
}


void HeaderBar::paint(Graphics &g) {
    auto localBoundsToFloat = getLocalBounds().toFloat();
    g.drawImage(borderImage, localBoundsToFloat);
    g.drawImage(buttonsBGImage, localBoundsToFloat);
    g.drawImage(buttonsFGImage, localBoundsToFloat);


    // We'll need to manually draw the time.
    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    g.setFont(localBoundsToFloat.getHeight() * 0.8f); // Larger font size for the time text
    g.setColour(UICfg::TEXT_COLOUR);
    g.drawFittedText(getCurrentTimeAsFormattedString(), timeTextBox.toNearestInt(),
                     Justification::centred, 1);

}


void HeaderBar::commandOccurred(ShowCommand command) {
    if (activeShowOptions.numberOfCueItems == 0) {
        setButtonEnabled(upButton, false);
        setButtonEnabled(downButton, false);
        setButtonEnabled(playButton, false);
        setButtonEnabled(downButton, false);
    } else {
        switch (command) {
            case SHOW_PLAY:
                setButtonEnabled(playButton, false);
                setButtonEnabled(stopButton, true);
                break;
            case SHOW_STOP:
                setButtonEnabled(playButton, true);
                setButtonEnabled(stopButton, false);
                break;
            case SHOW_NEXT_CUE: case SHOW_PREVIOUS_CUE:
                if (activeShowOptions.currentCueIndex == 0) {
                    // First Cue Selected
                    setButtonEnabled(upButton, false);
                    setButtonEnabled(downButton, true);
                } else if ((activeShowOptions.currentCueIndex + 1) == activeShowOptions.numberOfCueItems) {
                    // Last Cue Selected
                    setButtonEnabled(upButton, true);
                    setButtonEnabled(downButton, false);
                } else {
                    setButtonEnabled(upButton, true);
                    setButtonEnabled(downButton, true);
                }
                // Now we're on another cue, we also need to set the play button state.
                setButtonEnabled(playButton, !activeShowOptions.currentCuePlaying);
                setButtonEnabled(stopButton, activeShowOptions.currentCuePlaying);
                break;
            case SHOW_NAME_CHANGE: case CURRENT_CUE_ID_CHANGE:
                break;
            case FULL_SHOW_RESET:
                // The requirement in this case is to reset the buttons enabled state. If we trigger a
                // next/previous cue command manually, it can reset all the buttons relevant to this
                // header bar.
                commandOccurred(SHOW_NEXT_CUE);
                break;
        }
    }

    if (_showCommandsRequiringImageReconstruction.find(command) != _showCommandsRequiringImageReconstruction.end()) {
        reconstructImage();
        repaint();
    } else if (
        _showCommandsRequiringButtonReconstruction.find(command) != _showCommandsRequiringButtonReconstruction.end()) {
        reconstructButtonBackgroundImage();
        repaint();
    }
}
