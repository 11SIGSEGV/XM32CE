/*
  ==============================================================================

    AppComponents.cpp
    Created: 28 May 2025 3:53:04pm
    Author:  anony

  ==============================================================================
*/

#include "AppComponents.h"


OSCActionConstructor::MainComp::MainComp() {
    setSize (1000, 800);
    // Template Categories never change, so let's set it now
    int i = 0;
    for (auto &[tpltCategory, tpltGroup]: TEMPLATE_CATEGORY_MAP) {
        i++;
        tpltCategoryDd.addItem(tpltGroup.name, i);
        dDitemIDtoCategory[i] = tpltCategory;
    }

    tpltCategoryDd.setColour(ComboBox::ColourIds::backgroundColourId, UICfg::TRANSPARENT);
    tpltCategoryDd.setColour(ComboBox::ColourIds::textColourId, UICfg::TEXT_ACCENTED_COLOUR);
    addAndMakeVisible(tpltCategoryDd);
    tpltCategoryDd.addListener(this);

    tpltDd.setColour(ComboBox::ColourIds::backgroundColourId, UICfg::TRANSPARENT);
    tpltDd.setColour(ComboBox::ColourIds::textColourId, UICfg::TEXT_ACCENTED_COLOUR);
    addAndMakeVisible(tpltDd);
    tpltDd.addListener(this);

    addAndMakeVisible(enableFadeCommandBtn);
    enableFadeCommandBtn.addListener(this);

    // Temporary code TODO: Remove after testing
    faderArgInput.reset(new Fader());
    faderArgInput->setBounds(10, 240, 150, 400);
    addAndMakeVisible(*faderArgInput);
}


void OSCActionConstructor::MainComp::resized() {
    auto bounds = getLocalBounds();
    auto heightTenths = bounds.getHeight() * 0.1;
    fontSize = heightTenths * 0.3;
    auto widthTenths = bounds.getWidth() * 0.1;

    // First, set the 'area' boxes.
    tpltSelectionBox = bounds.removeFromTop(heightTenths);
    fadeCmdBox = tpltSelectionBox.removeFromRight(widthTenths * 2);
    tpltSelectionBox.removeFromRight(widthTenths * 0.2); // Padding
    pathBox = bounds.removeFromTop(heightTenths * 1.5);
    buttonsBox = bounds.removeFromBottom(heightTenths);
    argBox = bounds;

    auto padding = bounds.getWidth() * UICfg::STD_PADDING;


    // Let's split up the template selection area.
    auto selBoxCp = tpltSelectionBox;
    selBoxCp.removeFromLeft(padding); // Left Padding for text

    auto selBoxTop = selBoxCp.removeFromTop(selBoxCp.getHeight() * 0.5);
    auto selBoxBottom = selBoxCp;

    tpltCategoryTitleBox = selBoxTop.removeFromLeft(selBoxTop.getWidth() * 0.4);
    selBoxTop.removeFromLeft(selBoxTop.getWidth() * 0.1); // Padding
    tpltCategoryDropBox = selBoxTop;
    tpltCategoryDd.setBounds(tpltCategoryDropBox);

    tpltSelectionTitleBox = selBoxBottom.removeFromLeft(selBoxBottom.getWidth() * 0.4);
    selBoxBottom.removeFromLeft(selBoxBottom.getWidth() * 0.1); // Padding
    tpltSelectionDropBox = selBoxBottom;
    tpltDd.setBounds(tpltSelectionDropBox);


    // Let's split up the fade command area.
    auto fdBoxCp = fadeCmdBox;
    fdBoxCp = fdBoxCp.reduced(padding / 2); // Padding
    fadeCmdTextBox = fdBoxCp.removeFromTop(fdBoxCp.getHeight() * 0.5);
    auto _minLen = std::min(fdBoxCp.getWidth(), fdBoxCp.getHeight());
    fadeCmdButtonBox = Rectangle(fdBoxCp.getX(), fdBoxCp.getY(), _minLen, _minLen); // Ensure square.
    enableFadeCommandBtn.setBounds(fadeCmdButtonBox);

    // Ok, time for the path box.
    auto pthBoxCp = pathBox;
    pthBoxCp = pthBoxCp.reduced(padding); // Padding
    pathTitleBox = pthBoxCp.removeFromTop(pthBoxCp.getHeight() * 0.3);
    pathInputBox = pthBoxCp;

    // And now for the argument box.
    auto argsBoxCp = argBox;
    argsBoxCp.removeFromLeft(padding); // Padding
    argsBoxCp.removeFromRight(padding); // More padding
    argTitleBox = argsBoxCp.removeFromTop(argsBoxCp.getHeight() * 0.08);
    argInputArea = argsBoxCp;


    reconstructImage();
}


