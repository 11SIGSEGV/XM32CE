#include "MainComponent.h"

//==============================================================================


MainComponent::MainComponent(): cueListModel(cueListBox, cueListData) {
    DBG("OSC Device Connected on " + oscDeviceSender.getIPAddress());

    dispatcher.startRealtimeThread(Thread::RealtimeOptions().withPriority(8));
    dispatcher.registerListener(this);
    activeShowOptions.loadCueValuesFromCCIVector(cciVector);
    headerBar.registerListener(this);
    cciVector.addListener(this);
    cueListData.addListener(this);
    // addKeyListener(this);
    getTopLevelComponent()->addKeyListener(this);
    setWantsKeyboardFocus(true);


    cueListBox.setModel(&cueListModel);

    commandOccurred(FULL_SHOW_RESET);

    for (auto *comp: activeComps) {
        addAndMakeVisible(*comp);
    }

    if (ID_TO_TEMPLATE_MAP.empty()) {
        generateIDTemplateMap();
    }

    auto uuid = uuidGen.generate();
    actionConstructorWindows[uuid].reset(new OSCActionConstructor(uuid, "test",
        CueOSCAction("/ch/2/preamp/trim", {Channel::TRIM.NONITER}, {ValueStorer(3.f)}, Channel::TRIM.ID)));
    actionConstructorWindows[uuid].get()->setParentListener(this);
}


void MainComponent::paint(Graphics &g) {
    g.drawImage(backgroundPrerender, getLocalBounds().toFloat());

    // g.setFont(FontOptions(16.0f));
    // g.setColour(Colours::white);
    // g.drawText("Kewei's bad!", getLocalBounds(), Justification::centred, true);
    // g.drawText("james's fat!", getLocalBounds(), Justification::centred, true);
}


void MainComponent::resized() {
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    int idealBorderThickness = std::ceil(getHeight() * UICfg::COMPONENT_OUTLINE_THICKNESS_PROPORTIONAL_TO_PARENT_HEIGHT);
    std::vector<Rectangle<int>> borderBounds;

    auto bounds = getLocalBounds();
    auto headerBarBounds = bounds.removeFromTop(bounds.getHeight() * 0.05f);

    borderBounds.push_back(bounds.removeFromTop(idealBorderThickness));

    auto sidePanelBounds = bounds.removeFromLeft(bounds.getWidth() * 0.25f);

    borderBounds.push_back(bounds.removeFromLeft(idealBorderThickness));

    auto cueListBounds = bounds;
    headerBar.setBounds(headerBarBounds);
    sidePanel.setBounds(sidePanelBounds);
    auto boxHeight = bounds.getHeight() * 0.05f;
    auto cueListHeaderBox = bounds.removeFromTop(boxHeight);
    cueListBox.setBounds(bounds);
    cueListBox.setRowHeight(boxHeight);


    // Prerender the background for the cue list area and the titles for the cue list.
    backgroundPrerender = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(backgroundPrerender);

    g.setColour(UICfg::TEXT_ACCENTED_COLOUR);
    for (auto border: borderBounds) {
        g.fillRect(border);
    }

    g.setColour(UICfg::BG_SECONDARY_COLOUR);
    g.fillRect(bounds);
    g.setColour(UICfg::BG_COLOUR);
    g.fillRect(cueListHeaderBox);

    Font headerFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, cueListHeaderBox.getHeight() * 0.6f, Font::bold);
    g.setFont(headerFont);
    g.setColour(UICfg::TEXT_ACCENTED_COLOUR);

    auto cueListHeaderBoxCopy = cueListHeaderBox;

    float bounds20ths = cueListHeaderBoxCopy.getWidth() * 0.05f;
    float padding = UICfg::STD_PADDING * cueListHeaderBoxCopy.getHeight() * 7;

    auto numBox = cueListHeaderBoxCopy.removeFromLeft(bounds20ths);
    auto idBox = cueListHeaderBoxCopy.removeFromLeft(bounds20ths * 3);
    auto nameBox = cueListHeaderBoxCopy.removeFromLeft(bounds20ths * 10);
    auto numberOfActionsBox = cueListHeaderBoxCopy.removeFromLeft(bounds20ths);
    auto stateBox = cueListHeaderBoxCopy.removeFromLeft(bounds20ths * 3);
    auto editBox = cueListHeaderBoxCopy;

    g.drawText("No", numBox.reduced(padding), Justification::centred, true);
    g.drawText("ID", idBox.reduced(padding), Justification::centredLeft, true);
    g.drawText("Name", nameBox.reduced(padding), Justification::centredLeft, true);
    g.drawText("#A", numberOfActionsBox.reduced(padding), Justification::centred, true);
    g.drawText("Status", stateBox.reduced(padding), Justification::centred, true);
    g.drawText("Edit", editBox.reduced(padding), Justification::centred, true);

}


void MainComponent::updateActiveShowOptionsFromCCIIndex(size_t newIndex) {
    auto ccisInfoSize = cciVector.getSize();
    if (ccisInfoSize == 0) {
        // Set cue info to default
        activeShowOptions.currentCueIndex = 0;
        activeShowOptions.currentCueID = "";
        activeShowOptions.currentCuePlaying = false;
        activeShowOptions.numberOfCueItems = 0;
        activeShowOptions.currentCueInternalID = "";
        return;
    }

    auto &cci = cciVector.getCurrentCueInfoByIndex(newIndex);
    if (cci.isInvalid()) { return; } // Nothing to change...

    activeShowOptions.currentCueIndex = newIndex;
    activeShowOptions.currentCueID = cci.id;
    activeShowOptions.currentCuePlaying = cci.currentlyPlaying;
    activeShowOptions.numberOfCueItems = ccisInfoSize;
    activeShowOptions.currentCueInternalID = cci.getInternalID();
}


