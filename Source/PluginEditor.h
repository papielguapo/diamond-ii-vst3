#pragma once
#include "PluginProcessor.h"
class TypeRowButton : public juce::Button {
public:
    TypeRowButton():Button("Type") {}
    void paintButton(juce::Graphics& g,bool hover,bool down) override {
        if(hover || down) { g.setColour(juce::Colour(0xff7719ae).withAlpha(down?.23f:.12f)); g.fillRoundedRectangle(getLocalBounds().toFloat(),2); }
    }
};
class DiamondLook : public juce::LookAndFeel_V4 {
public:
    DiamondLook();
    juce::Image skin;
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    juce::Slider::SliderLayout getSliderLayout(juce::Slider&) override;
    juce::Font getLabelFont(juce::Label&) override { return juce::Font(juce::FontOptions(25.f,juce::Font::bold)); }
};
class DiamondIIAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit DiamondIIAudioProcessorEditor(DiamondIIAudioProcessor&);
    ~DiamondIIAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    static float typePointerAngle(int index) { return std::atan2(61.5f,195.5f-(134.f+26.f*juce::jlimit(0,4,index))); }
private:
    DiamondIIAudioProcessor& processor;
    DiamondLook look;
    juce::Component panel;
    std::array<juce::Slider,5> knobs;
    std::array<TypeRowButton,5> typeRows;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,5> attachments;
};
