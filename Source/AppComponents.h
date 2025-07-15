/*
  ==============================================================================

    AppComponents.h
    Created: 28 May 2025 3:53:04pm
    Author:  anony

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Helpers.h"


// TODO: Test if setting non-default minDeg and maxDeg works as expected... I'm scared.
struct EncoderRotary : public Slider {
    explicit EncoderRotary(Units unit,
                           double minDeg = -135.0, double minValue = 0.0,
                           double maxDeg = 135.0, double maxValue = 1.0,
                           double middlePV = 0.5,
                           double defaultPV = 0.0,
                           const String &minLabel = "",
                           const String &maxLabel = "",
                           int overrideRoundingToXDecimalPlaces = -1,
                           ParamType paramType = ParamType::LINF,
                           bool middleProvidedAsPercentage = true,
                           bool defaultProvidedAsPercentage = true
    );

    // We'll also allow Enum and OptionParams
    explicit EncoderRotary(const OptionParam &option,
                           double minDeg = -135.0,
                           double maxDeg = 135.0,
                           int defaultIndex = 0,
                           const String &minLabel = "",
                           const String &maxLabel = ""
    );

    explicit EncoderRotary(const EnumParam &enumParam,
                           double minDeg = -135.0,
                           double maxDeg = 135.0,
                           int defaultIndex = 0,
                           const String &minLabel = "",
                           const String &maxLabel = ""
    );

    void resized() override;

    void paint(Graphics &) override;

private:
    const bool isOptionParam;
    const bool isEnumParam;
    OptionParam option; // Only used if isOptionParam is true
    EnumParam enumParam; // Only used if isEnumParam is true
    Rectangle<int> Bounds;
    double middlePos;
    double middleValue; // Middle value is automatically generated from min and max value

    const double minPos;
    const String minLabel;
    const double minValue;
    const double maxPos;
    const String maxLabel;
    const double maxValue;

    double defaultPos;
    double defaultValue;

    const ParamType paramType;

    Image predrawnLabels;
    float encoderRadius;
    Rectangle<int> boundsCopy;
    float textHeight;
};


struct Encoder : public Component, public Slider::Listener, public TextEditor::Listener {
    /* Initalise UIRotary.
     * When middle/defaultProvidedAsPercentage is true, the middle and default values are
     * automatically interpolated based on the middlePV/defaultPV percentage and the minValue and maxValue.
     * Otherwise, the middlePV and defaultPV are used as the actual values.
     */
    explicit Encoder(Units unit,
                     double minDeg = -135.0, double minValue = 0.0,
                     double maxDeg = 135.0, double maxValue = 1.0,
                     double middlePV = 0.5,
                     double defaultPV = 0.0,
                     const String &minLabel = "",
                     const String &maxLabel = "",
                     int overrideRoundingToXDecimalPlaces = -1,
                     ParamType paramType = LINF,
                     bool middleProvidedAsPercentage = true,
                     bool defaultProvidedAsPercentage = true
    );

    explicit Encoder(const OptionParam &option,
                     double minDeg = -135.0,
                     double maxDeg = 135.0,
                     int defaultIndex = 0,
                     const String &minLabel = "",
                     const String &maxLabel = "");

    explicit Encoder(const EnumParam &enumParam,
                     double minDeg = -135.0,
                     double maxDeg = 135.0,
                     int defaultIndex = 0,
                     const String &minLabel = "",
                     const String &maxLabel = "");

    void init() {
        manualInputBox.setText(getValueAsDisplayString());
        manualInputBox.addListener(this);
        manualInputBox.setJustification(Justification::centred);
        encoder.addListener(this);
        addAndMakeVisible(encoder);
        addAndMakeVisible(manualInputBox);
    }

    ~Encoder() override {
        encoder.removeListener(this);
        manualInputBox.removeListener(this);
        setLookAndFeel(nullptr);
        deleteAllChildren();
    }


    String getValueAsDisplayString() const {
        // Value should be index when isOptionParam or isEnumParam, and a numerical value otherwise.
        auto value = encoder.getValue();

        if (_isOptionParam) {
            return _option.value.at(static_cast<int>(value));
        }
        if (_isEnumParam) {
            return _enumParam.value.at(static_cast<int>(value));
        }
        switch (unit) {
            case HERTZ: {
                return value < 1000.0 ? String(value, roundTo) + " Hz" : String(value / 1000.0, roundTo) + " kHz";
            }
            case DB: {
                if (value <= -90.0) {
                    return "-inf";
                }
                return value <= -90.0
                           ? "-inf"
                           : (
                                 (value > 0) ? "+" + String(value, roundTo) : String(value, roundTo)
                             ) + " dB";
            }
            case NONE: default:
                return String(value, roundTo);
        }
    }


    void sliderValueChanged(Slider *) override {
        // When value changed, we'll update the manual input box
        manualInputBox.setText(getValueAsDisplayString());
    }

    void textEditorReturnKeyPressed(TextEditor &editor) override {
        // Do checks to see if the value is valid
        auto [valid, value] = getDoubleValueFromTextEditor(editor.getText());
        if (valid) {
            std::cout << value << std::endl;
            // If valid, set the value to the encoder
            // Check if the value is within the range of the encoder
            if (value < minValue || value > maxValue) {
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                                 "Invalid Input",
                                                 "Value is out of range. Please enter a value between "
                                                 + String(minValue) + " and " + String(maxValue) + ".");
                return;
            }
            // Set the rounded value.
            encoder.setValue(std::round(value * _roundingMultiplier) / _roundingMultiplier);
        } else {
            // If not valid, show an error message
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                             "Invalid Input",
                                             "Please enter a valid value.");
        }
    }

    // void sliderDragStarted(Slider *) override {};
    // void sliderDragEnded(Slider *) override {};


    void paint(Graphics &g) override {};

    void resized() override;