void OSCActionConstructor::MainComp::paint(Graphics &g) {
    g.drawImage(backgroundImage, getLocalBounds().toFloat());

}


void OSCActionConstructor::MainComp::reconstructImage() {
    backgroundImage = Image(Image::ARGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(backgroundImage);

    g.setColour(Colours::red);
    g.drawRect(tpltSelectionBox);
    g.drawRect(pathBox);
    g.drawRect(buttonsBox);

    g.setFont(font);
    g.setFont(fontSize);
    g.setColour(UICfg::TEXT_COLOUR);

    g.drawText("Select a template category", tpltCategoryTitleBox, Justification::centredLeft);
    g.drawText("Select a template", tpltSelectionTitleBox, Justification::centredLeft);

    g.setFont(fontSize * 0.7);
    // Must use custom bool in case no template selected yet
    bool fadeEnabled;
    if (currentTemplateCopy == nullptr) {
        fadeEnabled = false;
    } else {
        fadeEnabled = currentTemplateCopy->FADE_ENABLED;
    }
    if (fadeEnabled) {
        g.setColour(UICfg::BG_SECONDARY_COLOUR);
        g.fillRect(fadeCmdBox);
        g.setColour(UICfg::TEXT_COLOUR);
    }
    g.drawFittedText(fadeEnabled ? "Enable fade command?": "Fading Not Available",
        fadeCmdTextBox, Justification::centredLeft, 2);

    g.setFont(fontSize);
    g.drawText("Path", pathTitleBox, Justification::centredLeft);


    // We need to reset the inputs upon a reconstruction.
    // HOWEVER, everything else stays.
    pathLabelInputs.clear();

    // No template is selected the first time the component is started, so check for nullptr.
    if (currentTemplateCopy == nullptr) {
        return;
    }

    // Handle in-path arguments
    Font fCopy = monospace;
    fCopy.setHeight(fontSize);
    fCopy.setStyleFlags(Font::plain);

    g.setFont(fCopy);
    Rectangle<int> pathBoxRemaining = pathInputBox;

    // Number of items in pathLabelInputValues (i.e., already generated values and not
    // cleared). If this is above 0, it is basically guaranteed the reconstructImage was called as a result of a resize
    // so the previous text should be restored.
    size_t existingLabelValuesSize = pathLabelInputValues.size();
    int numArgs {0};

    for (const auto& argTemplate: currentTemplateCopy->PATH) {
        if (pathBoxRemaining.getWidth() == 0) {
            jassertfalse; // We've literally run out of screen area... HOW LONG IS THE PATH?
            // OR WORSE, WHAT'S THE RESOLUTION OF YOUR WINDOW?
        }
        if (auto *string = std::get_if<std::string>(&argTemplate)) {
            String str {string->c_str()};
            g.drawText(str, pathBoxRemaining, Justification::centredLeft);
            pathBoxRemaining.removeFromLeft(GlyphArrangement::getStringWidthInt(fCopy, str));
        }
        else if (auto *nonIter = std::get_if<NonIter>(&argTemplate)) {
            bool autoAddToOtherVectors = true;
            numArgs++;
            String lblTxt;
            if (numArgs <= existingLabelValuesSize) {
                // Ok, we have an existing value to pull.
                lblTxt = pathLabelInputValues.at(numArgs - 1);
                autoAddToOtherVectors = false; // We already have a corresponding value for this label,
                // and hence we'd have a corresponding template and formatted value, and hence we don't need a new one.
                // Adding a new one would result in the 4 vectors being different sizes.
            }
            // See if previously registered text is available
            addInPathArgLabel(*nonIter, pathBoxRemaining, fCopy, lblTxt, autoAddToOtherVectors);
        }
    }

    // Handle Arguments
    g.setColour(Colours::red);
    g.drawRect(argTitleBox);
    g.setColour(UICfg::TEXT_COLOUR);
    g.setFont(font);
    g.setFont(fontSize);
    g.drawText("Argument", argTitleBox, Justification::centredLeft);
}


bool OSCActionConstructor::MainComp::validateTextInput(float input, const NonIter &argTemplate) {
    if (input < argTemplate.floatMin || input > argTemplate.floatMax) {
        return false;
    }
    return true;
}

bool OSCActionConstructor::MainComp::validateTextInput(int input, const NonIter &argTemplate) {
    if (input < argTemplate.intMin || input > argTemplate.intMax) {
        return false;
    }
    return true;
}

bool OSCActionConstructor::MainComp::validateTextInput(const String &input, const NonIter &argTemplate) {
    int len = input.length();
    if (len < argTemplate.intMin || len > argTemplate.intMax ) {
        return false;
    }
    return true;
}


Rectangle<int> OSCActionConstructor::MainComp::findProperLabelBounds(const NonIter &argTemplate,
    Rectangle<int> &remainingBox, const Font &fontInUse) {

    int boxWidth = 0;
    float singleCharWidth = GlyphArrangement::getStringWidth(fontInUse, "W");
    switch (argTemplate._meta_PARAMTYPE) {
        case STRING:
            // Remember, intMax also represents the maximum string length.
            boxWidth = static_cast<int>(std::ceil(singleCharWidth * (argTemplate.intMax + 1)));
            break;
        case INT:
            // We need to find the maximum length because of the negative sign. For example, a min val of -80 and max
            // val of 20 would mean the maximum length accounted for is 2 characters, but that's not right. It should be
            // 3 chars (because "-80" is 3 chars).
            // The extra 1 character is because the text box has margin.
            boxWidth = static_cast<int>(std::ceil(
                    (std::max(
                        std::to_string(argTemplate.intMax).length(),
                        std::to_string(argTemplate.intMin).length()
                        ) + 1) * singleCharWidth)
                    );
            break;
        default:
            jassertfalse; // Unallowed ParamType for an in-path label (text input field)
            return Rectangle<int>();
    }
    if (remainingBox.getWidth() < boxWidth) {
        jassertfalse; // We've run out of space! Help!
    }
    return remainingBox.removeFromLeft(boxWidth);
}


void OSCActionConstructor::MainComp::addInPathArgLabel(const NonIter& argTemplate, Rectangle<int>& remainingBox,
                                                       const Font& fontInUse, const String &text,
                                                       bool automaticallyAddToOtherVectors) {
    auto lblBounds = findProperLabelBounds(argTemplate, remainingBox, fontInUse);
    auto lbl = std::make_unique<Label>(argTemplate.name);

    lbl->setColour(Label::ColourIds::textColourId, UICfg::TEXT_ACCENTED_COLOUR);
    lbl->setColour(Label::ColourIds::backgroundWhenEditingColourId, UICfg::BG_SECONDARY_COLOUR);
    lbl->setColour(Label::ColourIds::outlineColourId, UICfg::TEXT_ACCENTED_COLOUR);
    lbl->setFont(fontInUse);
    lbl->setEditable(true);
    lbl->setHelpText(argTemplate.verboseName);
    lbl->setBounds(lblBounds);
    lbl->addListener(this);

    if (text.isEmpty()) {
        addAndMakeVisible(*lbl);
    }

    if (automaticallyAddToOtherVectors) {
        pathLabelInputTemplates.push_back(argTemplate);
        pathLabelInputValues.push_back(text);
        pathLabelFormattedValues.push_back({});
    }
    pathLabelInputs.push_back(std::move(lbl));


    // If the string is not empty, we have to get lbl again... yayyyyyyyyyyyyyyyyy
    // Remember, lbl is now invalid because we've just moved it into the pathLabelInputs vector.
    if (text.isNotEmpty()) {
        auto lblFromVec = pathLabelInputs.back().get();
        // Manually set text to trigger listener callback
        lblFromVec->setText(text, sendNotification);
        addAndMakeVisible(*lblFromVec);
    }
}

void OSCActionConstructor::MainComp::comboBoxChanged(ComboBox *comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == &tpltCategoryDd) {
        lastIndex = -1;
        currentCategory = dDitemIDtoCategory.at(tpltCategoryDd.getSelectedId());
        // Update the other dropdown's elements to match the options available for this category
        changeTpltDdBasedOnTpltCategory(currentCategory);
        return;
    }
    // This means our template has changed... i.e., we need to change EVERYTHING.
    if (comboBoxThatHasChanged == &tpltDd) {
        // Using the current category, we'll find the appropriate template according to the ID of the
        // tpltDd dropdown, whose ID will be equivalent to the index of the appropriate XM32Template in the
        // XM32TemplateGroup's vector.
        if (tpltDd.getSelectedItemIndex() == lastIndex) {
            return;
        }

        lastIndex = tpltDd.getSelectedItemIndex();
        const XM32TemplateGroup &tpltGroup = TEMPLATE_CATEGORY_MAP.at(currentCategory);
        currentTemplateCopy.reset(
            new XM32Template(tpltGroup.templates.at(lastIndex))
        );

        // Set fade command appropriately
        enableFadeCommandBtn.setEnabled(currentTemplateCopy->FADE_ENABLED);
        if (!currentTemplateCopy->FADE_ENABLED) {
            enableFadeCommandBtn.setToggleState(false, NotificationType::dontSendNotification);
        }


        // Clear the path label input vectors to hard-reset all path labels
        pathLabelInputs.clear();
        pathLabelInputTemplates.clear();
        pathLabelInputValues.clear();

        reconstructImage();
        repaint();
        return;
    }
}


