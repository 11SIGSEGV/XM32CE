#include "MainComponent.h"

//==============================================================================

void AppLnF::drawRotarySlider(Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, Slider &slide) {
    Rectangle<float> bounds(x, y, width, height);
    auto bottomLabelBounds = bounds.removeFromBottom(bounds.getHeight()*0.1);
    bounds.removeFromLeft(bounds.getHeight()*0.05); bounds.removeFromRight(bounds.getHeight()*0.05);
    const float radius = jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    auto enabled = slide.isEnabled();

    g.setColour(enabled ? UICfg::ROTARY_ENABLED_COLOUR : UICfg::ROTARY_DISABLED_COLOUR);
    g.fillEllipse(bounds);


    if (auto* uiRtry = dynamic_cast<UIRotary*>(&slide)) {
        auto center = bounds.getCentre();

        Path pointer;
        // To figure out the padding for the pointer, we need to know the text height BEFORE the pointer is drawn.
        const float textHeight = radius * UICfg::ROTARY_TEXT_HEIGHT;

        // We'll make a pointer rectangle.
        Rectangle<float> r;
        // Let's figure out the pointer width
        float ptrWdth = radius * UICfg::ROTARY_POINTER_WIDTH;
        r.setLeft(center.getX() - ptrWdth);
        r.setRight(center.getX() + ptrWdth);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - textHeight * UICfg::ROTARY_TEXT_PADDING);

        pointer.addRoundedRectangle(r, ptrWdth); // TODO: Check if this actually remotely makes sense

        float sliderAngleAsRadian = jmap(sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
        pointer.applyTransform(AffineTransform::rotation(sliderAngleAsRadian, center.getX(), center.getY()));

        g.setColour(UICfg::ROTARY_POINTER_COLOUR);
        g.fillPath(pointer);

        // Now, let's draw the value
        String displayString = uiRtry->getValueAsDisplayString();
        g.setFont(UICfg::DEFAULT_MONOSPACE_FONT);
        g.setFont(textHeight); // TODO: Test if this overrides above

        auto textWidth = GlyphArrangement::getStringWidthInt(g.getCurrentFont(), displayString);
        r.setSize(textWidth + textWidth * 0.25f, textHeight + textHeight * 0.25f);
        r.setCentre(bounds.getCentre());

        // Ok, we'll draw a rectangle behind the text, and the text
        g.setColour(enabled ? Colours::black : Colours::darkgrey);
        g.fillRect(r);
        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(displayString, r.toNearestInt(), Justification::centred, 1);

        auto minMaxLabels = uiRtry->getMinMaxLabel();

        // Now to TRY draw the labels. Emphasis on Try.
        auto labelHeights = textHeight * 0.7f;
        g.setFont(labelHeights);
        // This is the Minimum Label
        Rectangle<float> textBound;
        textBound.setSize(GlyphArrangement::getStringWidthInt(g.getCurrentFont(), minMaxLabels[0]), labelHeights);
        textBound.setCentre(center.getPointOnCircumference(radius + labelHeights * 0.75f, rotaryStartAngle));
        textBound.setY(textBound.getY() + labelHeights);

        g.drawFittedText(minMaxLabels[0], textBound.toNearestInt(), Justification::topLeft, 1);

        // This is the Maximum Label Label
        textBound.setSize(GlyphArrangement::getStringWidthInt(g.getCurrentFont(), minMaxLabels[1]), labelHeights);
        textBound.setCentre(center.getPointOnCircumference(radius + labelHeights * 0.75f, rotaryEndAngle));
        textBound.setY(textBound.getY() + labelHeights);

        g.drawFittedText(minMaxLabels[1], textBound.toNearestInt(), Justification::topRight, 1);

    }
}


UIRotary::UIRotary(
    Units unit, const double minDeg, const double minValue, const double maxDeg, const double maxValue,
    const double middlePV, const double defaultPV,
    const String &minLabel, const String &maxLabel,
    const int overrideRoundingToXDecimalPlaces, const ParamType paramType, const bool middleProvidedAsPercentage,
    const bool defaultProvidedAsPercentage):

    Slider(RotaryHorizontalVerticalDrag, NoTextBox), unit(unit),
    roundTo((overrideRoundingToXDecimalPlaces == -1) ?
        ROUND_TO_NUM_DECIMAL_PLACES_FOR_UNIT.at(unit) : overrideRoundingToXDecimalPlaces),
    paramType(paramType),
    middlePerc(middlePerc),
    defaultPerc(defaultPerc),
    minValue(minValue), minLabelPos(degreesToRadians(minDeg)), minLabel(minLabel),
    maxValue(maxValue), maxLabelPos(degreesToRadians(maxDeg)), maxLabel(maxLabel) {
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
    testRotary.setBounds(10, 300, 180, 180);
    testRotary2.setBounds(300, 300, 180, 180);
    testRotary.setLookAndFeel(&lnf);
    testRotary2.setLookAndFeel(&lnf);

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


void UIRotary::paint(Graphics &g) {
    auto bounds = getLocalBounds();
    getLookAndFeel().drawRotarySlider(
        g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
        getNormalisableRange().convertTo0to1(getValue()),
        degreesToRadians(180.f + 45.f), degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi,
        *this);
}
