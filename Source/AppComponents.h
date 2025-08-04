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


class GoToCueBtn: public Button {
public:
    GoToCueBtn(): Button("") {}
    void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {};
    void clicked() override { onClick(); };
    void clicked(const ModifierKeys &modifiers) override { onClick(); };
    void buttonStateChanged() override {};
    void paint(Graphics &g) override {};
    void paintOverChildren(Graphics &g) override {};
};


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

    // explicit Encoder(const NonIter& nonIter,
    //     double minDeg = -135.0,
    //     double maxDeg = 135.0,
    //     const String &minLabel = "",
    //     const String &maxLabel = ""
    //     );


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
        manualInputBox.setFont(inputBoxFont.withHeight(getHeight() * 0.125f * 0.7f));
        manualInputBox.addListener(this);
        manualInputBox.setJustification(Justification::centred);
        manualInputBox.setColour(TextEditor::ColourIds::backgroundColourId, UICfg::TRANSPARENT);
        manualInputBox.setColour(TextEditor::ColourIds::outlineColourId, UICfg::TRANSPARENT);
        manualInputBox.setColour(TextEditor::ColourIds::textColourId, UICfg::ROTARY_TEXT_COLOUR);
        encoder.addListener(this);
        addAndMakeVisible(encoder);
        addAndMakeVisible(manualInputBox);
        manualInputBox.setText(getValueAsDisplayString());
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

    void setValue(double value) {
        if (value < minValue || value > maxValue) {
            jassertfalse; // Value out of range
            return;
        }
        encoder.setValue(value);
    }
    void setNormalisedValue(double value) {
        if (value < 0 || value > 1) {
            jassertfalse; // Value out of range
            return;
        }
        encoder.setValue(inferValueFromMinMaxAndPercentage(minValue, maxValue, value, paramType));
    }
private:
    EncoderRotary encoder;

    std::vector<Listener*> encoderListeners;

    TextEditor manualInputBox{"manualInputBox"};
    Font inputBoxFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);

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

    void setValue(double value) {
        if (value < doubleMin || value > doubleMax) {
            jassertfalse; // Value out of range
            return;
        }
        slider->setValue(inferPercentageFromMinMaxAndValue(doubleMin, doubleMax, value, paramType));
    }
    void setNormalisedValue(double value) {
        if (value < 0.0 || value > 1.0) {
            jassertfalse; // Value out of range
            return;
        }
        slider->setValue(value);
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
struct DropdownWrapper: public ComboBox, public ComboBox::Listener {
    // Virtually identical to ComboBox::Listener.
    class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when a Dropdown has its selected item changed. */
        virtual void dropDownChanged (DropdownWrapper* dropDownThatHasChanged) = 0;
    };
    void init() {
        setColour(backgroundColourId, UICfg::TRANSPARENT);
        setColour(textColourId, UICfg::TEXT_COLOUR);
        setColour(outlineColourId, UICfg::BG_SECONDARY_COLOUR);
        setColour(focusedOutlineColourId, UICfg::TEXT_COLOUR);
        ComboBox::addListener(this);
    }
    explicit DropdownWrapper(const EnumParam& enumParam) {
        init();
        int i { 0 };
        for (const auto& str: enumParam.value) {
            i++;
            addItem(str,  i);
        }
        if (i != 0)
            setSelectedItemIndex(0, dontSendNotification);
    }

    explicit DropdownWrapper(const OptionParam& optionParam) {
        init();
        int i { 0 };
        for (const auto& str: optionParam.value) {
            i++;
            addItem(str,  i);
        }
        if (i != 0)
            setSelectedItemIndex(0, dontSendNotification);
    }

    void comboBoxChanged(ComboBox *) override {
        for (auto lstr: ddListeners) {
            if (lstr != nullptr) {
                lstr->dropDownChanged(this);
            }
        }
    };

    void addListener(Listener* lstr) {
        if (lstr != nullptr) {
            ddListeners.push_back(lstr);
        } else {
            jassertfalse; // Listener is null, this should never happen
        }
    }

    void removeListener(Listener* lstr) {
        ddListeners.erase(std::remove(ddListeners.begin(), ddListeners.end(), lstr), ddListeners.end());
    }

private:
    std::vector<Listener*> ddListeners;
};


struct TextEditorWrapper: public TextEditor, public TextEditor::Listener {
    // Virtually identical to TextEditor::Listener.
    class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when the user changes the text in some way. */
        virtual void wrappedTextEditorTextChanged (TextEditorWrapper*) {}