void MainComponent::commandOccurred(ShowCommand cmd) {
    bool currentCueListItemRequiresRedraw = false;
    // We should only redraw when in the message lock, so hence we set a flag and set the redraw later
    switch (cmd) {
        case SHOW_NEXT_CUE:
            updateActiveShowOptionsFromCCIIndex(activeShowOptions.currentCueIndex + 1);
            cueListBox.repaintRow(activeShowOptions.currentCueIndex);
            cueListBox.repaintRow(activeShowOptions.currentCueIndex - 1);
            break;
        case SHOW_PREVIOUS_CUE:
            updateActiveShowOptionsFromCCIIndex(activeShowOptions.currentCueIndex - 1);
            cueListBox.repaintRow(activeShowOptions.currentCueIndex);
            cueListBox.repaintRow(activeShowOptions.currentCueIndex + 1);
            break;
        case SHOW_PLAY: {
            // Dispatch the job.
            auto &cci = cciVector.getCurrentCueInfoByIndex(activeShowOptions.currentCueIndex);
            // If either of these circumstances, we shouldn't pass an invalid CCI onwards. The action is invalid.
            if (cci.isInvalid() || cci.actions.empty()) {
                sendCommandToAllListeners(SHOW_STOP);
                return;
            }

            cci.currentlyPlaying = true;
            activeShowOptions.currentCuePlaying = true;

            dispatcher.addCueToMessageQueue(cci);
            cciVector.setAsRunning(cci);
            currentCueListItemRequiresRedraw = true;
            break;
        }
        case SHOW_STOP: {
            auto &cci = cciVector.getCurrentCueInfoByIndex(activeShowOptions.currentCueIndex);
            if (cci.isInvalid()) { break; }

            cci.currentlyPlaying = false;
            activeShowOptions.currentCuePlaying = false;

            dispatcher.stopAllActionsInCCI(cci);
            cciVector.removeFromRunning(cci);
            currentCueListItemRequiresRedraw = true;
            break;
        }
        case SHOW_NAME_CHANGE:
        case FULL_SHOW_RESET:
            break; // Don't need to do anything, but we still need to broadcast it to all callbacks.
        case CUES_ADDED:
            activeShowOptions.numberOfCueItems = cciVector.getSize();
            break;
        case CUES_DELETED:
            // We also have to check that the current activeShowOptions hasn't just been deleted.
            if (cciVector.getSize() == 0) {
                // Nothing left... let's update the active show options
                updateActiveShowOptionsFromCCIIndex(0);
                break;
            }
            if (cciVector.cciInVector(activeShowOptions.currentCueInternalID)) {
                // The CCI has NOT been deleted from the vector.
                setNewIndexForCCI();
                break;
            }
            // Otherwise, let's try find a valid index.
            activeShowOptions.currentCueIndex--;
            updateActiveShowOptionsFromCCIIndex(activeShowOptions.currentCueIndex);
            break;
        case CUE_INDEXS_CHANGED:
            setNewIndexForCCI();
            break;
        case CUE_STOPPED:
            cmd = SHOW_STOP; // Morph command
            break;
        default:
            jassertfalse; // Invalid ShowCommand
    }
    sendCommandToAllListeners(cmd, currentCueListItemRequiresRedraw);
}


void MainComponent::sendCueCommandToAllListeners(const ShowCommand command, const std::string &cciInternalID,
                                                 const size_t cciCurrentIndex) const {
    for (auto *comp: callbackCompsUponActiveShowOptionsChanged) {
        if (comp != nullptr) {
            comp->cueCommandOccurred(command, cciInternalID, cciCurrentIndex);
            continue;
        }
        jassertfalse;
    }
}


void MainComponent::sendCommandToAllListeners(const ShowCommand cmd, bool currentCueListItemRequiresRedraw) {
    {
        const MessageManagerLock mmLock;
        if (!mmLock.lockWasGained()) {
            jassertfalse; // Woah woah woah... where's our message lock?
        }
        if (currentCueListItemRequiresRedraw) {
            cueListBox.repaintRow(activeShowOptions.currentCueIndex);
        }
        for (auto *comp: callbackCompsUponActiveShowOptionsChanged) {
            if (comp != nullptr) {
                comp->commandOccurred(cmd);
                continue;
            }
            jassertfalse; // Invalid pointer to callback
        }
    } // Force message manager lock to be released here, so that we can call this function from any thread.
}


void MainComponent::cueCommandOccurred(ShowCommand command, std::string cciInternalID, size_t cciCurrentIndex) {
    bool cueListItemRequiresRedraw = true;
    if (cciInternalID == activeShowOptions.currentCueInternalID) {
        cueListItemRequiresRedraw = false;
        // When a cue command is morphed and passed to commandOccurred, it should handle the redraw
        // Ok so, we also have to pass this command to commandOccurred(), but first we need to morph it.
        switch (command) {
            case CUE_STOPPED:
                commandOccurred(SHOW_STOP);
                break;
            case JUMP_TO_CUE:
                //...yeah, but we're already on this cue.
                return;
            default:
                break;
        }
    } else {
        switch (command) {
            case JUMP_TO_CUE: {
                // Reset activeShowOptions
                size_t previousIndex = activeShowOptions.currentCueIndex;
                updateActiveShowOptionsFromCCIIndex(cciCurrentIndex);
                cueListBox.repaintRow(previousIndex);
                cueListBox.repaintRow(cciCurrentIndex);
                break;
            }
            default:
                break;
        }
    }
    {
        const MessageManagerLock mmLock;
        if (!mmLock.lockWasGained()) {
            jassertfalse; // Woah woah woah... where's our message lock?
        }
        if (cueListItemRequiresRedraw) {
            cueListBox.repaintRow(cciCurrentIndex); // Any event that occurs must be redrawn
        }
        sendCueCommandToAllListeners(command, cciInternalID, cciCurrentIndex);
    } // Force message manager lock to be released here, so that we can call this function from any thread.
}


