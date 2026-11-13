#pragma once
#include "PluginProcessor.h"
class DiamondLook : public juce::LookAndFeel_V4 {
public:
    DiamondLook();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class DiamondIIAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit DiamondIIAudioProcessorEditor(DiamondIIAudioProcessor&);
    ~DiamondIIAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    DiamondIIAudioProcessor& processor;
    DiamondLook look;
    juce::Component panel;
    std::array<juce::Slider,3> knobs;
    juce::ComboBox brightness,type,zoom;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,3> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ba,ta;
};