void OSCActionConstructor::MainComp::labelTextChanged(Label *labelThatHasChanged) {
    size_t pathLabelInputsSize = pathLabelInputs.size();

    if (pathLabelInputsSize != pathLabelInputValues.size() ||
        pathLabelInputsSize != pathLabelInputTemplates.size()) {
        jassertfalse; // Uh oh... someone forgot to make sure the 3 vectors were properly updated...
        return;
    }
    for (size_t i = 0; i < pathLabelInputs.size(); i++) {
        // Check pointer validity
        auto& lblUniquePtr = pathLabelInputs.at(i);
        if (lblUniquePtr == nullptr) {
            jassertfalse; // ... I... I.. BUT HOW???
            continue;
        }

        // Get pointer, check if correct label, get text value, then send off to be validated.
        auto lbl = lblUniquePtr.get();
        if (labelThatHasChanged != lbl) {
            continue;
        }
        auto txt = labelThatHasChanged->getText();
        pathLabelInputValues.at(i) = txt;

        // Let's first figure out what we need to convert the text to
        bool labelErrorState = true;  // This is to track if any part of the label parsing process resulted in an
        // invalid value.

        const NonIter& argTemplate = pathLabelInputTemplates.at(i);
        switch (argTemplate._meta_PARAMTYPE) {
            case INT: {
                if (txt.isEmpty()) {
                    break; // Not worth continuing to process.
                }
                if (!txt.containsOnly("0123456789")) {
                    // Handle error here!
                    break;
                }
                int intVal = txt.getIntValue();
                if (!validateTextInput(intVal, argTemplate)) {
                    // Handle error here!
                    break;
                }
                pathLabelFormattedValues.at(i).changeStore(intVal);
                labelErrorState = false;
                break;
            }
            case STRING: {
                if (!validateTextInput(txt, argTemplate)) {
                    // Handle error here!
                    break;
                }
                pathLabelFormattedValues.at(i).changeStore(txt.toStdString());
                labelErrorState = false;
                break;
            }
            default:
                jassertfalse; // Invalid ParamType for in-path argument template
        }

        setPathLabelErrorState(lbl, labelErrorState);
    }
}