void MainComponent::actionFinished(std::string actionID) {
    auto remaining = cciVector.removeFromRunning(actionID);
    if (remaining == CurrentCueInfoVector::sizeTLimit) {
        // Oh no... we done goofed up.
        // Let's just... pretend nothing happened.
        // We're only returning without jassert because removeFromRunning should've already jasserted.
        return;
    }
    if (remaining == 0) {
        // Wait... that's the last action running for the CCI!
        // Let's update the active show options to reflect that the current cue is no longer playing.
        // First, find the CCI internal ID for the action ID
        const auto cciInternalID = cciVector.getParentCCIInternalID(actionID);
        if (cciInternalID.empty()) {
            jassertfalse; // Invalid CCI Internal ID. That means it wasn't in the actionIDtoCCIInternalIDMap.
            return;
        }
        // Now, find the index of the CCI internal ID in the cciIDtoIndexMap
        size_t cciIndex = cciVector.getIndexByCCIInternalID(cciInternalID);
        if (cciIndex == CurrentCueInfoVector::sizeTLimit) {
            jassertfalse; // Invalid CCI Index. That means it wasn't in the cciIDtoIndexMap.
            return;
        }

        // Now, set CCI as not playing
        auto &cci = cciVector.getCurrentCueInfoByIndex(cciIndex);
        cci.currentlyPlaying = false;
        if (cciIndex == activeShowOptions.currentCueIndex) {
            activeShowOptions.currentCuePlaying = false;
        }
        // Call the listener... the cueCommandOccurred listener will pass on a SHOW_STOP ShowCommand if the CCI is the current CCI.
        cueCommandOccurred(CUE_STOPPED, cci.getInternalID(), cciIndex);
    }
}


void MainComponent::closeRequested(WindowType windowType, std::string uuid) {
    switch (windowType) {
        case AppComponents_OSCActionConstructor:
            auto it = actionConstructorWindows.find(uuid);
            if (it != actionConstructorWindows.end()) {
                auto cueOSCAction = it->second->getCompiledCurrentCueAction();
                if (cueOSCAction.oat != EXIT_THREAD) {
                    it->second.reset(new OSCActionConstructor(uuidGen.generate(), "Constructor 2", cueOSCAction));
                } else {
                    it->second.reset();
                }
            }
    }
}

void MainComponent::terminateChildWindows() {
    for (auto& [id, ptr]: actionConstructorWindows) {
        ptr.reset();
    }
}


// ==========================================================================


void CueListData::paintContents(int rowNum, Graphics &g, Rectangle<int> bounds) {
    auto &cci = cciVector.getCurrentCueInfoByIndex(rowNum);
    if (cci.isInvalid()) {
        return;
    }
    auto boundsCopy = bounds;
    if (rowNum == activeShowOptions.currentCueIndex) {
        g.setColour(UICfg::SELECTED_CUE_LIST_ITEM_BG_COLOUR);
        g.fillRect(boundsCopy);
    }

    auto textHeight = boundsCopy.getHeight() * 0.6f;
    float bounds20ths = boundsCopy.getWidth() * 0.05f;
    float padding = UICfg::STD_PADDING * boundsCopy.getHeight() * 7;

    auto numBox = boundsCopy.removeFromLeft(bounds20ths);
    auto idBox = boundsCopy.removeFromLeft(bounds20ths * 3);
    auto nameBox = boundsCopy.removeFromLeft(bounds20ths * 10);
    auto numberOfActionsBox = boundsCopy.removeFromLeft(bounds20ths);
    auto stateBox = boundsCopy.removeFromLeft(bounds20ths * 3);
    auto editBox = boundsCopy;

    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    g.setFont(textHeight);
    g.setColour(UICfg::TEXT_COLOUR);

    g.drawText(String(rowNum + 1), numBox.reduced(padding), Justification::centred);
    g.drawText(cci.id, idBox.reduced(padding), Justification::centredLeft);
    g.drawText(cci.name, nameBox.reduced(padding), Justification::centredLeft);
    g.drawText(String(cci.actions.size()), numberOfActionsBox.reduced(padding), Justification::centred);
    g.setColour(cci.currentlyPlaying ? UICfg::POSITIVE_BUTTON_COLOUR : UICfg::NEGATIVE_BUTTON_COLOUR);
    g.fillRect(stateBox);
    g.setColour(UICfg::TEXT_COLOUR);
    g.drawText(cci.currentlyPlaying ? "PLAYING" : "STOPPED", stateBox.reduced(padding), Justification::centred);
    // g.drawText(, numberOfActionsBox, Justification::centred);

    // Draw boxes for all rectangles
    g.setColour(UICfg::CUE_LIST_ITEM_INSIDE_OUTLINES_COLOUR);
    g.drawRect(numBox);
    g.drawRect(idBox);
    g.drawRect(nameBox);
    g.drawRect(numberOfActionsBox);
    g.drawRect(stateBox);
    g.drawRect(editBox);
    g.setColour(UICfg::CUE_LIST_ITEM_OUTLINE_COLOUR);
    g.drawRect(bounds);

    // .getInternalID() )
}


// ==========================================================================