        /** Called when the user presses the return key. */
        virtual void wrappedTextEditorReturnKeyPressed (TextEditorWrapper*) {}

        /** Called when the user presses the escape key. */
        virtual void wrappedTextEditorEscapeKeyPressed (TextEditorWrapper*) {}

        /** Called when the text editor loses focus. */
        virtual void wrappedTextEditorFocusLost (TextEditorWrapper*) {}
    };

    explicit TextEditorWrapper(const NonIter& nonIter) {
        String showWhenEmpty;
        switch (nonIter._meta_PARAMTYPE) {
            case LINF:
            case LOGF:
            case LEVEL_1024:
            case LEVEL_161:
                showWhenEmpty = String(nonIter.floatMin) + " <-> " + String(nonIter.floatMax);
                break;
            case STRING:
                showWhenEmpty = String(nonIter.intMin) + " <-> " + String(nonIter.intMax) + " characters";
                break;
            case INT:
                showWhenEmpty = String(nonIter.intMin) + " <-> " + String(nonIter.intMax);
                break;
            default:
                jassertfalse; // Unsupported ParamType for TextEditorWrapper
                break;
        }
        setTextToShowWhenEmpty(showWhenEmpty, UICfg::TEXT_COLOUR_DARK);
        setText(nonIter.defaultStringValue);
        setColour(backgroundColourId, UICfg::TRANSPARENT);
        setColour(outlineColourId, UICfg::BG_SECONDARY_COLOUR);
        setColour(textColourId, UICfg::TEXT_COLOUR);
        setColour(focusedOutlineColourId, UICfg::TEXT_COLOUR);
        TextEditor::addListener(this);
        // 24 as a default. We don't actually know what's the best option.
        setFont(FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 24, Font::plain));
    }

    void addListener(Listener* lstr) {
        if (lstr != nullptr) {
            textEditorListeners.push_back(lstr);
        } else {
            jassertfalse; // Listener is null, this should never happen
        }
    }

    void removeListener(Listener* lstr) {
        textEditorListeners.erase(std::remove(textEditorListeners.begin(), textEditorListeners.end(), lstr), textEditorListeners.end());
    }

    void textEditorEscapeKeyPressed(TextEditor &) override {
        for (auto lstr: textEditorListeners) {
            if (lstr != nullptr) {
                lstr->wrappedTextEditorEscapeKeyPressed(this);
            }
        }
    }
    void textEditorFocusLost(TextEditor &) override {
        for (auto lstr: textEditorListeners) {
            if (lstr != nullptr) {
                lstr->wrappedTextEditorFocusLost(this);
            }
        }
    }
    void textEditorReturnKeyPressed(TextEditor &) override {
        for (auto lstr: textEditorListeners) {
            if (lstr != nullptr) {
                lstr->wrappedTextEditorReturnKeyPressed(this);
            }
        }
    }
    void textEditorTextChanged(TextEditor &) override {

        for (auto lstr: textEditorListeners) {
            if (lstr != nullptr) {
                lstr->wrappedTextEditorTextChanged(this);
            }
        }
    }
private:
    std::vector<Listener*> textEditorListeners;
};



struct ButtonArray: public Component {
};




class ParentWindowListener {
public:
    enum WindowType {
        AppComponents_OSCActionConstructor,
        AppComponents_OSCCCIConstructor,
    };

    virtual ~ParentWindowListener() = default;

    // Please don't pass std::string uuid as reference... this is setting ourselves up for SIGSEGVs!
    virtual void closeRequested(WindowType windowType, std::string uuid) = 0;
};


class OSCActionConstructor : public DocumentWindow {
public:
    // When provided an editThisAction, a valid XM32Template ID must be specified with the CueOSCAction.
    explicit OSCActionConstructor(const std::string &uuid, String name = "Cue OSC Action Constructor", const CueOSCAction& editThisAction = CueOSCAction(true)) : DocumentWindow(name,
        UICfg::BG_COLOUR,
        allButtons), uuid(uuid) {
        centreWithSize(1000, 800);
        setUsingNativeTitleBar(true);
        // setAlwaysOnTop(false);
        mainComponent = std::make_unique<MainComp>(editThisAction);
        setContentOwned(mainComponent.get(), true);
        setVisible(true);
    }