private:
    EncoderRotary encoder;

    TextEditor manualInputBox{"manualInputBox"};

    Rectangle<int> Bounds;
    const bool _isOptionParam;
    const bool _isEnumParam;
    OptionParam _option; // Only used if isOptionParam is true
    EnumParam _enumParam; // Only used if isEnumParam is true

    double middlePos;
    double middleValue; // Middle value is automatically generated from min and max value

    const double minPos;
    const String minLabel;
    const double minValue;
    const double maxPos;
    const String maxLabel;
    const double maxValue;

    double defaultPos;
    double defaultValue;

    const Units unit;
    const int roundTo;
    const double _roundingMultiplier;
    const ParamType paramType;
};


struct Fader: public Component, public Slider::Listener {

    struct FaderSlider: public Slider {
        FaderSlider(): Slider(LinearBarVertical, NoTextBox) {
            setRange(0.0, 1.0);
            setColour(backgroundColourId, UICfg::TRANSPARENT);
            setColour(thumbColourId, UICfg::TRANSPARENT);
            setColour(trackColourId, UICfg::TRANSPARENT);
            setColour(rotarySliderFillColourId, UICfg::TRANSPARENT);
            setColour(rotarySliderOutlineColourId, UICfg::TRANSPARENT);
            setColour(textBoxTextColourId, UICfg::TRANSPARENT);
            setColour(textBoxBackgroundColourId, UICfg::TRANSPARENT);
            setColour(textBoxHighlightColourId, UICfg::TRANSPARENT);
            setColour(textBoxOutlineColourId , UICfg::TRANSPARENT);
        }
        void paint(Graphics &g) override {};
    };
    // A bound width:height of 3:8 is recommended (e.g., 150 width, 400 height).
    Fader(const NonIter& nonIter): is1024(nonIter._meta_PARAMTYPE == LEVEL_1024) {
        if (nonIter._meta_PARAMTYPE != LEVEL_1024 && nonIter._meta_PARAMTYPE != LEVEL_161) {
            jassertfalse; // Unsupported ParamType
        }
        slider = std::make_unique<FaderSlider>();
        slider->setDoubleClickReturnValue(true,
            inferPercentageFromMinMaxAndValue(nonIter.floatMin, nonIter.floatMax, nonIter.defaultFloatValue, nonIter._meta_PARAMTYPE));
        addAndMakeVisible(*slider);
        slider->addListener(this);
    }