void OSCActionConstructor::MainComp::setPathLabelErrorState(Label *lbl, bool error) {
    lbl->setColour(Label::ColourIds::outlineColourId,
        error ? UICfg::NEGATIVE_BUTTON_COLOUR: UICfg::POSITIVE_BUTTON_COLOUR);
    lbl->setColour(Label::ColourIds::textColourId,
        error ? UICfg::NEGATIVE_BUTTON_COLOUR: UICfg::TEXT_ACCENTED_COLOUR);
}


// ==============================================================================


void Encoder::resized() {
    // The encoder rotary can handle its own bounds
    encoder.setBounds(getLocalBounds());

    // We still have to manually set the bounds for the manual input box.
    auto bounds = getLocalBounds().removeFromTop(0.9 * getHeight());
    bounds.removeFromLeft(0.05 * bounds.getWidth());
    bounds.removeFromRight(0.05 * bounds.getWidth());

    auto inputBoxBounds = Rectangle<int>(bounds.getCentreX() - getWidth() * 0.25,
                                         bounds.getCentreY() - getHeight() * 0.0625,
                                         getWidth() * 0.5, getHeight() * 0.125);
    manualInputBox.setBounds(inputBoxBounds);
}



Encoder::Encoder(
    Units unit, const double minDeg, const double minValue, const double maxDeg, const double maxValue,
    const double middlePV, const double defaultPV,
    const String &minLabel, const String &maxLabel,
    const int overrideRoundingToXDecimalPlaces, const ParamType paramType, const bool middleProvidedAsPercentage,
    const bool defaultProvidedAsPercentage): unit(unit),
                                             roundTo((overrideRoundingToXDecimalPlaces == -1)
                                                         ? ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT.at(unit)
                                                         : overrideRoundingToXDecimalPlaces),
                                             paramType(paramType),
                                             minValue(minValue), minPos(degreesToRadians(minDeg)), minLabel(minLabel),
                                             maxValue(maxValue), maxPos(degreesToRadians(maxDeg)), maxLabel(maxLabel),
                                             encoder(unit, minDeg, minValue, maxDeg, maxValue,
                                                     middlePV, defaultPV, minLabel, maxLabel,
                                                     overrideRoundingToXDecimalPlaces, paramType,
                                                     middleProvidedAsPercentage, defaultProvidedAsPercentage),
                                             _roundingMultiplier(std::pow(10.0, roundTo)), _isOptionParam(false),
                                             _isEnumParam(false), _option(nullOption), _enumParam(nullEnum) {
    if (minValue >= maxValue) {
        jassertfalse; // Min value must be less than max value
        return;
    }
    init();
}