    void closeButtonPressed() override {
        if (mainComponent == nullptr) {
            return;
        }
        goodCueOSCAction = mainComponent->exitWasGraceful();

        if (registeredListener != nullptr) {
            registeredListener->closeRequested(ParentWindowListener::AppComponents_OSCActionConstructor, uuid);
        }
    }

    void setParentListener(ParentWindowListener *lstnr) {
        registeredListener = lstnr;
    }

    void removeParentListener() { registeredListener = nullptr; }

    // Returns the proper CueOSCAction from the user input. If invalid, returns an EXIT_THREAD CueOSCAction.
    CueOSCAction getCompiledCurrentCueAction() const {
        if (!goodCueOSCAction || mainComponent == nullptr) {
            return CueOSCAction(false);
        }
        return mainComponent->getCueOSCAction();
    }

    class MainComp : public Component, public ComboBox::Listener, public Button::Listener,
                    // public TextButton::Listener,
                     public Label::Listener, public Fader::Listener, public Encoder::Listener,
                    public TextEditorWrapper::Listener, public DropdownWrapper::Listener {
    public:
        enum InputMethod {
            FADER,
            ENCODER,
            DROPDOWN,
            TEXTBOX,
            BUTTON_ARRAY,
            NONE // Only used when second input is not required
        };

        MainComp(const CueOSCAction& editThisAction = CueOSCAction(true));

        ~MainComp() override { removeAllChildren(); }

        // All the component and children calls
        void uiInit();

        // Try find the XM32Template for the editThisAction CueOSCAction.
        void findEditThisActionsTemplate(const CueOSCAction& editThisAction);

        void resized() override;

        // Resizes the components used for inputs (i.e., faderInputs, encoderInputs, ddInputs, textInputs and
        // btnArrInputs).
        void resizeArgs(bool updateFirst = true, bool updateSecond = true);

        void paint(Graphics &g) override;

        void reconstructImage();


        // Change template dropdown based on new template category
        // If you need to select a template, you can pass the Template ID to try select it.
        void changeTpltDdBasedOnTpltCategory(const TemplateCategory category, const std::string& selectByTemplateID = "") {
            auto it = TEMPLATE_CATEGORY_MAP.find(category);
            if (it == TEMPLATE_CATEGORY_MAP.end()) {
                jassertfalse; // 🤦 how... how is this even possible?
                return;
            }
            const XM32TemplateGroup &tpltGroup = it->second;
            tpltDd.clear();
            int i = 0;
            for (const auto &tplt: tpltGroup.templates) {
                i++;
                tpltDd.addItem(tplt.NAME, i);
                if (selectByTemplateID == tplt.ID) {
                    tpltDd.setSelectedId(i, dontSendNotification);
                }
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

        // Used for non-argument inputs (i.e., static elements)
        void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override;

        // Called to update the component when a new template is selected. Expects ONLY currentTemplateCopy to be set.
        void uponNewTemplateSelected();

        // Called to update the component when a new input method is selected. Expects existing values to be set.
        void uponNewInputMethodSelected(bool updateFirst = true, bool updateSecond = true);

        // Called when a valuestorer object has changed without changing the child components.
        void uponValueStorerExternallyChanged(bool updateFirst = true, bool updateSecond = true);

        // Called to update the component when fading becomes enabled. Expects most items to be set. Usually called upon
        // the fade button being ticked and with uponNewTemplateSelected()
        void uponFadeCommandEnabledOrDisabled();

        // In our case, basically interchangeable with buttonClicked.
        void buttonStateChanged(Button *btn) override {}

        void faderValueChanged(Fader *) override;

        void encoderValueChanged(Encoder *) override;

        void buttonClicked(Button *) override;

        void wrappedTextEditorTextChanged(TextEditorWrapper* textEditor) override;

        // Use to listen for changes to a Dropdown input for values. NOT used for non-argument dropdowns (e.g.,
        // current template, current category, input method selector)
        void dropDownChanged(DropdownWrapper *dropDownThatHasChanged) override;

        static void setTextEditorErrorState(TextEditorWrapper *textEditor, bool error);

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

        // Uses firstInputMethod/secondInputMethod to reset child components used as input methods.
        // Must be called BEFORE setting a new ParamType to first/secondInputMethod
        void clearCurrentInputMethod(bool first = true, bool second = true);

        // Checks if the value and template is suitable for creating a valid CueOSCAction
        bool currentActionIsValid() const;

        // Returns a CueOSCAction. Expects currentActionIsValid to be true. Returns an EXIT_THREAD CueOSCAction if invalid
        CueOSCAction getCueOSCAction() const;

        bool exitWasGraceful() { return gracefulExit; }
    private:
        bool gracefulExit { false }; // Used to determine if an exit was made via pressing OK. If this is false, an
        // invalid CueOSCAction is returned regardless if the current the action is valid.

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
        String pathFromEditThisAction; // The path when editThisAction is provided. Bypasses checking, but is reset/overriden when a new template is selected or when all path label inputs are valid.
        mutable bool usePathFromEditThisAction = false; // This bool is set by currentActionIsValid(). ALL THE REASON MORE YOU MUST CALL IT BEFORE CALLING getCueOSCAction()!


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
        Label fadeTimeInput;
        float lastValidFadeTime { 0.f };
        bool fadeCommandEnabled = false;

        TextButton okBtn;
        TextButton cancelBtn;

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
        Rectangle<int> fadeTimeInputTitleBox;

        Rectangle<int> pathTitleBox;
        Rectangle<int> pathInputBox;

        Rectangle<int> argTitleBoxLeft;
        Rectangle<int> argInputAreaLeft;
        Rectangle<int> argTitleBoxRight;
        Rectangle<int> argInputAreaRight;


        FontOptions font = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::bold);
        FontOptions descFont = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::plain);
        FontOptions monospace = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::bold);
        FontOptions monospacePlain = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);


        // Allowed input methods for each paramtype. The first index in the vector is ALWAYS the preferred/default method.
        const std::unordered_map<ParamType, std::vector<InputMethod>> ALLOWED_INPUT_METHODS_FOR_TYPE = {
            {LEVEL_161, {FADER, ENCODER, TEXTBOX}},
            {LEVEL_1024, {FADER, ENCODER, TEXTBOX}},
            {LINF, {TEXTBOX, ENCODER, FADER}},
            {LOGF, {TEXTBOX, ENCODER, FADER}},
            {INT, {TEXTBOX}},
            {_GENERIC_FLOAT, {TEXTBOX, ENCODER, FADER}},
            {ENUM, {DROPDOWN, ENCODER}},
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


        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComp)
    };