    void resized() override {
        auto boundsWidth = getWidth();
        faderHeight = boundsWidth*UICfg::FADER_KNOB_WIDTH_AS_PROPORTION_TO_BOUNDS_WIDTH*UICfg::FADER_KNOB_HEIGHT_AS_PROPORTION_TO_WIDTH;

        auto boundsHeight = getHeight();
        boundsHeight -= faderHeight;
        boundsHeightWithoutFaderOverflow = boundsHeight;


        bgImage = Image(Image::ARGB, boundsWidth, boundsHeight, true);
        slider->setBounds(Rectangle<int>(0, faderHeight / 2, getWidth(), getHeight() - faderHeight));
        Graphics g(bgImage);

        // Centerline
        Rectangle<int> centerBarRect = getLocalBounds();
        int sidePadding = static_cast<int>(std::ceil(boundsWidth * UICfg::FADER_CENTERLINE_SIDE_PADDING));
        centerBarRect.removeFromLeft(sidePadding);
        centerBarRect.removeFromRight(sidePadding);
        g.setColour(UICfg::CENTERLINE_COLOUR);
        g.fillRect(centerBarRect);


        // Let's also set our value label font height
        monospaceFontWithFontSize.setHeight(boundsHeight * UICfg::FADER_VALUE_TEXT_FONT_HEIGHT);

        // Level markers
        int lineThickness = std::ceil(boundsHeight * UICfg::LEVEL_MARKER_THICKNESS);
        int lineSidePadding = std::ceil(boundsWidth * 0.35f);
        int labelToMarkerPadding = std::ceil(lineSidePadding * UICfg::STD_PADDING); // Padding between the marker line and the text labelling the line

        float textHeight = boundsHeight * UICfg::FADER_MARKER_LABEL_FONT_HEIGHT;
        g.setFont(levelLabelFont);
        g.setFont(textHeight);

        for (const auto& [lbl, heightPerc]: LEVEL_MARKERS) {
            if (heightPerc > 1.f  || heightPerc < 0.f) {
                jassertfalse; // Right... and no one sees a problem with this right?
            }
            int centerHeight = std::ceil(heightPerc * boundsHeight);
            int initialY;
            // The code below ensures the initial Y-coord does not go outside the bounds.
            if (centerHeight == 0) {
                // At top height
                initialY = 0;
            } else if (centerHeight == boundsHeight) {
                // At bottom height
                initialY = boundsHeight - lineThickness;
            } else {
                initialY = centerHeight - lineThickness;
            }
            // We need to leave some space on left & right for label markers (e.g., -90, -60, ..., +10)
            Rectangle<int> line = {lineSidePadding, initialY, boundsWidth - lineSidePadding * 2, lineThickness};
            g.drawRect(line);


            // Again, the code below ensures the initial Y-coord does not go outside the bounds.
            if (centerHeight == 0) {
                // At top height
                initialY = 0;
            } else if (centerHeight == boundsHeight) {
                // At bottom height
                initialY = boundsHeight - static_cast<int>(std::floor(textHeight));
            } else {
                initialY = static_cast<int>(std::floor(centerHeight - textHeight / 2));
            }

            // We also need the level label
            Rectangle<int> labelTextBox = {boundsWidth - lineSidePadding + labelToMarkerPadding,
                initialY, lineSidePadding - labelToMarkerPadding, static_cast<int>(std::floor(textHeight))
            };

            // Format text appropriately
            g.drawText(lbl, labelTextBox, Justification::centredLeft);
        }

        // Fader knob size is soley dependent on width.
        faderKnobImage = Image(Image::ARGB, boundsWidth*UICfg::FADER_KNOB_WIDTH_AS_PROPORTION_TO_BOUNDS_WIDTH,
            faderHeight,true);
        // Half of the fader knob width left of the centerline
        faderTopLeftX = std::ceil(
            boundsWidth / 2 - boundsWidth * UICfg::FADER_KNOB_WIDTH_AS_PROPORTION_TO_BOUNDS_WIDTH / 2);

        auto faderLineHeight = faderKnobImage.getHeight() * UICfg::FADER_KNOB_LINE_HEIGHT_AS_PROPORTION_TO_HEIGHT;

        Graphics gFdr(faderKnobImage);
        gFdr.setColour(UICfg::STRONG_BORDER_COLOUR);
        gFdr.fillRect(getLocalBounds());
        gFdr.setColour(UICfg::BG_SECONDARY_COLOUR);

        gFdr.fillRect(Rectangle<int>(0, (faderHeight - faderLineHeight) / 2, faderKnobImage.getWidth(), faderLineHeight));
    }