Encoder::Encoder(const OptionParam &option, const double minDeg, const double maxDeg, const int defaultIndex,
                 const String &minLabel, const String &maxLabel): unit(Units::NONE),
                                                                  roundTo(-1),
                                                                  paramType(ParamType::LINF),
                                                                  minValue(0), minPos(degreesToRadians(minDeg)),
                                                                  minLabel(minLabel),
                                                                  maxValue(option.len - 1),
                                                                  maxPos(degreesToRadians(maxDeg)),
                                                                  maxLabel(maxLabel),
                                                                  encoder(option, minDeg, maxDeg, defaultIndex,
                                                                          minLabel,
                                                                          maxLabel),
                                                                  _roundingMultiplier(-1.0), _isOptionParam(true),
                                                                  _isEnumParam(false),
                                                                  _option(option), _enumParam(nullEnum) {
    init();
}

Encoder::Encoder(const EnumParam &enumParam, const double minDeg, const double maxDeg, const int defaultIndex,
                 const String &minLabel, const String &maxLabel): unit(NONE),
                                                                  roundTo(-1),
                                                                  paramType(LINF),
                                                                  minValue(0), minPos(degreesToRadians(minDeg)),
                                                                  minLabel(minLabel),
                                                                  maxValue(enumParam.len - 1),
                                                                  maxPos(degreesToRadians(maxDeg)),
                                                                  maxLabel(maxLabel),
                                                                  encoder(enumParam, minDeg, maxDeg, defaultIndex,
                                                                          minLabel,
                                                                          maxLabel),
                                                                  _roundingMultiplier(-1.0), _isOptionParam(false),
                                                                  _isEnumParam(true),
                                                                  _option(nullOption), _enumParam(enumParam) {
    init();
}