int CCIActionList::getTheoreticallyRequiredHeight(float usingFontSize) {
    if (usingFontSize <= 0) {
        usingFontSize = targetFontSize;
    }
    float eachLinePadding = usingFontSize * UICfg::STD_PADDING * 3;
    float eachActionPadding = usingFontSize * UICfg::STD_PADDING * 30;
    float height = 0;
    // We don't actually need to check if getCCI() is valid because an invalid one would just have an empty actions vector.
    for (auto &action: getCCI().actions) {
        // /path/to/osc/arg
        // COMMAND / FADE COMMAND (this line is 0.7 times fontsize)
        height += usingFontSize + usingFontSize * 0.7f + eachLinePadding * 2;
        switch (action.oat) {
            case OAT_COMMAND:
                // Verbose Name         Val (s/f/i)
                height += usingFontSize + eachActionPadding;
                break;
            case OAT_FADE:
                // Verbose Name         0.0 >> 1.0
                // Fade Time            0.0
                height += 2 * (usingFontSize + eachLinePadding);
                break;
            case EXIT_THREAD:
                jassertfalse; // Why is an EXIT_THREAD command being passed to CCIActionList...?
                break;
        }
        height += eachActionPadding;
    }
    return static_cast<int>(std::ceil(height));
}


void CCIActionList::resized() {
    auto boundsWidth = getWidth();
    if (boundsWidth == 0 || getHeight() == 0) {
        return;
    }
    auto widthTenths = boundsWidth * 0.1;
    /* Vertical scaling and horizonal scaling should be separate but:
    - Font size should be purely determined by component height
    - Widths should be purely determined by component width
    */
    valueAndTypeMaxWidth = widthTenths * 3;
    // The width minus the valueAndType width and minus the standard padding
    argumentVerboseNameMaxWidth = boundsWidth - valueAndTypeMaxWidth - boundsWidth * UICfg::STD_PADDING;

    // For value of ValueStorer
    oscArgumentValueFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, targetFontSize, Font::italic);
    // Find how many single characters can fit in the width for valueAndTypeMaxWidth
    oscArgumentValueMaxChars = std::floor(
        valueAndTypeMaxWidth / GlyphArrangement::getStringWidth(oscArgumentValueFont, "W"));

    // For COMMAND/FADE
    oatFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, targetFontSize * 0.7f, Font::plain);
    oatFontMaxChars = std::floor(boundsWidth / GlyphArrangement::getStringWidth(oatFont, "W"));

    // For verbose name of the argument
    verboseNameFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, targetFontSize, Font::plain);
    verboseNameMaxChars = std::floor(
        argumentVerboseNameMaxWidth / GlyphArrangement::getStringWidth(oscArgumentValueFont, "W"));

    // For Path of OSC Argument (e.g., /path/to/command)
    pathFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, targetFontSize, Font::bold);
    pathMaxChars = std::floor(boundsWidth / GlyphArrangement::getStringWidth(pathFont, "W"));
}


String CCIActionList::getWidthAdjustedArgumentValueString(const String &value, const String &typeAlias) const {
    auto finalTypeAlias = typeAlias;
    if (typeAlias.length() != 1) {
        jassertfalse; // It's a type alias, not a typename... only meant to be one character.
        finalTypeAlias = "?";
    }
    // First, test value & type as just exactly provides values
    String aliasedStr = value + " (" + finalTypeAlias + ")";
    int aliasedStrLen = aliasedStr.length();

    if (aliasedStrLen <= oscArgumentValueMaxChars) {
        // Str width is within bounds, let's return
        return aliasedStr;
    }

    // Otherwise kill some children.
    // Start by removing type hint - 4 chars (i.e., " (x)"
    if (aliasedStrLen - 4 <= oscArgumentValueMaxChars) {
        return value; // I.e., un-aliased value
    }

    // Ok, that clearly doesn't work. Let's concatenate the value.
    // Luckily monospace fonts have the same character width YAYAYYAYAYYAYYAYY

    if (oscArgumentValueMaxChars < 3) { return ""; } // We can't even fit "..." anyway sooo.... return nothing ig?
    if (oscArgumentValueMaxChars == 3) { return "..."; } // Well at least we can fit ellipses right?

    // Otherwise, simply return a concatenated version with ... at the end
    return value.substring(0, oscArgumentValueMaxChars - 3) + "..."; // At least 4 characters
    // The above substring should always be in range as:
    // - oscArgumentMaxChars must be > 3 to reach here
    // - value.length() > oscArgumentMaxChars
    // - hence, value.length() >= 4 to reach here.
}


String CCIActionList::getWidthAdjustedArgumentValueString(const ValueStorer &value, ParamType type) const {
    String valueAsString;
    String typeAlias;
    switch (type) {
        case STRING:
            valueAsString = value.stringValue;
            typeAlias = "s";
            break;
        case INT:
            valueAsString = String(value.intValue);
            typeAlias = "i";
            break;
        case _GENERIC_FLOAT: {
            auto rounded = roundTo(value.floatValue, UICfg::ROUND_TO_WHEN_IN_DOUBT);
            valueAsString = String(rounded, UICfg::ROUND_TO_WHEN_IN_DOUBT);
            typeAlias = "f";
            break;
        }
        case LINF:
        case LOGF:
        case ENUM:
        case LEVEL_1024:
        case LEVEL_161:
        case BITSET:
        case OPTION:
        case _BLANK:
            jassertfalse; // Why are you passing invalid types here?
            break;
    }
    return getWidthAdjustedArgumentValueString(valueAsString, typeAlias);
}


String CCIActionList::getWidthAdjustedVerboseName(const String &verboseName) const {
    int strLen = verboseName.length();
    if (strLen <= verboseNameMaxChars) {
        return verboseName;
    }
    // Ok, let's start killing children
    // We start by checking if we reasonably fit anything into the width.
    if (verboseNameMaxChars < 3) { return ""; }
    if (verboseNameMaxChars == 3) { return "..."; }
    // Ok let's now replace with _______...
    return verboseName.substring(0, verboseNameMaxChars - 3) + "...";
}


