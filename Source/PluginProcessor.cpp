#include "PluginProcessor.h"
#include "PluginEditor.h"
static juce::AudioProcessorValueTreeState::ParameterLayout layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"input",1},"Input",juce::NormalisableRange<float>{-10,10,.01f},0.f));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"process",1},"Process",juce::NormalisableRange<float>{0,100,.01f},0.f));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output",1},"Output",juce::NormalisableRange<float>{-6,6,.01f},0.f));
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"brightness",1},"Brightness",juce::StringArray{"Opal","Gold","Sapphire"},1));
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"type",1},"Type",juce::StringArray{"Luminscnt","Iridescnt","Radiant","Luster","DarkEssnc"},0));
    return l;
}
DiamondIIAudioProcessor::DiamondIIAudioProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"DiamondII",layout()) {
    int i=0; for(auto id:{"input","process","output","brightness","type"}) values[i++]=state.getRawParameterValue(id);
}
void DiamondIIAudioProcessor::prepareToPlay(double fs,int) {
    engine.prepare(fs); inputGain.reset(fs,.015); outputGain.reset(fs,.015); amount.reset(fs,.015);
    inputGain.setCurrentAndTargetValue(std::pow(10.,values[0]->load()/20.)); outputGain.setCurrentAndTargetValue(std::pow(10.,values[2]->load()/20.)); amount.setCurrentAndTargetValue(values[1]->load()*.01f);
}
bool DiamondIIAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const { return l.getMainInputChannelSet()==l.getMainOutputChannelSet() && (l.getMainOutputChannelSet()==juce::AudioChannelSet::mono() || l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()); }
void DiamondIIAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
    juce::ScopedNoDenormals guard;
    const float in=values[0]->load(),p=values[1]->load()*.01f,out=values[2]->load();
    engine.setBrightness(juce::roundToInt(values[3]->load())); const int t=juce::roundToInt(values[4]->load());
    inputGain.setTargetValue(std::pow(10.,in/20.)); outputGain.setTargetValue(std::pow(10.,out/20.)); amount.setTargetValue(p);
    const bool unity=in==0 && p==0 && out==0;
    for(int n=0;n<b.getNumSamples();++n) {
        const auto ig=inputGain.getNextValue(),og=outputGain.getNextValue(); const float mix=amount.getNextValue();
        for(int c=0;c<b.getNumChannels();++c) b.setSample(c,n,engine.tickGained(b.getSample(c,n),c,ig,mix,og,t,unity));
    }
}
juce::AudioProcessorEditor* DiamondIIAudioProcessor::createEditor() { return new DiamondIIAudioProcessorEditor(*this); }
void DiamondIIAudioProcessor::getStateInformation(juce::MemoryBlock& b) {
    auto s=state.copyState(); s.setProperty("width",editorWidth.load(),nullptr); s.setProperty("height",editorHeight.load(),nullptr); copyXmlToBinary(*s.createXml(),b);
}
void DiamondIIAudioProcessor::setStateInformation(const void* d,int n) {
    auto xml=getXmlFromBinary(d,n); if(!xml || !xml->hasTagName("DiamondII")) return;
    auto s=juce::ValueTree::fromXml(*xml); state.replaceState(s);
    editorWidth=juce::jlimit(751,2504,(int)s.getProperty("width",1252)); editorHeight=juce::roundToInt(editorWidth.load()*380./1252.);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DiamondIIAudioProcessor; }