// TODO: Remove redundant isOption/EnumParam, and option and enumParam members
EncoderRotary::EncoderRotary(
    Units unit,
    const double minDeg, const double minValue,
    const double maxDeg, const double maxValue,
    const double middlePV,
    const double defaultPV,
    const String &minLabel,
    const String &maxLabel,
    const int overrideRoundingToXDecimalPlaces,
    const ParamType paramType,
    const bool middleProvidedAsPercentage,
    const bool defaultProvidedAsPercentage): paramType(paramType),
                                             minValue(minValue), minPos(degreesToRadians(minDeg)), minLabel(minLabel),
                                             maxValue(maxValue), maxPos(degreesToRadians(maxDeg)), maxLabel(maxLabel),
                                             isOptionParam(false), isEnumParam(false),
                                             Slider(RotaryHorizontalVerticalDrag, NoTextBox),
                                             option(nullOption), enumParam(nullEnum) {
    if (minValue >= maxValue) {
        jassertfalse; // Min value must be less than max value
        return;
    }
    if (middleProvidedAsPercentage) {
        middleValue = inferValueFromMinMaxAndPercentage(minValue, maxValue, middlePV, paramType);
        middlePos = degreesToRadians(minDeg + (maxDeg - minDeg) * middlePV);
    } else {
        middleValue = middlePV;
        // Normalise middlePV to the range of minValue and maxValue
        middlePos = degreesToRadians(
            minDeg + (maxDeg - minDeg) * inferPercentageFromMinMaxAndValue(
                minValue, maxValue, middlePV, paramType));
    }
    if (defaultProvidedAsPercentage) {
        defaultValue = inferValueFromMinMaxAndPercentage(minValue, maxValue, defaultPV, paramType);
        defaultPos = degreesToRadians(minDeg + (maxDeg - minDeg) * defaultPV);
    } else {
        defaultValue = defaultPV;
        defaultPos = degreesToRadians(
            minDeg + (maxDeg - minDeg) *
            inferPercentageFromMinMaxAndValue(minValue, maxValue, defaultPV, paramType));
    }
    setDoubleClickReturnValue(true, defaultValue);
    setValue(defaultValue);


    int roundTo = (overrideRoundingToXDecimalPlaces == -1)
                      ? ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT.at(unit)
                      : overrideRoundingToXDecimalPlaces;

    switch (paramType) {
        case LINF:
            setRange(minValue, maxValue, std::pow(10.0, -roundTo));
            break;
        case LOGF:
            setNormalisableRange(getNormalisableRangeExp(minValue, maxValue));
                break;
        case LEVEL_161:
            setNormalisableRange(LEVEL_161_NORMALISABLE_RANGE);
            break;
        case LEVEL_1024:
            setNormalisableRange(LEVEL_1024_NORMALISABLE_RANGE);
            break;
        default:
            jassertfalse;
    }
}


EncoderRotary::EncoderRotary(const OptionParam &option, const double minDeg, const double maxDeg,
                             const int defaultIndex,
                             const String &minLabel, const String &maxLabel): isOptionParam(true), isEnumParam(false),
                                                                              option(option),
                                                                              paramType(LINF),
                                                                              minValue(0),
                                                                              minPos(degreesToRadians(minDeg)),
                                                                              minLabel(minLabel),
                                                                              maxValue(option.len - 1),
                                                                              maxPos(degreesToRadians(maxDeg)),
                                                                              maxLabel(maxLabel),
                                                                              Slider(
                                                                                  SliderStyle::RotaryHorizontalVerticalDrag,
                                                                                  Slider::NoTextBox),
                                                                              enumParam(nullEnum) {
    // Let's set the default value and position
    setDoubleClickReturnValue(true, defaultIndex);
    setValue(defaultIndex);
    setRange(0, option.len - 1, 1); // Set the range from 0 to len - 1 with a step of 1
}