String CCIActionList::oatAppropriateForWidth(OSCActionType oat) const {
    switch (oat) {
        case OAT_COMMAND:
            // It's actually meant to say arguments... not commands. Fixed!
            if (oatFontMaxChars >= 8) { return "ARGUMENTS"; }
            if (oatFontMaxChars >= 4) { return "ARGS"; }
            if (oatFontMaxChars == 3) { return "ARG"; }
            return "";
        case OAT_FADE:
            if (oatFontMaxChars >= 12) { return "FADE ARGUMENTS"; }
            if (oatFontMaxChars >= 9) { return "FADE ARGS"; }
            if (oatFontMaxChars >= 4) { return "FADE"; }
            if (oatFontMaxChars == 3) { return "FDE"; }
            return "";
        case EXIT_THREAD:
            jassertfalse; // PLEASE.
            // FOR. THE. LAST. TIME.
            // STOP.
    }
    return "";
}


Rectangle<int> CCIActionList::drawArgumentNameAndValue(Graphics &g, int leftmostX, float currentHeight,
                                                       const String &formattedVerboseName,
                                                       const String &formattedArgumentValue,
                                                       int verboseNameFontHeightRounded,
                                                       int oatArgumentFontHeightRounded) {
    if (verboseNameFontHeightRounded == -1) {
        verboseNameFontHeightRounded = static_cast<int>(std::ceil(verboseNameFont.getHeight()));
    }
    if (oatArgumentFontHeightRounded == -1) {
        oatArgumentFontHeightRounded = static_cast<int>(std::ceil(oscArgumentValueFont.getHeight()));
    }
    int currentHeightCeil = static_cast<int>(std::ceil(currentHeight));

    Rectangle<int> verboseNameBox = {
        leftmostX,
        currentHeightCeil,
        static_cast<int>(std::floor(argumentVerboseNameMaxWidth)),
        verboseNameFontHeightRounded
    };

    g.setFont(oscArgumentValueFont);
    g.drawFittedText(formattedVerboseName, verboseNameBox, Justification::centredLeft, 1);

    Rectangle<int> argumentValueBox{
        static_cast<int>(std::ceil(getWidth() - valueAndTypeMaxWidth)),
        // We actually want the ceil as this will be closer to the right
        currentHeightCeil,
        static_cast<int>(std::ceil(valueAndTypeMaxWidth)), // Same here!
        oatArgumentFontHeightRounded
    };

    g.setFont(oscArgumentValueFont);
    g.drawFittedText(formattedArgumentValue, argumentValueBox, Justification::centredRight, 1);

    // Now return a new rectangle encompassing both texts we just drew.
    return {
        leftmostX,
        currentHeightCeil,
        getWidth(),
        (oatArgumentFontHeightRounded > verboseNameFontHeightRounded)
            ? oatArgumentFontHeightRounded
            : verboseNameFontHeightRounded // Whichever height is higher
    };
}


void CCIActionList::commandOccurred(ShowCommand command) {
    switch (command) {
        case SHOW_NEXT_CUE:
        case SHOW_PREVIOUS_CUE:
        case FULL_SHOW_RESET:
            repaint();
            lastRenderedCCIInternalID = getCCI().getInternalID();
            break;
        case CUES_ADDED:
        case CUES_DELETED: {
            // If the previous CCI was different, repaint().
            auto &cci = getCCI();
            if (cci.getInternalID() != lastRenderedCCIInternalID) {
                repaint();
                lastRenderedCCIInternalID = cci.getInternalID();
            }
            break;
        }
        case SHOW_PLAY:
        case SHOW_STOP:
        case SHOW_NAME_CHANGE:
        case CUE_STOPPED:
            break;
        default:
            jassertfalse; // ...I... but... like... ...like... all the... commands are already... covered...?????
            break;
    }
}


