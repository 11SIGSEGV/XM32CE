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
// TODO: Test if NonIter overload functions
// Acts as a wrapper for Slider. Overwrites painting and resizing.
struct EncoderRotary : public Slider {
    explicit EncoderRotary(Units unit,
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

    // We'll also allow NonIter, Enum and OptionParams
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

    // explicit EncoderRotary(const NonIter &nonIter, double minDeg = -135.0,
    // double maxDeg = 135.0, const String &minLabel = "", const String &maxLabel = "");

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
    // Virtually identical to Slider::Listener, but returns the Encoder object changes instead of the internal slider
    // used by the Encoder.
    class Listener {
    public:
        //==============================================================================
        /** Destructor. */
        virtual ~Listener() = default;

        //==============================================================================
        // Called when the encoder's value is changed.
        virtual void encoderValueChanged (Encoder*) = 0;

        //==============================================================================
        // Called when the encoder is about to be dragged.
        virtual void encoderDragStarted (Encoder*) {}

        // Called after a drag operation has finished.
        virtual void encoderDragEnded (Encoder*) {}
    };


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

    explicit Encoder(const NonIter& nonIter,
        double minDeg = -135.0,
        double maxDeg = 135.0,
        const String &minLabel = "",
        const String &maxLabel = ""
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
    }

    double getValue() { return encoder.getValue(); }



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
        // Pass onto listeners
        for (auto lstr: encoderListeners) {
            if (lstr != nullptr) {
                lstr->encoderValueChanged(this);
            }
        }
    }

    void textEditorReturnKeyPressed(TextEditor &editor) override {
        // Do checks to see if the value is valid
        auto [valid, value] = getDoubleValueFromTextEditor(editor.getText());
        if (valid) {
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
            encoder.setValue(roundTo(value, roundToNDigits));
        } else {
            // If not valid, show an error message
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                             "Invalid Input",
                                             "Please enter a valid value.");
        }
    }

    // void sliderDragStarted(Slider *) override {};
    // void sliderDragEnded(Slider *) override {};


    void paint(Graphics &g) override {}

    void resized() override;

    void sliderDragStarted(Slider *) override {
        // We don't need it internally, but pass onto listeners
        for (auto lstr: encoderListeners) {
            if (lstr != nullptr) {
                lstr->encoderDragStarted(this);
            }
        }
    }


    void sliderDragEnded(Slider *) override {
        // We don't need it internally, but pass onto listeners
        for (auto lstr: encoderListeners) {
            if (lstr != nullptr) {
                lstr->encoderDragEnded(this);
            }
        }
    }


    void addListener(Listener* lstr) {
        if (lstr != nullptr) {
            encoderListeners.push_back(lstr);
        } else {
            jassertfalse; // Listener is null, this should never happen
        }
    }

    void removeListener(Listener* lstr) {
        encoderListeners.erase(std::remove(encoderListeners.begin(), encoderListeners.end(), lstr), encoderListeners.end());
    }
private:
    EncoderRotary encoder;

    std::vector<Listener*> encoderListeners;

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
    const int roundToNDigits;
    const ParamType paramType;

    const std::set<ParamType> ALLOWED_PARAMTYPES = {LINF, LOGF, INT, LEVEL_161, LEVEL_1024};
};


struct Fader : public Component, public Slider::Listener {
    // Virtually identical to Slider::Listener, but returns the Fader object changes instead of the internal slider used
    // by the Fader.
    class Listener {
    public:
        //==============================================================================
        /** Destructor. */
        virtual ~Listener() = default;

        //==============================================================================
        // Called when the fader's value is changed.
        virtual void faderValueChanged (Fader*) = 0;

        //==============================================================================
        // Called when the fader is about to be dragged.
        virtual void faderDragStarted (Fader*) {}

        // Called after a drag operation has finished.
        virtual void faderDragEnded (Fader*) {}
    };


    struct FaderSlider : public Slider {
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
            setColour(textBoxOutlineColourId, UICfg::TRANSPARENT);
        }

