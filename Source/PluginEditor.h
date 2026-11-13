#pragma once
#include "PluginProcessor.h"
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
private:
    DiamondIIAudioProcessor& processor;
    DiamondLook look;
    juce::Component panel;
    std::array<juce::Slider,5> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,5> attachments;
};