void CCIActionList::paint(Graphics &g) {
    auto cci = getCCI();
    if (cci.id.isEmpty()) {
        return; // Don't bother drawing: an empty ID implies a null CCI.
    }
    int localBoundsWidth = getWidth();
    int leftmostX = getX();
    float currentHeight = 0.f;

    const float EACH_LINE_PADDING = targetFontSize * UICfg::STD_PADDING * 3;
    const float EACH_ACTION_PADDING = targetFontSize * UICfg::STD_PADDING * 30;


    int pathFontHeight = static_cast<int>(std::ceil(pathFont.getHeight()));
    int oscArgumentFontHeight = static_cast<int>(std::ceil(oscArgumentValueFont.getHeight()));
    int oatFontHeight = static_cast<int>(std::ceil(oatFont.getHeight()));
    int verboseNameFontHeight = static_cast<int>(std::ceil(verboseNameFont.getHeight()));


    for (CueOSCAction &action: cci.actions) {
        // We have to draw the command for both COMMAND and FADE.

        // First draw the address path
        String addr = action.oscAddress.toString();
        int addrLen = addr.length();
        if (addrLen > pathMaxChars) {
            if (pathMaxChars < 3) { addr = ""; } else if (pathMaxChars == 3) { addr = "..."; } else {
                addr = addr.substring(0, pathMaxChars - 3) + "...";
            }
        }
        Rectangle<int> pathBox = {
            leftmostX,
            static_cast<int>(std::ceil(currentHeight)),
            localBoundsWidth,
            pathFontHeight
        };
        g.setColour(UICfg::TEXT_ACCENTED_COLOUR);
        g.setFont(pathFont);
        g.drawFittedText(addr, pathBox, Justification::topLeft, 1);
        currentHeight += pathBox.getHeight() + EACH_LINE_PADDING;

        // Now draw COMMAND/FADE
        Rectangle<int> commandTypeBox = {
            leftmostX,
            static_cast<int>(std::ceil(currentHeight)),
            localBoundsWidth,
            oatFontHeight
        };
        g.setColour(UICfg::TEXT_COLOUR);
        g.setFont(oatFont);
        g.drawFittedText(oatAppropriateForWidth(action.oat), commandTypeBox, Justification::topLeft, 1);
        currentHeight += commandTypeBox.getHeight() + EACH_LINE_PADDING;

        // Now, draw each/the osc argument(s) associated with each action
        switch (action.oat) {
            case OAT_COMMAND: {
                // For OAT_COMMAND, we previously could have multiple parameters. THIS IS NOT TRUE ANYMORE!
                // Make a copy of the current template. Sacrifice the memory, not the compiler.
                OSCMessageArguments currentTemplate = action.oatCommandOSCArgumentTemplate;

                // Figure out which template type this is to get the verbose name
                String verboseName;
                if (auto *optVal = std::get_if<OptionParam>(&currentTemplate)) {
                    verboseName = optVal->verboseName;
                } else if (auto *enumVal = std::get_if<EnumParam>(&currentTemplate)) {
                    verboseName = enumVal->verboseName;
                } else if (auto *nonIter = std::get_if<NonIter>(&currentTemplate)) {
                    verboseName = nonIter->verboseName;
                }

                auto currentArgument = action.argument;
                // Draw the verbose name and value
                auto drawnTextBox = drawArgumentNameAndValue(
                    g, leftmostX, currentHeight,
                    getWidthAdjustedVerboseName(verboseName),
                    getWidthAdjustedArgumentValueString(currentArgument, currentArgument._meta_PARAMTYPE),
                    verboseNameFontHeight, oscArgumentFontHeight);

                currentHeight += drawnTextBox.getHeight() + EACH_LINE_PADDING;
                break;
            }
            case OAT_FADE: {
                // Required as getWidthAdjustedVerboseName requires juce::String&, not std::string
                String verboseName = action.oscArgumentTemplate.verboseName;
                String idealArgumentValueStr;
                String typeAlias = "?";
                switch (action.oscArgumentTemplate._meta_PARAMTYPE) {
                    case INT: {
                        idealArgumentValueStr = String(action.startValue.intValue) + " >> " + String(
                                                    action.endValue.intValue);
                        typeAlias = "i";
                        break;
                    }
                    case LINF:
                    case LOGF:
                    case LEVEL_1024:
                    case LEVEL_161: {
                        idealArgumentValueStr = String(action.startValue.floatValue) + " >> " + String(
                                                    action.endValue.floatValue);
                        typeAlias = "f";
                        break;
                    }
                    default:
                        jassertfalse;
                        // WHAT IS YOUR PROBLEM WITH PASSING INCORRECT PARAMTYPES AHHAHAHAHHAHHHHHAAHAAAHHHHAHHHAAHHHAHAHHAHHHHHHHHHHHHHHHHHHHHHHHHHHAHHAHHAHAHHAHHAHHAHHAHHHAHAAHAHHAHAHHAAAAHHAHAHHAHHHHHH
                }
                auto drawnTextBox = drawArgumentNameAndValue(
                    g, leftmostX, currentHeight,
                    getWidthAdjustedVerboseName(verboseName),
                    getWidthAdjustedArgumentValueString(idealArgumentValueStr, typeAlias),
                    verboseNameFontHeight, oscArgumentFontHeight);

                currentHeight += drawnTextBox.getHeight() + EACH_LINE_PADDING;
                // We also need to draw another one to indicate the fade time.
                verboseName = "Fade Time (s)";
                typeAlias = "f";
                idealArgumentValueStr = String(roundTo(action.fadeTime, UICfg::ROUND_TO_WHEN_IN_DOUBT));
                drawnTextBox = drawArgumentNameAndValue(
                    g, leftmostX, currentHeight,
                    getWidthAdjustedVerboseName(verboseName),
                    getWidthAdjustedArgumentValueString(idealArgumentValueStr, typeAlias),
                    verboseNameFontHeight, oscArgumentFontHeight);
                currentHeight += drawnTextBox.getHeight() + EACH_LINE_PADDING;
                break;
            }
            case EXIT_THREAD:
                jassertfalse; // ...you realise no matter how many times you insist on passing an invalid ParamType,
                // I'm still not implementing a special case for it?
                break;
        }
        currentHeight += EACH_ACTION_PADDING;
    }
    lastRenderHeight = static_cast<int>(std::ceil(currentHeight));
}


// ==========================================================================


CCISidePanel::CCISidePanel(ActiveShowOptions &activeShowOptions,
                           CurrentCueInfoVector &cciVector): activeShowOptions(activeShowOptions), cciVector(cciVector),
                                                             actionList(activeShowOptions, cciVector, 1.f) {
    cueActionListViewport.setViewedComponent(&actionList, false);
    cueActionListViewport.setScrollBarsShown(true, false, true, false);
    // cueActionListViewport.setColour(ScrollBar::thumbColourId, UICfg::LIGHT_BG_COLOUR); // Can't seem to change colour?
    addAndMakeVisible(cueActionListViewport);
}