private:
    const std::string uuid;
    ParentWindowListener *registeredListener = nullptr;
    std::unique_ptr<MainComp> mainComponent;
    bool goodCueOSCAction { false };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCActionConstructor)
};


class OSCCCIConstructor : public DocumentWindow {
public:
    class RowUpdateRequiredCallback {
        public:
        virtual ~RowUpdateRequiredCallback() {}
        virtual void rowRequiresUpdate(int rowNum) = 0;
    };

    // When provided an editThisCCI, each CurrentCueInfo must be specified with CueOSCActions carrying valid UUIDs.
    explicit OSCCCIConstructor(const std::string &uuid, String name = "Cue Constructor", const CurrentCueInfo& editThisCCI = CurrentCueInfo()) : DocumentWindow(name,
        UICfg::BG_COLOUR,
        allButtons), uuid(uuid) {
        centreWithSize(1000, 800);
        setUsingNativeTitleBar(true);
        // setAlwaysOnTop(false);
        mainComponent = std::make_unique<MainComp>(editThisCCI);
        setContentOwned(mainComponent.get(), true);
        setVisible(true);
    }

    void closeButtonPressed() override {
        if (mainComponent == nullptr) {
            return;
        }
        // Temporary for testing
        JUCEApplication::getInstance()->systemRequestedQuit();

    }

    void setParentListener(ParentWindowListener *lstnr) {
        registeredListener = lstnr;
    }

    void removeParentListener() { registeredListener = nullptr; }

    // Returns the proper CCI from the user input. If invalid, returns an invalid CCI.
    CurrentCueInfo getCompiledCurrentCueAction() const;


    class ActionListModel: public ListBoxModel, public ParentWindowListener {
    public:
        Image editIcon{getIconImageFile(IconID::EDIT)};

        RowUpdateRequiredCallback* callbackUponChildWindowExit;
        std::vector<std::unique_ptr<CueOSCAction>> actions;
        std::unordered_map<std::string, int> actionConstructorUUIDsToIndex;
        std::vector<std::unique_ptr<OSCActionConstructor>> actionConstructors;

        void updateSize() { lastSize = static_cast<int>(actions.size()); }

        int getNumRows() override { return lastSize; }

