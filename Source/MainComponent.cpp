#include "MainComponent.h"

//==============================================================================


MainComponent::MainComponent() {
    setSize(600, 400);


    testRotary.setBounds(100, 300, 180, 180);
    testRotary2.setBounds(300, 300, 180, 180);
    testRotary3.setBounds(500, 300, 180, 180);
    testRotary4.setBounds(700, 300, 180, 180);
    testRotary5.setBounds(100, 500, 180, 180);

    for (auto *comp: getComponents()) {
        addAndMakeVisible(*comp);
    }
}

MainComponent::~MainComponent() {
    // for (auto *comp : getComponents())
    // {
    // removeAllChildren();
    // }
    removeAllChildren();
    for (auto *comp: activeComps) {
        if (comp != nullptr)
            comp->setLookAndFeel(nullptr);
    }
    delete this;
}


//==============================================================================
void MainComponent::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(juce::FontOptions(16.0f));
    g.setColour(juce::Colours::white);
    g.drawText("Kewei's bad!", getLocalBounds(), juce::Justification::centred, true);
}


void MainComponent::resized() {
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    auto bounds = getLocalBounds();
}


void HeaderBar::resized() {
    auto bounds = getLocalBounds();
    // The bounds passed should be the bounds of ONLY the intended header bar.
    auto boundWidthTenths = bounds.getWidth() / 10;
    showNameBox = bounds.removeFromLeft(boundWidthTenths * 4);
    cueIDBox = bounds.removeFromLeft(boundWidthTenths * 2);
    cueNoBox = bounds.removeFromLeft(boundWidthTenths * 2);
    stopBox = bounds.removeFromLeft(boundWidthTenths * 2);
    downBox = bounds.removeFromLeft(boundWidthTenths);
    upBox = bounds.removeFromLeft(boundWidthTenths);
    playBox = bounds.removeFromLeft(boundWidthTenths);
    timeBox = bounds.removeFromLeft(boundWidthTenths * 2);

    // Each box required a border. Let's draw it to an image.
    Graphics g(borderImage);
    borderImage = Image(Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    g.setColour(UICfg::STRONG_BORDER_COLOUR);
    g.drawRect(showNameBox, 2);
    g.drawRect(cueIDBox, 2);
    g.drawRect(cueNoBox, 2);
    g.drawRect(stopBox, 2);
    g.drawRect(downBox, 2);
    g.drawRect(upBox, 2);
    g.drawRect(playBox, 2);
    g.drawRect(timeBox, 2);
}


void HeaderBar::paint(Graphics &g) {

}