        void paint(Graphics &g) override {
        };
    };

    // A bound width:height of 3:8 is recommended (e.g., 150 width, 400 height).
    Fader(const NonIter &nonIter): doubleMin(nonIter.floatMin), doubleMax(nonIter.floatMax),
                                   paramType(nonIter._meta_PARAMTYPE),
                                   argUnit(nonIter._meta_UNIT) {
        // if (nonIter._meta_PARAMTYPE != LEVEL_1024 && nonIter._meta_PARAMTYPE != LEVEL_161) {
        // jassertfalse; // Unsupported ParamType
        // }
        slider = std::make_unique<FaderSlider>();
        slider->setDoubleClickReturnValue(true,
                                          inferPercentageFromMinMaxAndValue(
                                              doubleMin, doubleMax, nonIter.defaultFloatValue, paramType));
        addAndMakeVisible(*slider);
        slider->addListener(this);
    }

    void resized() override;


    void paint(Graphics &g) override;


    void sliderValueChanged(Slider *sldr) override {
        repaint();
        for (auto lstr: faderListeners) {
            if (lstr != nullptr) {
                lstr->faderValueChanged(this);
            }
        }
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


    void sliderDragStarted(Slider *sldr) override {
        // Simply broadcast to listeners. We don't need it in our internal struct.
        for (auto lstr: faderListeners) {
            if (lstr != nullptr) {
                lstr->faderDragStarted(this);
            }
        }
    }

    void sliderDragEnded(Slider *sldr) override {
        // Simply broadcast to listeners. We don't need it in our internal struct.
        for (auto lstr: faderListeners) {
            if (lstr != nullptr) {
                lstr->faderDragEnded(this);
            }
        }
    }

    void addListener(Listener* lstr) {
        if (lstr != nullptr) {
            faderListeners.push_back(lstr);
        } else {
            jassertfalse; // Listener is null, this should never happen
        }
    }

    void removeListener(Listener* lstr) {
        faderListeners.erase(std::remove(faderListeners.begin(), faderListeners.end(), lstr), faderListeners.end());
    }
private:
    std::unique_ptr<FaderSlider> slider;
    std::vector<Listener*> faderListeners;
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


// Literally just a wrapper to create a temporary ComboBox using a EnumParam or Option.
// A gentle reminder that the ID of each ComboBox element is simply the index + 1.
// This is literally just here to make initalising the ComboBox easier. You still have to handle everything else.
struct DropdownWrapper: public ComboBox {
    DropdownWrapper(const EnumParam& enumParam) {
        int i { 0 };
        for (auto str: enumParam.value) {
            i++;
            addItem(str,  i);
        }
    }

    DropdownWrapper(const OptionParam& optionParam) {
        int i { 0 };
        for (auto str: optionParam.value) {
            i++;
            addItem(str,  i);
        }
    }
};


struct TextEditorWrapper: public TextEditor {
    TextEditorWrapper(const NonIter& nonIter) {
        setTextToShowWhenEmpty(nonIter.verboseName, UICfg::TEXT_COLOUR_DARK);
        setText(nonIter.defaultStringValue);
    }
};



struct ButtonArray: public Component {
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

    void closeButtonPressed() override {
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
                     public Label::Listener, public Fader::Listener, public Encoder::Listener,
                    public TextEditor::Listener {
    public:
        enum InputMethod {
            FADER,
            ENCODER,
            DROPDOWN,
            TEXTBOX,
            BUTTON_ARRAY,
            NONE // Only used when second input is not required
        };

        MainComp();

        ~MainComp() override {}

        void resized() override;

        // Resizes the components used for inputs (i.e., faderInputs, encoderInputs, ddInputs, textInputs and
        // btnArrInputs).
        void resizeArgs();

        void paint(Graphics &g) override;

        void reconstructImage();

        static bool validateTextInput(float input, const NonIter &argTemplate);

        static bool validateTextInput(int input, const NonIter &argTemplate);

        static bool validateTextInput(const String &input, const NonIter &argTemplate);

        // Change template dropdown based on new template category
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

        // TODO: allow listener to catch dropdown for argument
        void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override;

        // Called to update the component when a new template is selected. Expects ONLY currentTemplateCopy to be set.
        void uponNewTemplateSelected();

        // Called to update the component when a new input method is selected. Expects existing values to be set.
        void uponNewInputMethodSelected();


        // Called to update the component when fading becomes enabled. Expects most items to be set. Usually called upon
        // the fade button being ticked and with uponNewTemplateSelected()
        void uponFadeCommandEnabledOrDisabled();


        void buttonStateChanged(Button *btn) override {
            fadeCommandEnabled = btn->getToggleState();
        }

        void faderValueChanged(Fader *) override;

        void encoderValueChanged(Encoder *) override;

        void buttonClicked(Button *) override;

        void textEditorTextChanged(TextEditor &) override;

        void labelTextChanged(Label *labelThatHasChanged) override;

        static void setPathLabelErrorState(Label *lbl, bool error);

        template <typename T>
        static void clearInputPair(std::pair<std::unique_ptr<T>, std::unique_ptr<T>>& pair) {
            if (pair.first != nullptr) {
                pair.first.reset();
            }
            if (pair.second != nullptr) {
                pair.second.reset();
            }
        }

    private:
        int indexOfLastTemplateSelected = -1; // Used as dropdown tends to send unnecessary comboBoxChanged callbacks
        std::unique_ptr<XM32Template> currentTemplateCopy;
        ParamType currentParamType {_BLANK};
        TemplateCategory currentCategory;

        std::unordered_map<int, TemplateCategory> dDitemIDtoCategory; // Dropdown item ID to XM32 Template Category

        Image backgroundImage;

        std::vector<String> pathLabelInputValues; // Use for upon reconstructImage (raw values)
        std::vector<NonIter> pathLabelInputTemplates;
        std::vector<std::unique_ptr<Label>> pathLabelInputs;
        ValueStorerArray pathLabelFormattedValues; // But this... is only used for when the final CueOSCAction object has to be returned by this component


        // In the case of a fadeCommand, the first element will be the start value, vice versa
        // Otherwise, the first element is always implied to be used.
        InputMethod firstInputMethod { NONE };
        InputMethod secondInputMethod { NONE };
        std::pair<std::unique_ptr<Fader>, std::unique_ptr<Fader>> faderInputs;
        std::pair<std::unique_ptr<Encoder>, std::unique_ptr<Encoder>> encoderInputs;
        std::pair<std::unique_ptr<DropdownWrapper>, std::unique_ptr<DropdownWrapper>> ddInputs;
        std::pair<std::unique_ptr<TextEditorWrapper>, std::unique_ptr<TextEditorWrapper>> textInputs;
        std::pair<std::unique_ptr<ButtonArray>, std::unique_ptr<ButtonArray>> btnArrInputs;


        std::pair<ValueStorer, ValueStorer> inputValues;


        ComboBox tpltCategoryDd;
        ComboBox tpltDd;

        // Used to select the input method. Use firstInputMethodDd when only one input is required.
        ComboBox firstInputMethodDd;
        ComboBox secondInputMethodDd; // Set this component to disabled when not in use.

        ToggleButton enableFadeCommandBtn;
        bool fadeCommandEnabled = false;

        Rectangle<int> tpltSelectionBox;
        Rectangle<int> pathBox;
        Rectangle<int> argBox;
        Rectangle<int> fadeCmdBox;
        Rectangle<int> descBox;
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

        Rectangle<int> argTitleBoxLeft;
        Rectangle<int> argInputAreaLeft;
        Rectangle<int> argTitleBoxRight;
        Rectangle<int> argInputAreaRight;


        FontOptions font = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::bold);
        FontOptions descFont = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::plain);
        FontOptions monospace = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::bold);

        // Allowed input methods for each paramtype. The first index in the vector is ALWAYS the preferred/default method.
        const std::unordered_map<ParamType, std::vector<InputMethod>> ALLOWED_INPUT_METHODS_FOR_TYPE = {
            {LEVEL_161, {FADER, ENCODER, TEXTBOX}},
            {LEVEL_1024, {FADER, ENCODER, TEXTBOX}},
            {LINF, {TEXTBOX, ENCODER, FADER}},
            {LOGF, {TEXTBOX, ENCODER, FADER}},
            {INT, {TEXTBOX}},
            {_GENERIC_FLOAT, {TEXTBOX, ENCODER, FADER}},
            {ENUM, {ENCODER, DROPDOWN}},
            {OPTION, {DROPDOWN}},
            {BITSET, {BUTTON_ARRAY}},
            {STRING, {TEXTBOX}},
            {_BLANK, {}}
        };
        const std::unordered_map<InputMethod, String> INPUT_METHOD_NAME = {
            {FADER, "Fader"},
            {ENCODER, "Encoder"},
            {TEXTBOX, "Textbox"},
            {DROPDOWN, "Dropdown"},
            {BUTTON_ARRAY, "Button Array"},
            {NONE, "None"}
        };
    };

private:
    const std::string uuid;
    ParentWindowListener *registeredListener = nullptr;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCActionConstructor)
};
