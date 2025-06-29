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


int CCIActionList::getTheoreticallyRequiredHeight(float usingFontSize) {
    if (usingFontSize <= 0) {
        usingFontSize = targetFontSize;
    }
    float eachLinePadding = usingFontSize * UICfg::STD_PADDING;
    float eachActionPadding = usingFontSize * UICfg::STD_PADDING * 2;
    float height = 0;
    // We don't actually need to check if getCCI() is valid because an invalid one would just have an empty actions vector.
    for (auto &action: getCCI().actions) {
        // /path/to/osc/arg
        // COMMAND / FADE COMMAND (this line is 0.7 times fontsize)
        height += usingFontSize + usingFontSize * 0.7f + eachLinePadding * 2;
        switch (action.oat) {
            case OAT_COMMAND:
                // Verbose Name         Val (s/f/i)
                // ...
                height += (usingFontSize + eachActionPadding) * action.oatCommandOSCArgumentTemplate.size();
                break;
            case OAT_FADE:
                // Verbose Name         0.0 >> 1.0
                // Fade Time            0.0
                height += 2 * (usingFontSize + eachLinePadding);
                break;
            case _EXIT_THREAD:
                jassertfalse; // Why is an _EXIT_THREAD command being passed to CCIActionList...?
                break;
        }
        height -= eachLinePadding - eachActionPadding; // i.e., height = height - eachLinePadding + eachActionPadding
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
    oscArgumentValueMaxChars = std::floor(valueAndTypeMaxWidth / GlyphArrangement::getStringWidth(oscArgumentValueFont, "W"));

    // For COMMAND/FADE
    oatFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, targetFontSize * 0.7f, Font::plain);
    oatFontMaxChars = std::floor(boundsWidth / GlyphArrangement::getStringWidth(oatFont, "W"));

    // For verbose name of the argument
    verboseNameFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, targetFontSize, Font::plain);
    verboseNameMaxChars = std::floor(argumentVerboseNameMaxWidth / GlyphArrangement::getStringWidth(oscArgumentValueFont, "W"));

    // For Path of OSC Argument (e.g., /path/to/command)
    pathFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, targetFontSize, Font::bold);
    pathMaxChars = std::floor(boundsWidth / GlyphArrangement::getStringWidth(pathFont, "W"));
}


String CCIActionList::getWidthAdjustedArgumentValueString(const String& value, const String& typeAlias) {
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

    if (oscArgumentValueMaxChars < 3) {return "";}           // We can't even fit "..." anyway sooo.... return nothing ig?
    if (oscArgumentValueMaxChars == 3) {return "...";}       // Well at least we can fit ellipses right?

    // Otherwise, simply return a concatenated version with ... at the end
    return value.substring(0, oscArgumentValueMaxChars - 3) + "..."; // At least 4 characters
    // The above substring should always be in range as:
    // - oscArgumentMaxChars must be > 3 to reach here
    // - value.length() > oscArgumentMaxChars
    // - hence, value.length() >= 4 to reach here.
}


String CCIActionList::getWidthAdjustedArgumentValueString(const ValueStorer &value, ParamType type) {
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
        case LINF: case LOGF: case ENUM: case LEVEL_1024: case LEVEL_161: case BITSET: case OPTION: case _BLANK:
            jassertfalse; // Why are you passing invalid types here?
            break;
    }
    return getWidthAdjustedArgumentValueString(valueAsString, typeAlias);
}


String CCIActionList::getWidthAdjustedVerboseName(const String &verboseName) {
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


String CCIActionList::oatAppropriateForWidth(OSCActionType oat) {
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
        case _EXIT_THREAD:
            jassertfalse; // PLEASE.
            // FOR. THE. LAST. TIME.
            // STOP.
    }
    return "";
}


Rectangle<int> CCIActionList::drawArgumentNameAndValue(Graphics &g, int leftmostX, float currentHeight,
    const String &formattedVerboseName, const String &formattedArgumentValue,
    int verboseNameFontHeightRounded, int oatArgumentFontHeightRounded) {

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

    Rectangle<int> argumentValueBox {
        static_cast<int>(std::ceil(getWidth() - valueAndTypeMaxWidth)), // We actually want the ceil as this will be closer to the right
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
        (oatArgumentFontHeightRounded > verboseNameFontHeightRounded) ?
            oatArgumentFontHeightRounded: verboseNameFontHeightRounded // Whichever height is higher
    };
}