        void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override {
            Rectangle<int> bounds {0, 0, width, height};
            g.setColour(UICfg::TEXT_COLOUR_DARK);
            g.drawRect(bounds);
            g.setColour(UICfg::TEXT_COLOUR);
            auto widthTenths = width * 0.1f;
            auto editBtnBox = bounds.removeFromLeft(widthTenths * 0.7f);
            auto oatBox = bounds.removeFromLeft(widthTenths * 0.8f);
            auto pathBox = bounds.removeFromLeft(widthTenths * 3);
            auto valuesBox = bounds;


            g.setColour(UICfg::TEXT_COLOUR);
            // auto currentAction = &actions[rowNumber];
            if (actions[rowNumber] == nullptr) {
                return; // Invalid action
            }

            g.drawImage(editIcon, editBtnBox.reduced(editBtnBox.getWidth() * UICfg::STD_PADDING * 4).toFloat(), RectanglePlacement::centred);
            auto currentAction = *actions[rowNumber].get();
            g.setColour(UICfg::TEXT_COLOUR);
            g.setFont(textFont.withHeight(height * 0.5f));
            g.drawText(currentAction.oscAddress.toString(), pathBox, Justification::centredLeft);

            g.setFont(textFont.withHeight(height * 0.6f));
            switch (currentAction.oat) {
                case OAT_COMMAND: {
                    g.drawText("CMD", oatBox, Justification::centredLeft);
                    String valueText;
                    switch (currentAction.argument._meta_PARAMTYPE) {
                        case INT:
                            valueText = String(currentAction.argument.intValue) + " (i)";
                            break;
                        case STRING:
                            valueText = String(currentAction.argument.stringValue) + " (s)";
                            break;
                        case _GENERIC_FLOAT:
                            valueText = String(currentAction.argument.floatValue) + " (f)";
                            break;
                    }
                    g.drawText(valueText, valuesBox, Justification::centredLeft);
                    break;
                }
                case OAT_FADE: {
                    g.drawText("FADE", oatBox, Justification::centredLeft);
                    auto fadeTimeBox = valuesBox.removeFromLeft(widthTenths);
                    auto startValueBox = valuesBox.removeFromLeft(widthTenths * 2);
                    auto endValuesBox = valuesBox;
                    String startValueText;
                    String endValueText;
                    switch (currentAction.startValue._meta_PARAMTYPE) {
                        case INT:
                            startValueText = String(currentAction.startValue.intValue) + " (i) >>";
                            break;
                        case _GENERIC_FLOAT:
                            startValueText = String(currentAction.startValue.floatValue) + " (f) >>";
                            break;
                    }
                    switch (currentAction.endValue._meta_PARAMTYPE) {
                        case INT:
                            endValueText = String(currentAction.endValue.intValue) + " (i)";
                            break;
                        case _GENERIC_FLOAT:
                            endValueText = String(currentAction.endValue.floatValue) + " (f)";
                            break;
                    }
                    g.drawText(String(currentAction.fadeTime, 2)+"s", fadeTimeBox, Justification::centredLeft);
                    g.drawText(startValueText, startValueBox, Justification::centredLeft);
                    g.drawText(endValueText, endValuesBox, Justification::centredLeft);
                    break;
                }
                case EXIT_THREAD: {
                    g.drawText("EXIT", oatBox, Justification::centredLeft);
                    break;
                }
            }

        }

        Component* refreshComponentForRow(int rowNumber, bool /* isRowSelected */, Component *existingComponentToUpdate) override {
            // We need to an edit action button.
            if (rowNumber >= getNumRows()) {
                return nullptr; // Ignore. Not making a component for a row that doesn't exist.
            }
            auto* alibcw = static_cast<ActionListItemButtonComponentWrapper*>(existingComponentToUpdate);
            if (alibcw == nullptr) {
                alibcw = new ActionListItemButtonComponentWrapper(*this);
            }
            alibcw->setIndexOfAction(rowNumber);

            return alibcw;
        };

        void editAction(int indexInVector) {
            if (indexInVector >= getNumRows()) {
                jassertfalse; // Attempted to access action in invalid index
                return;
            }
            if (actionConstructors.size() != getNumRows()) {
                actionConstructors.resize(getNumRows());
            }
            // In case the size hasn't been correctly updated, double check index is valid.
            if (actionConstructors.size() <= indexInVector) {
                jassertfalse; // This likely means your vector size hasn't been updated properly. You need to call updateSize().
                return;
            }

            if (actionConstructors[indexInVector] != nullptr)
                return;
            if (actions[indexInVector] == nullptr)
                return;
            // Create new constructor window. Order of action vector MUST NOT CHANGE!
            auto uuid = uuidGen.generate();
            actionConstructors[indexInVector] = std::make_unique<OSCActionConstructor>(uuid, "Editing Cue Action",
                *actions[indexInVector].get());
            actionConstructorUUIDsToIndex[uuid] = indexInVector;
            actionConstructors[indexInVector]->setParentListener(this);
        }