    void paint(Graphics &g) override {
        g.drawImage(bgImage, Rectangle<float>(0, faderHeight / 2, getWidth(), getHeight() - faderHeight));
        g.drawImageAt(faderKnobImage, faderTopLeftX, (1.0 - slider->getValue()) * boundsHeightWithoutFaderOverflow);

        g.setColour(UICfg::TEXT_COLOUR);
        g.setFont(monospaceFontWithFontSize);

        // Heavily inspired by https://forum.juce.com/t/draw-rotated-text/14695/11. Thanks matkatmusic!
        GlyphArrangement ga;
        String strVal = getFormattedStringVal();
        ga.addLineOfText(monospaceFontWithFontSize, strVal, 0, 0);
        Path p;
        ga.createPath(p);

        auto pathBounds = p.getBounds();
        auto centerX = static_cast<int>(std::ceil(getWidth() * UICfg::FADER_CENTERLINE_SIDE_PADDING)) / 2;
        auto centerY = getHeight()/2;

        p.applyTransform(AffineTransform()
                         .translated(centerX - GlyphArrangement::getStringWidthInt(monospaceFontWithFontSize, strVal) / 2 , centerY)
                         .rotated(-MathConstants<float>::halfPi,
                             centerX,
                             centerY
                             )
                         );
        g.fillPath(p);

        g.setColour(Colours::red);
        g.drawHorizontalLine(getLocalBounds().getCentreY(), 0, getWidth());
        g.drawRect(getLocalBounds());

    }

    void sliderValueChanged(Slider *sldr) override {
        DBG(String(getValue()));
        repaint();
    }

    [[nodiscard]] float getValue() const {
        if (is1024) {
            return XM32::floatToDb(slider->getValue());
        }
        return XM32::roundToNearest(XM32::floatToDb(slider->getValue()), levelValues_161);
    }

    String getFormattedStringVal() const {
        auto val = roundTo(getValue(), 2);
        if (val > 0) {
            return "+" + String(val) + "dB";
        }
        return String(val) + "dB";
    }
private:
    std::unique_ptr<FaderSlider> slider;

    float faderHeight;
    float boundsHeightWithoutFaderOverflow;
    int faderTopLeftX;
    Image bgImage;
    Image faderKnobImage;
    const bool is1024; // See if 1024 or 161 level
    Font monospaceFontWithFontSize = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);

    Font levelLabelFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);

    // Label to Height Proportion (i.e., % of height)
    const std::map<String, float> LEVEL_MARKERS = {
        {"-inf", 1.f},
        {"-60", 0.9375f},
        {"-50", 0.875},
        {"-40", 0.8125f},
        {"-30", 0.75f},
        {"-20", 0.625f},
        {"-10", 0.5f},
        {"-5", 0.375f},
        {"0", 0.25f},
        {"+5", 0.125f},
        {"+10", 0.f},
    };
};


class ParentWindowListener {
public:
    enum WindowType {
        AppComponents_OSCActionConstructor,
    };

    virtual ~ParentWindowListener() = default;

    // Please don't pass std::string uuid as reference... this is setting ourselves up for SIGSEGVs!
    virtual void closeRequested(WindowType windowType, std::string uuid) = 0;
};


class OSCActionConstructor : public DocumentWindow {
public:
    OSCActionConstructor(const std::string &uuid, String name = "Cue OSC Action Constructor") : DocumentWindow(name,
        UICfg::BG_COLOUR,
        allButtons), uuid(uuid) {
        centreWithSize(1000, 800);
        setUsingNativeTitleBar(true);
        setAlwaysOnTop(true);
        setContentOwned(new MainComp(), true);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        JUCEApplication::getInstance()->systemRequestedQuit(); // Temporarily here
        if (registeredListener != nullptr) {
            registeredListener->closeRequested(ParentWindowListener::AppComponents_OSCActionConstructor, uuid);
        }
    }

    void setParentListener(ParentWindowListener *lstnr) {
        registeredListener = lstnr;
    }

    void removeParentListener() { registeredListener = nullptr; }