void CCIActionList::commandOccurred(ShowCommand command) {
    switch (command) {
        case SHOW_NEXT_CUE: case SHOW_PREVIOUS_CUE: case FULL_SHOW_RESET:
            repaint();
            break;
        case SHOW_PLAY: case SHOW_STOP: case SHOW_NAME_CHANGE: case CURRENT_CUE_ID_CHANGE:
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
    const float EACH_ACTION_PADDING = targetFontSize * UICfg::STD_PADDING * 20;


    int pathFontHeight = static_cast<int>(std::ceil(pathFont.getHeight()));
    int oscArgumentFontHeight = static_cast<int>(std::ceil(oscArgumentValueFont.getHeight()));
    int oatFontHeight = static_cast<int>(std::ceil(oatFont.getHeight()));
    int verboseNameFontHeight = static_cast<int>(std::ceil(verboseNameFont.getHeight()));

    g.setColour(UICfg::TEXT_COLOUR);

    for (CueOSCAction &action: cci.actions) {
        // We have to draw the command for both COMMAND and FADE.

        // First draw the address path
        String addr = action.oscAddress.toString();
        int addrLen = addr.length();
        if (addrLen > pathMaxChars) {
            if (pathMaxChars < 3) { addr = ""; }
            else if (pathMaxChars == 3) { addr = "..."; }
            else { addr = addr.substring(0, pathMaxChars - 3) + "..."; }
        }
        Rectangle<int> pathBox = {
            leftmostX,
            static_cast<int>(std::ceil(currentHeight)),
            localBoundsWidth,
            pathFontHeight
        };
        g.setFont(pathFont);
        g.drawFittedText(addr, pathBox, Justification::topLeft, 1);
        currentHeight += pathBox.getHeight();
        currentHeight += EACH_LINE_PADDING;

        // Now draw COMMAND/FADE
        Rectangle<int> commandTypeBox = {
            leftmostX,
            static_cast<int>(std::ceil(currentHeight)),
            localBoundsWidth,
            oatFontHeight
        };
        g.setFont(oatFont);
        g.drawFittedText(oatAppropriateForWidth(action.oat), commandTypeBox, Justification::topLeft, 1);
        currentHeight += EACH_LINE_PADDING + commandTypeBox.getHeight();

        // Now, draw each/the osc argument(s) associated with each action
        switch (action.oat) {
            case OAT_COMMAND: {
                // For OAT_COMMAND, we could have numerous parameters.
                // First check the len of templates and valuestorers is the same.
                if (action.oatCommandOSCArgumentTemplate.size() != action.arguments.size()) {
                    jassertfalse; // OSC Action Argument Templates and Argument ValueStorer Vector must be the same size
                    break;
                }
                // This loop draws the argument verbose name and value for each argument in the action
                for (int i = 0; i < action.oatCommandOSCArgumentTemplate.size(); i++) {
                    OSCMessageArguments &currentTemplate = action.oatCommandOSCArgumentTemplate[i];
                    // Figure out which template type this is to get the verbose name
                    String verboseName;
                    if (auto *optVal = std::get_if<OptionParam>(&currentTemplate)) {
                        verboseName = optVal->verboseName;
                    } else if (auto *enumVal = std::get_if<EnumParam>(&currentTemplate)) {
                        verboseName = enumVal->verboseName;
                    } else if (auto *nonIter = std::get_if<NonIter>(&currentTemplate)) {
                        verboseName = nonIter->verboseName;
                    }
                    auto currentArgument = action.arguments[i];

                    // Draw the verbose name and value
                    auto drawnTextBox = drawArgumentNameAndValue(
                        g, leftmostX, currentHeight,
                        getWidthAdjustedVerboseName(verboseName),
                        getWidthAdjustedArgumentValueString(currentArgument, currentArgument._meta_PARAMTYPE),
                        verboseNameFontHeight, oscArgumentFontHeight
                        );

                    currentHeight += drawnTextBox.getHeight() + EACH_LINE_PADDING;
                }
                break;
            }
            case OAT_FADE: {
                // Required as getWidthAdjustedVerboseName requires juce::String&, not std::string
                String verboseName = action.oscArgumentTemplate.verboseName;
                String idealArgumentValueStr;
                String typeAlias = "?";
                switch (action.oscArgumentTemplate._meta_PARAMTYPE) {
                    case INT: {
                        idealArgumentValueStr = String(action.startValue.intValue) + " >> " + String(action.endValue.intValue);
                        typeAlias = "i";
                        break;
                    }
                    case LINF: case LOGF: case LEVEL_1024: case LEVEL_161: {
                        idealArgumentValueStr = String(action.startValue.floatValue) + " >> " + String(action.endValue.floatValue);
                        typeAlias = "f";
                        break;
                    }
                    default:
                        jassertfalse; // WHAT IS YOUR PROBLEM WITH PASSING INCORRECT PARAMTYPES AHHAHAHAHHAHHHHHAAHAAAHHHHAHHHAAHHHAHAHHAHHHHHHHHHHHHHHHHHHHHHHHHHHAHHAHHAHAHHAHHAHHAHHAHHHAHAAHAHHAHAHHAAAAHHAHAHHAHHHHHH
                }
                auto drawnTextBox = drawArgumentNameAndValue(
                    g, leftmostX, currentHeight,
                    getWidthAdjustedVerboseName(verboseName),
                    getWidthAdjustedArgumentValueString(idealArgumentValueStr, typeAlias),
                    verboseNameFontHeight, oscArgumentFontHeight);

                currentHeight += drawnTextBox.getHeight() + EACH_LINE_PADDING;
                // We also need to draw another one to indicate the fade time.
                verboseName = "Fade Time";
                typeAlias = "f";
                idealArgumentValueStr = String(roundTo(action.fadeTime, UICfg::ROUND_TO_WHEN_IN_DOUBT));
                drawnTextBox = drawArgumentNameAndValue(
                    g, leftmostX, currentHeight,
                    getWidthAdjustedVerboseName(verboseName),
                    getWidthAdjustedArgumentValueString(idealArgumentValueStr, typeAlias),
                    verboseNameFontHeight, oscArgumentFontHeight);
                break;
            }
            case _EXIT_THREAD:
                jassertfalse; // ...you realise no matter how many times you insist on passing an invalid ParamType,
                // I'm still not implementing a special case for it?
                break;
        }
        currentHeight += EACH_ACTION_PADDING;
    }
}


// ==========================================================================


CCISidePanel::CCISidePanel(ActiveShowOptions &activeShowOptions, std::vector<CurrentCueInfo>& currentCueInfos):
    activeShowOptions(activeShowOptions), currentCueInfos(currentCueInfos),
    actionList(activeShowOptions, currentCueInfos, 1.f) {
    addAndMakeVisible(actionList);
}


void CCISidePanel::constructImage() {
    panelImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(panelImage);

    g.fillAll(UICfg::BG_COLOUR);

    g.setColour(UICfg::TEXT_COLOUR);

    if (activeShowOptions.numberOfCueItems == 0) {
        return; // Don't draw nothing!
     }

    // Assumes currentCueIndex is valid!
    auto currentCueInfo = currentCueInfos[activeShowOptions.currentCueIndex];

    // Cue Name
    g.setFont(titleFont);
    g.setFont(cueNameBox.getHeight()*0.7);
    // Figure out string width
    // If the name is too long, then scale down and use two lines
    if (GlyphArrangement::getStringWidthInt(g.getCurrentFont(), currentCueInfo.name) <= cueNameBox.getWidth()) {
        g.drawFittedText(currentCueInfo.name, cueNameBox.toNearestInt(), Justification::bottomLeft, 1);
    } else {
        g.setFont(cueNameBox.getHeight()*0.4);
        g.drawFittedText(currentCueInfo.name, cueNameBox.toNearestInt(), Justification::bottomLeft, 2);
    }

    // "COMMANDS" (dear github copilot, please be smart. It is not that hard to generate a line of code to draw this text. How are you struggling at this?)
    g.setFont(commandsTitleBox.getHeight() * 0.8);
    g.drawFittedText("COMMANDS", commandsTitleBox.toNearestInt(), Justification::centredLeft, 1);


    // Cue Description
    g.setFont(textFont);
    g.setFont(cueDescriptionBox.getHeight()*0.15);
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

    // Action List - we'll give it rest of the bounds
    actionList.setBounds(bounds);
    // We also have to figure out its font size
    actionList.setTargetFontSize(cueNameBox.getHeight() * 0.2f);

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