        void closeRequested(WindowType windowType, std::string uuid) override {
            if (windowType != AppComponents_OSCActionConstructor) {
                jassertfalse; // Listener only accepts attempted closes from OSC Action Constructors.
                return;
            }
            auto it = actionConstructorUUIDsToIndex.find(uuid);
            if (it == actionConstructorUUIDsToIndex.end()) {
                jassertfalse; // UUID Not found
                return;
            }
            if (it->second >= getNumRows() || it->second >= actionConstructors.size()) {
                jassertfalse; // Somehow the index is out of range. You really need to lock in.
                return;
            }
            // Now if the CueOSCAction is valid, set the new index.
            auto &constructor = actionConstructors.at(it->second);
            if (constructor == nullptr) {
                jassertfalse; // Constructor is null. JUST HOW DID YOU MANAGE TO PULL THAT OFF
                return;
            }
            auto compiledCCA = constructor->getCompiledCurrentCueAction();
            if (compiledCCA.oat == EXIT_THREAD) {
                // Indicates invalid action. Return, do nothing, but still reset the window.
                actionConstructors[it->second].reset();
                if (callbackUponChildWindowExit != nullptr)
                    callbackUponChildWindowExit->rowRequiresUpdate(it->second);
                return;
            }
            switch (compiledCCA.oat) {
                case OAT_COMMAND: {
                    actions[it->second].reset(new CueOSCAction(compiledCCA.oscAddress,
                                                               compiledCCA.oatCommandOSCArgumentTemplate,
                                                               compiledCCA.argument, compiledCCA.argumentTemplateID));
                    break;
                }
                case OAT_FADE:{
                    actions[it->second].reset(new CueOSCAction(compiledCCA.oscAddress, compiledCCA.fadeTime,
                        compiledCCA.oscArgumentTemplate, compiledCCA.startValue, compiledCCA.endValue, compiledCCA.argumentTemplateID));
                    break;
                }
            }
            actionConstructors[it->second].reset();
            if (callbackUponChildWindowExit != nullptr)
                callbackUponChildWindowExit->rowRequiresUpdate(it->second);
        }