    class MainComp : public Component, public ComboBox::Listener, public ToggleButton::Listener,
                     public Label::Listener {
    public:
        MainComp();

        ~MainComp() override {
        };

        void resized() override;

        void paint(Graphics &g) override;

        void reconstructImage();

        static bool validateTextInput(float input, const NonIter &argTemplate);

        static bool validateTextInput(int input, const NonIter &argTemplate);

        static bool validateTextInput(const String &input, const NonIter &argTemplate);

        void changeTpltDdBasedOnTpltCategory(const TemplateCategory category) {
            auto it = TEMPLATE_CATEGORY_MAP.find(category);
            if (it == TEMPLATE_CATEGORY_MAP.end()) {
                jassertfalse; // 🤦 how... how is this even possible?
            }
            const XM32TemplateGroup &tpltGroup = it->second;
            tpltDd.clear();
            int i = 0;
            for (const auto &tplt: tpltGroup.templates) {
                i++;
                tpltDd.addItem(tplt.NAME, i);
                // To access the correct template, simply use tpltGroup.templates[i-1].
            }
        }


        // Finds the appropriate bounds for an in-path argument label (i.e., text input field). Returns the bounds,
        // and removes it from the remainingBox from the left.
        // It is recommended to use a monospace font, but if this is not the case, the function assumes 'W' is the
        // longest character and uses it to determine the required string width.
        static Rectangle<int> findProperLabelBounds(const NonIter &argTemplate, Rectangle<int> &remainingBox,
                                                    const Font &fontInUse);

        // Creates and adds in an in-path argument input box. Calls findProperLabelBounds to properly bound the
        // new Label object.
        // If a non-empty value is provided for the 'text' argument, a Label::Listener::labelTextChanged is broadcasted.
        // automaticallyAddToOtherVectors is used when the input box created is 'new' and does not already have a
        // corresponding input value, template and formatted value. E.g., this is true when a new template is selected
        // and all labels are reset.
        void addInPathArgLabel(const NonIter &argTemplate, Rectangle<int> &remainingBox, const Font &fontInUse,
            const String &text = String(), bool automaticallyAddToOtherVectors = true);


        void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override;

        void buttonStateChanged(Button *btn) override {
            fadeCommandEnabled = btn->getToggleState();
        }

        void buttonClicked(Button *) override {
        };

        void labelTextChanged(Label *labelThatHasChanged) override;

        void setPathLabelErrorState(Label *lbl, bool error);
    private:
        int lastIndex = -1; // Used as dropdown tends to send unnecessary comboBoxChanged callbacks
        std::unique_ptr<XM32Template> currentTemplateCopy;
        TemplateCategory currentCategory;

        std::unordered_map<int, TemplateCategory> dDitemIDtoCategory; // Dropdown item ID to XM32 Template Category

        Image backgroundImage;

        std::vector<String> pathLabelInputValues; // Use for upon reconstructImage (as
        std::vector<NonIter> pathLabelInputTemplates;
        std::vector<std::unique_ptr<Label>> pathLabelInputs;
        ValueStorerArray pathLabelFormattedValues;

        std::unique_ptr<Fader> faderArgInput;

        ComboBox tpltCategoryDd;
        ComboBox tpltDd;
        ToggleButton enableFadeCommandBtn;
        bool fadeCommandEnabled = false;

        Rectangle<int> tpltSelectionBox;
        Rectangle<int> pathBox;
        Rectangle<int> argBox;
        Rectangle<int> fadeCmdBox;
        float fontSize;


        Rectangle<int> buttonsBox;

        Rectangle<int> tpltCategoryTitleBox;
        Rectangle<int> tpltCategoryDropBox;
        Rectangle<int> tpltSelectionTitleBox;
        Rectangle<int> tpltSelectionDropBox;

        Rectangle<int> fadeCmdTextBox;
        Rectangle<int> fadeCmdButtonBox;

        Rectangle<int> pathTitleBox;
        Rectangle<int> pathInputBox;

        Rectangle<int> argTitleBox;
        Rectangle<int> argInputArea;

        FontOptions font = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::bold);
        FontOptions monospace = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::bold);
    };

private:
    const std::string uuid;
    ParentWindowListener *registeredListener = nullptr;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCActionConstructor)
};