void CCISidePanel::constructImage() {
    if (localBoundsIsInvalid()) {
        return; // Don't construct an image if the local bounds are invalid.
    }
    panelImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(panelImage);

    g.fillAll(UICfg::BG_COLOUR);

    g.setColour(UICfg::TEXT_COLOUR);


    auto &currentCueInfo = cciVector.getCurrentCueInfoByIndex(activeShowOptions.currentCueIndex);
    if (currentCueInfo.isInvalid()) {
        return; // Don't draw nothing!
    }


    // Cue Name
    g.setFont(titleFont);
    g.setFont(cueNameBox.getHeight() * 0.7);
    // Figure out string width
    // If the name is too long, then scale down and use two lines
    if (GlyphArrangement::getStringWidthInt(g.getCurrentFont(), currentCueInfo.name) <= cueNameBox.getWidth()) {
        g.drawFittedText(currentCueInfo.name, cueNameBox.toNearestInt(), Justification::bottomLeft, 1);
    } else {
        g.setFont(cueNameBox.getHeight() * 0.4);
        g.drawFittedText(currentCueInfo.name, cueNameBox.toNearestInt(), Justification::bottomLeft, 2);
    }

    // "COMMANDS" (dear github copilot, please be smart. It is not that hard to generate a line of code to draw this text. How are you struggling at this?)
    g.setFont(commandsTitleBox.getHeight() * 0.8);
    g.drawFittedText("COMMANDS", commandsTitleBox.toNearestInt(), Justification::centredLeft, 1);


    // Cue Description
    g.setFont(textFont);
    g.setFont(cueDescriptionBox.getHeight() * 0.15);
    g.drawFittedText(currentCueInfo.description, cueDescriptionBox.toNearestInt(), Justification::topLeft, 6);


    auto stoppedPlayingIndicatorBoxWidth = stoppedPlayingIndicatorBox.getWidth();
    auto stoppedPlayingIndicatorBoxHeight = stoppedPlayingIndicatorBox.getHeight();

    // Draw for Playing
    playingIndicatorImage = Image(Image::ARGB, stoppedPlayingIndicatorBoxWidth,
                                  stoppedPlayingIndicatorBoxHeight, true);
    Graphics pII(playingIndicatorImage);

    pII.fillAll(UICfg::POSITIVE_BUTTON_COLOUR);
    pII.setFont(monospaceFont);
    pII.setFont(stoppedPlayingIndicatorBoxHeight * 0.6);
    pII.setColour(UICfg::TEXT_COLOUR);
    pII.drawFittedText("PLAYING",
                       0, 0, stoppedPlayingIndicatorBoxWidth, stoppedPlayingIndicatorBoxHeight,
                       Justification::centred, 1);


    // Draw for Stopped
    stoppedIndicatorImage = Image(Image::ARGB, stoppedPlayingIndicatorBoxWidth,
                                  stoppedPlayingIndicatorBoxHeight, true);
    Graphics sII(stoppedIndicatorImage);

    sII.fillAll(UICfg::NEGATIVE_BUTTON_COLOUR);
    sII.setFont(monospaceFont);
    sII.setFont(stoppedPlayingIndicatorBoxHeight * 0.6);
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

    // "COMMANDS"
    commandsTitleBox = bounds.removeFromTop(heightTenths * 0.3f);

    // Padding
    bounds.removeFromTop(padding);

    // Viewport
    cueActionListViewport.setScrollBarThickness(widthTenths * 0.15f);
    cueActionListViewport.setBounds(bounds);
    viewportBox = bounds;
    resizeActionList();

    constructImage();
}


void CCISidePanel::resizeActionList() {
    actionList.setTargetFontSize(cueNameBox.getHeight() * 0.25f, false);
    // Don't need to repaint; setting new bounds will repaint it.
    actionList.setBounds(viewportBox.getX(), viewportBox.getY(),
                         viewportBox.getWidth() - cueActionListViewport.getScrollBarThickness(),
                         actionList.getTheoreticallyRequiredHeight());
}

void CCISidePanel::selectedCueChanged() {
    resizeActionList();
    constructImage();
    repaint();
    auto &cci = cciVector.getCurrentCueInfoByIndex(activeShowOptions.currentCueIndex);
    if (cci.isInvalid()) {
        lastCCIInternalID = "";
        return;
    }
    // Assumes currentCueIndex is valid!
    lastCCIInternalID = cci.getInternalID();
}


void CCISidePanel::paint(Graphics &g) {
    g.drawImage(panelImage, getLocalBounds().toFloat());
    if (activeShowOptions.currentCuePlaying) {
        g.drawImage(playingIndicatorImage, stoppedPlayingIndicatorBox.toFloat());
    } else {
        g.drawImage(stoppedIndicatorImage, stoppedPlayingIndicatorBox.toFloat());
    }
}


void CCISidePanel::commandOccurred(const ShowCommand command) {
    switch (command) {
        case SHOW_PLAY:
        case SHOW_STOP:
            repaint();
            break;
        case SHOW_NEXT_CUE:
        case SHOW_PREVIOUS_CUE: {
            selectedCueChanged();
            break;
        }
        case CUES_DELETED: {
            // If the cue deleted is this one... then we need to update the side panel.
            auto &cci = cciVector.getCurrentCueInfoByIndex(activeShowOptions.currentCueIndex);
            if (cci.isInvalid() || // If no cues... or
                cci.getInternalID() != lastCCIInternalID) {
                // If the current cue is not the last one we had
                commandOccurred(SHOW_NEXT_CUE); // Triggers a repaint, resize and reimage
            }
            break;
        }
        case FULL_SHOW_RESET:
            commandOccurred(SHOW_NEXT_CUE); // This practically resets the side panel.
            break;
        default:
            break;
    }
}

void CCISidePanel::cueCommandOccurred(ShowCommand command, std::string cciInternalID, size_t cciCurrentIndex) {
    switch (command) {
        case JUMP_TO_CUE:
            selectedCueChanged();
            break;
        default:
            break;
    }
}


// ==========================================================================