    private:
        Font textFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);
        int lastSize { 0 };
        UUIDGenerator uuidGen;
    };


    // Component to wrap the edit button for an Action List.
    struct ActionListItemButtonComponentWrapper: public Component {
    public:
        ActionListItemButtonComponentWrapper(ActionListModel& owner): owner(owner) {
            addAndMakeVisible(goToCueBtn);
        };
        void setIndexOfAction(size_t indx) {
            goToCueBtn.onClick = [=]() {
                owner.editAction(indx);
            };
        }
        void paint(Graphics &g) override {};
        void resized() override {
            goToCueBtn.setBounds(getLocalBounds().removeFromLeft(getWidth() * 0.1f));
        }

    private:
        GoToCueBtn goToCueBtn;
        ActionListModel& owner;
    };

    class MainComp : public Component, public RowUpdateRequiredCallback {
    public:
        MainComp(const CurrentCueInfo& editThisCCI = CurrentCueInfo()) {
            setSize(1000, 800);
            actionsListModel.callbackUponChildWindowExit = this;
            actionsListModel.actions.emplace_back(
                std::make_unique<CueOSCAction>(
                    "/ch/01/eq/1/type", 4.f, Channel::EQ_BAND_FREQ.NONITER, ValueStorer(20.f), ValueStorer(60.f), Channel::EQ_BAND_FREQ.ID));
            actionsListModel.updateSize();
            actionsList.updateContent();
            uiInit();
            addAndMakeVisible(idInput);
            addAndMakeVisible(nameInput);
            addAndMakeVisible(descInput);
            addAndMakeVisible(actionsList);
            // Required as for some reason, the texteditor input size is for some reason cooked upon init.
            resized();
        };

        ~MainComp() override { removeAllChildren(); }

        void rowRequiresUpdate(int rowNum) override { actionsList.repaintRow(rowNum); };

        // All the component and children calls
        void uiInit() {
            idInput.setColour(TextEditor::ColourIds::backgroundColourId, UICfg::TRANSPARENT);
            idInput.setColour(TextEditor::ColourIds::focusedOutlineColourId, UICfg::TEXT_ACCENTED_COLOUR);
            idInput.setColour(TextEditor::ColourIds::outlineColourId, UICfg::TEXT_COLOUR);
            idInput.setColour(TextEditor::ColourIds::textColourId, UICfg::TEXT_COLOUR);
            idInput.setJustification(Justification::topLeft);
            idInput.setTextToShowWhenEmpty("Enter Cue ID", UICfg::TEXT_COLOUR_DARK);
            idInput.setFont(inputFont);
            nameInput.setColour(TextEditor::ColourIds::backgroundColourId, UICfg::TRANSPARENT);
            nameInput.setColour(TextEditor::ColourIds::focusedOutlineColourId, UICfg::TEXT_ACCENTED_COLOUR);
            nameInput.setColour(TextEditor::ColourIds::outlineColourId, UICfg::TEXT_COLOUR);
            nameInput.setColour(TextEditor::ColourIds::textColourId, UICfg::TEXT_COLOUR);
            nameInput.setJustification(Justification::topLeft);
            nameInput.setTextToShowWhenEmpty("Enter Cue Name", UICfg::TEXT_COLOUR_DARK);
            nameInput.setFont(inputFont);
            descInput.setColour(TextEditor::ColourIds::backgroundColourId, UICfg::TRANSPARENT);
            descInput.setColour(TextEditor::ColourIds::focusedOutlineColourId, UICfg::TEXT_ACCENTED_COLOUR);
            descInput.setColour(TextEditor::ColourIds::outlineColourId, UICfg::TEXT_COLOUR);
            descInput.setColour(TextEditor::ColourIds::textColourId, UICfg::TEXT_COLOUR);
            descInput.setTextToShowWhenEmpty("Enter Cue Description", UICfg::TEXT_COLOUR_DARK);
            descInput.setJustification(Justification::topLeft);
            descInput.setFont(inputFont);

            actionsList.setColour(ListBox::ColourIds::backgroundColourId, UICfg::TRANSPARENT);
            actionsList.setColour(ListBox::ColourIds::outlineColourId, UICfg::TRANSPARENT);
            actionsList.setColour(ListBox::ColourIds::textColourId, UICfg::TEXT_COLOUR);
        };

        void resized() override {
            auto bounds = getLocalBounds();
            bounds = bounds.reduced(std::min(getWidth(), getHeight()) * UICfg::STD_PADDING);

            auto heightTenths = bounds.getHeight() * 0.1f;
            auto padding = std::min(bounds.getHeight(), bounds.getWidth()) * UICfg::STD_PADDING;
            auto topBounds = bounds.removeFromTop(heightTenths * 1.2f);
            bounds.removeFromTop(padding);
            auto descBounds = bounds.removeFromTop(heightTenths * 1.5f);
            bounds.removeFromTop(padding);
            auto actionBounds = bounds;

            // We'll use this height across top, center and bottom boxes for consistency for heights
            auto topBoxHeightTenths = topBounds.getHeight() * 0.1f;

            // Deal with ID and Name
            auto idBounds = topBounds.removeFromLeft(bounds.getWidth() * 0.2f);
            // auto idBoundsHeightTenths = idBounds.getHeight() * 0.1f;
            idTitle = idBounds.removeFromTop(topBoxHeightTenths * 4);
            idBounds.removeFromTop(topBoxHeightTenths);
            idInputBounds = idBounds;
            idInput.setBounds(idInputBounds);
            auto fontSize = idInputBounds.getHeight() * 0.5f;
            idInput.setFont(inputFont.withHeight(fontSize));
            // .setText checks that the newText != self.getText().
            // So first set the TextEditor to a blank string, then to the previous string.
            String idInputStr = idInput.getText();
            idInput.setText("");
            idInput.setText(idInputStr); // Looks really stupid, but it's required to resize text to new font size


            topBounds.removeFromLeft(padding);
            auto nameBounds = topBounds;
            // auto nameBoundsHeightTenths = nameBounds.getHeight() * 0.1f;
            nameTitle = nameBounds.removeFromTop(topBoxHeightTenths * 4);
            nameBounds.removeFromTop(topBoxHeightTenths);
            nameInputBounds = nameBounds;
            nameInput.setBounds(nameInputBounds);
            nameInput.setFont(inputFont.withHeight(fontSize));
            String nameInputStr = nameInput.getText();
            nameInput.setText("");
            nameInput.setText(nameInputStr);

            // Now deal with description. For consistency, we'll use the topBoxHeightTenths to keep padding and font
            // size consistent
            // auto descBoundsHeightTenths = descBounds.getHeight() * 0.1f;
            descTitle = descBounds.removeFromTop(topBoxHeightTenths * 4);
            descBounds.removeFromTop(topBoxHeightTenths);
            descInputBounds = descBounds;
            descInput.setBounds(descInputBounds);
            descInput.setFont(inputFont.withHeight(fontSize * 0.8f));
            String descInputStr = descInput.getText();
            descInput.setText("");
            descInput.setText(descInputStr);

            // Do similarly for action list.
            actionsTitle = actionBounds.removeFromTop(topBoxHeightTenths * 4);
            actionBounds.removeFromTop(topBoxHeightTenths);
            actionsHeader = actionBounds.removeFromTop(topBoxHeightTenths * 5);
            actionsListBounds = actionBounds;
            actionsList.setBounds(actionsListBounds);
            actionsList.setRowHeight(topBoxHeightTenths * 5);

            reconstructImage();
        };

        void paint(Graphics &g) override {
            g.drawImage(bgImage, getLocalBounds().toFloat());
        }

        void reconstructImage() {
            bgImage = Image(Image::ARGB, getWidth(), getHeight(), true);
            Graphics g(bgImage);

            g.setFont(titleFont.withHeight(idTitle.getHeight() * 0.7f));
            g.setColour(UICfg::TEXT_COLOUR);
            g.drawText("ID", idTitle, Justification::centredLeft);
            g.drawText("Name", nameTitle, Justification::centredLeft);
            g.drawText("Description", descTitle, Justification::centredLeft);
            g.drawText("Actions", actionsTitle, Justification::centredLeft);

            auto widthTenths = actionsHeader.getWidth() * 0.1f;
            auto bounds = actionsHeader;
            auto editBtnBox = bounds.removeFromLeft(widthTenths * 0.7f);
            auto oatBox = bounds.removeFromLeft(widthTenths * 0.8f);
            auto pathBox = bounds.removeFromLeft(widthTenths * 3);
            auto valuesBox = bounds;
            g.setColour(UICfg::TEXT_COLOUR_DARK);
            g.setFont(inputFont.withHeight(editBtnBox.getHeight() * 0.5f).boldened());
            g.drawText("Edit", editBtnBox, Justification::centredLeft);
            g.drawText("Type", oatBox, Justification::centredLeft);
            g.drawText("Address", pathBox, Justification::centredLeft);
            g.drawText("Arguments", valuesBox, Justification::centredLeft);
        };

        // Checks if values and child CueOSCActions are suitable for creating a valid CCI
        bool currentCCIIsValid() const;

        // Returns a CCI. Expects currentCCIIsValid to be true. Returns an invalid CCI if invalid
        CurrentCueInfo getCCI() const;

        bool exitWasGraceful() const { return gracefulExit; }
    private:
        bool gracefulExit { false }; // Used to determine if an exit was made via pressing OK. If this is false, an
        // invalid CueOSCAction is returned regardless if the current the action is valid.

        Rectangle<int> idTitle;
        Rectangle<int> idInputBounds;
        Rectangle<int> nameTitle;
        Rectangle<int> nameInputBounds;
        Rectangle<int> descTitle;
        Rectangle<int> descInputBounds;
        Rectangle<int> actionsTitle;
        Rectangle<int> actionsHeader;
        Rectangle<int> actionsListBounds;


        TextEditor idInput;
        TextEditor nameInput;
        TextEditor descInput;
        ActionListModel actionsListModel;
        ListBox actionsList {"", &actionsListModel};

        Image bgImage;

        Font titleFont = FontOptions(UICfg::DEFAULT_SANS_SERIF_FONT_NAME, 1.f, Font::bold);
        Font inputFont = FontOptions(UICfg::DEFAULT_MONOSPACE_FONT_NAME, 1.f, Font::plain);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComp)
    };


private:
    const std::string uuid;
    ParentWindowListener *registeredListener = nullptr;
    std::unique_ptr<MainComp> mainComponent;
    bool goodCCI { false };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCCCIConstructor)
};
