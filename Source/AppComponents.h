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
        return formatValueUsingUnit(unit, value);
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
    Fader(const NonIter& nonIter): doubleMin(nonIter.floatMin), doubleMax(nonIter.floatMax), paramType(nonIter._meta_PARAMTYPE),
    argUnit(nonIter._meta_UNIT) {
        // if (nonIter._meta_PARAMTYPE != LEVEL_1024 && nonIter._meta_PARAMTYPE != LEVEL_161) {
            // jassertfalse; // Unsupported ParamType
        // }
        slider = std::make_unique<FaderSlider>();
        slider->setDoubleClickReturnValue(true,
            inferPercentageFromMinMaxAndValue(doubleMin, doubleMax, nonIter.defaultFloatValue, paramType));
        addAndMakeVisible(*slider);
        slider->addListener(this);
    }

    void resized() override;


    void paint(Graphics &g) override;

    void sliderValueChanged(Slider *sldr) override {
        repaint();
    }

    // getValue always returns the NON-normalised value
    [[nodiscard]] double getValue() const {
        return inferValueFromMinMaxAndPercentage(doubleMin, doubleMax, slider->getValue(), paramType);
    }

    [[nodiscard]] double getNormalisedValue() const {
        return slider->getValue();
    }

    [[nodiscard]] String getFormattedStringVal() const {
        return formatValueUsingUnit(argUnit, getValue());
    }
private:
    std::unique_ptr<FaderSlider> slider;

    float faderHeight;
    float boundsHeightWithoutFaderOverflow;
    int faderTopLeftX;
    Image bgImage;
    Image faderKnobImage;
    Font monospaceFontWithFontSize = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);
    const ParamType paramType;
    const Units argUnit;
    // To avoid casting from float->double for each getValue() call.
    const double doubleMin;
    const double doubleMax;

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

    const std::unordered_set<ParamType> ALLOWED_PARAMTYPES = {LEVEL_161, LEVEL_1024, LINF, LOGF};
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