void HeaderBar::reconstructButtonBackgroundImage() {
    if (localBoundsIsInvalid())
        return;

    buttonsBGImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);

    Graphics gBG(buttonsBGImage);

    bool stopEnabled = stopButton.isEnabled();
    bool downEnabled = downButton.isEnabled();
    bool upEnabled = upButton.isEnabled();
    bool playEnabled = playButton.isEnabled();


    gBG.setColour(stopEnabled ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(stopBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(stopBox, 1);

    gBG.setColour(downEnabled ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(downBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(downBox, 1);

    gBG.setColour(upEnabled ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(upBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(upBox, 1);

    gBG.setColour(playEnabled ? UICfg::HEADER_BG_COLOUR : UICfg::HEADER_BTN_DISABLED_BG_COLOUR);
    gBG.fillRect(playBox);
    gBG.setColour(UICfg::STRONG_BORDER_COLOUR);
    gBG.drawRect(playBox, 1);

    // For STOP and PLAY buttons, we need text labels
    gBG.setFont(UICfg::DEFAULT_MONOSPACE_FONT.withHeight(stopButtonTextBox.getHeight()));
    gBG.setColour(stopEnabled ? UICfg::TEXT_COLOUR: UICfg::TEXT_COLOUR_DARK);
    gBG.drawFittedText("STOP", stopButtonTextBox.toNearestInt(), Justification::centredLeft, 1);
    gBG.setColour(playEnabled ? UICfg::TEXT_COLOUR: UICfg::TEXT_COLOUR_DARK);
    gBG.drawFittedText("PLAY", playButtonTextBox.toNearestInt(), Justification::centredLeft, 1);

    // Also need enabled and disabled STOP, PLAY, UP and DOWN icons
    gBG.drawImage(stopEnabled ? stopIcon: stopIconDisabled, stopButtonIconBox, RectanglePlacement::centred);
    gBG.drawImage(playEnabled ? playIcon: playIconDisabled, playButtonIconBox, RectanglePlacement::centred);
    gBG.drawImage(upEnabled ? upIcon: upIconDisabled, upButtonIconBox, RectanglePlacement::centred);
    gBG.drawImage(downEnabled ? downIcon: downIconDisabled, downButtonIconBox, RectanglePlacement::centred);


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
    g.drawFittedText("ID: " + String(activeShowOptions.currentCueID), cueIDTextBox.toNearestInt(),
                     Justification::centredLeft, 1);
    g.drawFittedText(
        "Cue " + String(activeShowOptions.currentCueIndex + 1) + "/" + String(activeShowOptions.numberOfCueItems),
        cueNoTextBox.toNearestInt(), Justification::centredLeft, 1);

    /*
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
    */
}


void HeaderBar::resized() {
    if (localBoundsIsInvalid())
        return;

    auto bounds = getLocalBounds();

    // The bounds passed should be the bounds of ONLY the intended header bar.
    auto boundWidthTenths = bounds.getWidth() / 10;

    showNameBox = bounds.removeFromLeft(boundWidthTenths * 2);
    cueIDBox = bounds.removeFromLeft(boundWidthTenths * 1.5f);
    cueNoBox = bounds.removeFromLeft(boundWidthTenths * 1.5f);
    stopBox = bounds.removeFromLeft(boundWidthTenths);
    downBox = bounds.removeFromLeft(boundWidthTenths * 0.5f);
    upBox = bounds.removeFromLeft(boundWidthTenths * 0.5f);
    playBox = bounds.removeFromLeft(boundWidthTenths);
    timeBox = bounds; // Simply use remaining bounds.
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
    g.drawImage(buttonsBGImage, localBoundsToFloat);
    // g.drawImage(buttonsFGImage, localBoundsToFloat);
    g.drawImage(borderImage, localBoundsToFloat);


    // We'll need to manually draw the time.
    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    g.setFont(localBoundsToFloat.getHeight() * 0.8f); // Larger font size for the time text
    g.setColour(UICfg::TEXT_COLOUR);
    g.drawFittedText(getCurrentTimeAsFormattedString(), timeTextBox.toNearestInt(),
                     Justification::centred, 1);

}


void HeaderBar::selectedCueChanged() {
    // This case is not NEXT/PREVIOUS dependent. This means jumping to a cue not next or previous to the
    // currently selected one can still use this function.
    if (activeShowOptions.numberOfCueItems == 1) {
        // One cue only
        upButton.setEnabled(false);
        downButton.setEnabled(false);
    } else if (activeShowOptions.currentCueIndex == 0) {
        // First Cue Selected
        upButton.setEnabled(false);
        downButton.setEnabled(true);
    } else if ((activeShowOptions.currentCueIndex + 1) == activeShowOptions.numberOfCueItems) {
        // Last Cue Selected
        upButton.setEnabled(true);
        downButton.setEnabled(false);
    } else {
        upButton.setEnabled(true);
        downButton.setEnabled(true);
    }
    // Now we're on another cue, we also need to set the play button state.
    playButton.setEnabled(!activeShowOptions.currentCuePlaying);
    stopButton.setEnabled(activeShowOptions.currentCuePlaying);
}

void HeaderBar::commandOccurred(const ShowCommand command) {
    if (activeShowOptions.numberOfCueItems == 0) {
        upButton.setEnabled(false);
        downButton.setEnabled(false);
        playButton.setEnabled(false);
        downButton.setEnabled(false);
    } else {
        switch (command) {
            case SHOW_PLAY:
                playButton.setEnabled(false);
                stopButton.setEnabled(true);
                break;
            case SHOW_STOP:
                playButton.setEnabled(true);
                stopButton.setEnabled(false);
                break;
            case SHOW_NEXT_CUE:
            case SHOW_PREVIOUS_CUE:
                selectedCueChanged();
                break;
            case SHOW_NAME_CHANGE:
                break;
            case FULL_SHOW_RESET:
            case CUES_ADDED:
            case CUES_DELETED:
            case CUE_INDEXS_CHANGED:
                // The requirement in this case is to reset the buttons enabled state. If we trigger a
                // next/previous cue command manually, it can reset all the buttons relevant to this
                // header bar.
                commandOccurred(SHOW_NEXT_CUE);
                break;
            case CUE_STOPPED:
                break;
            default:
                jassertfalse; //... what show command are you sending?
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

void HeaderBar::cueCommandOccurred(ShowCommand command, std::string cciInternalID, size_t cciCurrentIndex) {
    switch (command) {
        case JUMP_TO_CUE:
            selectedCueChanged();
            break;
        default:
            break;
    }
}
