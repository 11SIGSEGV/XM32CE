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

void OSCActionConstructor::MainComp::resized() {
    auto bounds = getLocalBounds();
    auto heightTenths = bounds.getHeight() * 0.1;

    templateSelectionBox = bounds.removeFromTop(heightTenths * 2);
    pathBox = bounds.removeFromTop(heightTenths * 2);
    buttonsBox = bounds.removeFromBottom(heightTenths);

}

void OSCActionConstructor::MainComp::paint(Graphics &g) {
    g.setColour(Colours::red);
    g.drawRect(templateSelectionBox);
    g.drawRect(pathBox);
    g.drawRect(buttonsBox);
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
                 const String &minLabel, const String &maxLabel): unit(Units::NONE),
                                                                  roundTo(-1),
                                                                  paramType(ParamType::LINF),
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


void Encoder::paint(Graphics &g) {
    // encoder.paint(g);
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


EncoderRotary::EncoderRotary(const OptionParam &option, const double minDeg, const double maxDeg,
                             const int defaultIndex,
                             const String &minLabel, const String &maxLabel): isOptionParam(true), isEnumParam(false),
                                                                              option(option),
                                                                              paramType(ParamType::LINF),
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
      paramType(ParamType::LINF),
      minValue(0),
      minPos(degreesToRadians(minDeg)),
      minLabel(minLabel),
      maxValue(option.len - 1),
      maxPos(degreesToRadians(maxDeg)),
      maxLabel(maxLabel),
      Slider(
          SliderStyle::RotaryHorizontalVerticalDrag,
          Slider::NoTextBox),
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