EncoderRotary::EncoderRotary(const EnumParam &enumParam, const double minDeg, const double maxDeg,
                             const int defaultIndex, const String &minLabel, const String &maxLabel)
    : isOptionParam(false), isEnumParam(true),
      enumParam(enumParam),
      paramType(LINF),
      minValue(0),
      minPos(degreesToRadians(minDeg)),
      minLabel(minLabel),
      maxValue(option.len - 1),
      maxPos(degreesToRadians(maxDeg)),
      maxLabel(maxLabel),
      Slider(RotaryHorizontalVerticalDrag,
          NoTextBox),
      option(nullOption) {
    // Let's set the default value and position
    setDoubleClickReturnValue(true, defaultIndex);
    setValue(defaultIndex);
    setRange(0, enumParam.len - 1, 1); // Set the range from 0 to len - 1 with a step of 1
}


void EncoderRotary::resized() {
    // Let's set up our encoder bounds.
    boundsCopy = getLocalBounds();
    boundsCopy.removeFromBottom(getLocalBounds().getHeight() * 0.1);
    // Labels will be drawn at the bottom 10% of the bounds.
    // Use getLocalBounds() as boundsCopy will have change after a removeFromLeft call, meaning the removeFromRight will
    // remove 5% of the remaining 95% width, not the original width.
    boundsCopy.removeFromLeft(getLocalBounds().getWidth() * 0.05);
    boundsCopy.removeFromRight(getLocalBounds().getWidth() * 0.05);
    encoderRadius = jmin(boundsCopy.getWidth(), boundsCopy.getHeight()) * 0.5f;
    auto center = boundsCopy.getCentre();

    textHeight = encoderRadius * UICfg::ROTARY_TEXT_HEIGHT;


    // Now draw label to image to prevent rerendering every paint call.
    predrawnLabels = Image(Image::ARGB, getWidth(), getHeight(), true);
    Graphics g(predrawnLabels);

    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
    auto labelHeights = textHeight * 0.7f;
    g.setFont(labelHeights);

    // This is the Minimum Label
    Rectangle<float> textBound;
    textBound.setSize(GlyphArrangement::getStringWidthInt(g.getCurrentFont(), minLabel), labelHeights);
    textBound.setCentre(center.getPointOnCircumference(encoderRadius + labelHeights * 0.75f, minPos));
    textBound.setY(textBound.getY() + labelHeights);

    g.drawFittedText(minLabel, textBound.toNearestInt(), Justification::topLeft, 1);

    // This is the Maximum Label
    textBound.setSize(GlyphArrangement::getStringWidthInt(g.getCurrentFont(), maxLabel), labelHeights);
    textBound.setCentre(center.getPointOnCircumference(encoderRadius + labelHeights * 0.75f, maxPos));
    textBound.setY(textBound.getY() + labelHeights);

    g.drawFittedText(maxLabel, textBound.toNearestInt(), Justification::topRight, 1);
}


void EncoderRotary::paint(Graphics &g) {
    g.setColour(Colours::red);

    auto enabled = isEnabled();

    g.setColour(enabled ? UICfg::ROTARY_ENABLED_COLOUR : UICfg::ROTARY_DISABLED_COLOUR);
    g.fillEllipse(boundsCopy.toFloat());

    auto center = boundsCopy.getCentre();

    Path pointer;

    // We'll make a pointer rectangle.
    Rectangle<float> r;
    // Let's figure out the pointer width
    float ptrWdth = encoderRadius * UICfg::ROTARY_POINTER_WIDTH;
    r.setLeft(center.getX() - ptrWdth);
    r.setRight(center.getX() + ptrWdth);
    r.setTop(boundsCopy.getY());
    r.setBottom(center.getY() - textHeight * UICfg::ROTARY_TEXT_PADDING);

    pointer.addRoundedRectangle(r, ptrWdth);

    float sliderAngleAsRadian = jmap(getNormalisableRange().convertTo0to1(getValue()),
                                     minPos, maxPos);
    pointer.applyTransform(AffineTransform::rotation(sliderAngleAsRadian, center.getX(), center.getY()));

    g.setColour(UICfg::ROTARY_POINTER_COLOUR);
    g.fillPath(pointer);


    // Draw the pre-drawn labels
    g.drawImage(predrawnLabels, getLocalBounds().toFloat());
}
