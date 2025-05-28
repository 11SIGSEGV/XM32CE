/*
  ==============================================================================

    AppComponents.cpp
    Created: 28 May 2025 3:53:04pm
    Author:  anony

  ==============================================================================
*/

#include "AppComponents.h"

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
                                             middlePerc(middlePerc),
                                             defaultPerc(defaultPerc),
                                             minValue(minValue), minPos(degreesToRadians(minDeg)), minLabel(minLabel),
                                             maxValue(maxValue), maxPos(degreesToRadians(maxDeg)), maxLabel(maxLabel),
                                             encoder(unit, minDeg, minValue, maxDeg, maxValue,
                                                     middlePV, defaultPV, minLabel, maxLabel,
                                                     overrideRoundingToXDecimalPlaces, paramType,
                                                     middleProvidedAsPercentage, defaultProvidedAsPercentage),
                                             _roundingMultiplier(std::pow(10.0, roundTo)) {
    if (minValue >= maxValue) {
        jassertfalse; // Min value must be less than max value
        return;
    }
    manualInputBox.setText(getValueAsDisplayString());
    manualInputBox.addListener(this);
    manualInputBox.setJustification(Justification::centred);
    encoder.addListener(this);
    addAndMakeVisible(encoder);
    addAndMakeVisible(manualInputBox);
}


void Encoder::paint(Graphics &g) {
    // encoder.paint(g);
}


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
                                             middlePerc(middlePerc),
                                             defaultPerc(defaultPerc),
                                             minValue(minValue), minPos(degreesToRadians(minDeg)), minLabel(minLabel),
                                             maxValue(maxValue), maxPos(degreesToRadians(maxDeg)), maxLabel(maxLabel),
                                             Slider(SliderStyle::RotaryHorizontalVerticalDrag, Slider::NoTextBox) {
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
        defaultValue = inferValueFromMinMaxAndPercentage(minValue, maxValue, defaultPerc, paramType);
        defaultPos = degreesToRadians(minDeg + (maxDeg - minDeg) * defaultPerc);
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
        case ParamType::LINF:
            setRange(minValue, maxValue, std::pow(10.0, -roundTo));
            break;
        case ParamType::LOGF:
            setNormalisableRange(getNormalisableRangeExp(minValue, maxValue));
            break;
        case ParamType::LEVEL_161:
            setNormalisableRange(LEVEL_161_NORMALISABLE_RANGE);
            break;
        case ParamType::LEVEL_1024:
            setNormalisableRange(LEVEL_1024_NORMALISABLE_RANGE);
            break;
        default:
            jassertfalse;
    }
}


void EncoderRotary::paint(Graphics &g) {
    auto boundsCopy = getLocalBounds();
    g.setColour(Colours::red);

    auto manualInputBounds = boundsCopy.removeFromBottom(getLocalBounds().getHeight() * 0.1);
    // Use getLocalBounds() as boundsCopy will have change after a removeFromLeft call, meaning the removeFromRight will
    // remove 5% of the remaining 95% width, not the original width.
    boundsCopy.removeFromLeft(getLocalBounds().getWidth() * 0.05);
    boundsCopy.removeFromRight(getLocalBounds().getWidth() * 0.05);
    const float radius = jmin(boundsCopy.getWidth(), boundsCopy.getHeight()) * 0.5f;

    auto enabled = isEnabled();

    g.setColour(enabled ? UICfg::ROTARY_ENABLED_COLOUR : UICfg::ROTARY_DISABLED_COLOUR);
    g.fillEllipse(boundsCopy.toFloat());

    auto center = boundsCopy.getCentre();

    Path pointer;
    // To figure out the padding for the pointer, we need to know the text height BEFORE the pointer is drawn.
    const float textHeight = radius * UICfg::ROTARY_TEXT_HEIGHT;

    // We'll make a pointer rectangle.
    Rectangle<float> r;
    // Let's figure out the pointer width
    float ptrWdth = radius * UICfg::ROTARY_POINTER_WIDTH;
    r.setLeft(center.getX() - ptrWdth);
    r.setRight(center.getX() + ptrWdth);
    r.setTop(boundsCopy.getY());
    r.setBottom(center.getY() - textHeight * UICfg::ROTARY_TEXT_PADDING);

    pointer.addRoundedRectangle(r, ptrWdth); // TODO: Check if this actually remotely makes sense

    float sliderAngleAsRadian = jmap(getNormalisableRange().convertTo0to1(getValue()),
                                     minPos, maxPos);
    pointer.applyTransform(AffineTransform::rotation(sliderAngleAsRadian, center.getX(), center.getY()));

    g.setColour(UICfg::ROTARY_POINTER_COLOUR);
    g.fillPath(pointer);


    // Now let's draw the min-max labels.
    g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);

    // Now to TRY draw the labels. Emphasis on Try.
    auto labelHeights = textHeight * 0.7f;
    g.setFont(labelHeights);
    // This is the Minimum Label
    Rectangle<float> textBound;
    textBound.setSize(GlyphArrangement::getStringWidthInt(g.getCurrentFont(), minLabel), labelHeights);
    textBound.setCentre(center.getPointOnCircumference(radius + labelHeights * 0.75f, minPos));
    textBound.setY(textBound.getY() + labelHeights);

    g.drawFittedText(minLabel, textBound.toNearestInt(), Justification::topLeft, 1);

    // This is the Maximum Label
    textBound.setSize(GlyphArrangement::getStringWidthInt(g.getCurrentFont(), maxLabel), labelHeights);
    textBound.setCentre(center.getPointOnCircumference(radius + labelHeights * 0.75f, maxPos));
    textBound.setY(textBound.getY() + labelHeights);

    g.drawFittedText(maxLabel, textBound.toNearestInt(), Justification::topRight, 1);
}
