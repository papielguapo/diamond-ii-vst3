#include "../Source/PluginEditor.h"
#include <bit>
#include <iostream>
#include <random>
#include <stdexcept>
int checks=0;
void require(bool ok,const char* what) { ++checks; if(!ok) throw std::runtime_error(what); }
void set(DiamondIIAudioProcessor& p,const char* id,float v) { auto* a=p.state.getParameter(id); a->setValueNotifyingHost(a->convertTo0to1(v)); }
int main() { try {
    juce::ScopedJuceInitialiser_GUI gui;
    std::mt19937 rng(2026); std::uniform_real_distribution<float> noise(-1,1);
    for(double fs:{44100.,48000.,88200.,96000.,192000.}) {
        for(int t=0;t<5;++t) for(int brightness=0;brightness<3;++brightness) {
            DiamondIIAudioProcessor p; p.prepareToPlay(fs,257); set(p,"type",(float)t); set(p,"brightness",(float)brightness);
            juce::AudioBuffer<float> b(2,257),original(2,257); juce::MidiBuffer midi;
            for(int block=0;block<8;++block) {
                for(int c=0;c<2;++c) for(int n=0;n<257;++n) b.setSample(c,n,noise(rng));
                b.setSample(0,0,-0.f); b.setSample(1,0,0.f); original.makeCopyOf(b);
                p.processBlock(b,midi);
                for(int c=0;c<2;++c) for(int n=0;n<257;++n) require(std::bit_cast<uint32_t>(b.getSample(c,n))==std::bit_cast<uint32_t>(original.getSample(c,n)),"bit-accurate null");
            }
            set(p,"process",100); set(p,"input",18); set(p,"output",18);
            p.prepareToPlay(fs,257);
            double energy=0;
            for(int block=0;block<32;++block) {
                for(int n=0;n<257;++n) { b.setSample(0,n,noise(rng)); b.setSample(1,n,0); }
                p.processBlock(b,midi);
                for(int n=0;n<257;++n) { auto x=b.getSample(0,n); require(std::isfinite(x)&&std::abs(x)<512,"DSP finite/bounded"); require(b.getSample(1,n)==0,"stereo isolation"); energy+=x*x; }
            }
            require(energy>0,"active signal");
            p.prepareToPlay(fs,257); b.clear(); p.processBlock(b,midi);
            for(int c=0;c<2;++c) for(int n=0;n<257;++n) require(b.getSample(c,n)==0,"reset silence");
        }
        std::cout<<"PASS: "<<fs<<" Hz, all 5 types x 3 brightness, bitwise null and stress\n";
    }
    require(std::abs(diamond::Engine::pade32(1e-7)/1e-7-1)<1e-10,"Pade32 unit slope");
    require(std::abs(diamond::Engine::pade54(1e-7)/1e-7-1)<1e-10,"Pade54 unit slope");
    // Unity must not depend on amplitude, subnormals or the sign of zero.
    {
        DiamondIIAudioProcessor exact; exact.prepareToPlay(48000,512);
        juce::AudioBuffer<float> b(2,512),ref(2,512); juce::MidiBuffer midi;
        for(int c=0;c<2;++c) for(int n=0;n<512;++n) {
            uint32_t bits=rng(); if((bits&0x7f800000u)==0x7f800000u) bits^=0x00800000u;
            b.setSample(c,n,std::bit_cast<float>(bits));
        }
        b.setSample(0,0,std::bit_cast<float>(uint32_t{1})); b.setSample(1,0,std::bit_cast<float>(uint32_t{0x80000001})); ref.makeCopyOf(b); exact.processBlock(b,midi);
        for(int c=0;c<2;++c) for(int n=0;n<512;++n) require(std::bit_cast<uint32_t>(b.getSample(c,n))==std::bit_cast<uint32_t>(ref.getSample(c,n)),"wide-range bitwise null");
    }
    {
        DiamondIIAudioProcessor gain; set(gain,"input",6); set(gain,"output",-3); gain.prepareToPlay(48000,32);
        auto mono=gain.getBusesLayout(); mono.inputBuses.set(0,juce::AudioChannelSet::mono()); mono.outputBuses.set(0,juce::AudioChannelSet::mono());
        require(gain.setBusesLayout(mono),"mono layout");
        juce::AudioBuffer<float> b(1,32); juce::MidiBuffer midi; for(int n=0;n<32;++n) b.setSample(0,n,.1f);
        gain.processBlock(b,midi); for(int n=0;n<32;++n) require(std::abs(b.getSample(0,n)-.1f*std::pow(10.f,3.f/20))<1e-6f,"dry gain routing");
    }
    for(int type=0;type<5;++type) {
        diamond::Engine e; e.prepare(48000); double diff=0;
        for(int n=0;n<48000;++n) { float x=.7f*std::sin(float(n)*.0576f); float y=e.tick(x,0,0,1,0,type); diff+=std::abs(y-x); }
        require(diff>1,"wet actually modifies audio");
    }
    DiamondIIAudioProcessor p; set(p,"input",2.5f); set(p,"process",48); set(p,"output",-1.2f); set(p,"type",1);
    { DiamondIIAudioProcessorEditor ed(p); require(ed.isResizable(),"resizable");
      for(int w:{588,735,980,1225,1470,1960}) { ed.setSize(w,juce::roundToInt(w*300./980.)); require(p.editorWidth==w,"resize persisted"); }
    }
    juce::MemoryBlock state; p.getStateInformation(state); DiamondIIAudioProcessor restored; restored.setStateInformation(state.getData(),(int)state.getSize());
    require(restored.editorWidth==1960,"restore width"); require(restored.state.getRawParameterValue("process")->load()==48,"restore parameter");
    juce::MemoryBlock again; restored.getStateInformation(again);
    require(state==again,"state round trip");
    // Warm state must not leak into a later exact-unity block.
    p.prepareToPlay(48000,64); juce::AudioBuffer<float> b(2,64); juce::MidiBuffer m;
    for(int c=0;c<2;++c) for(int n=0;n<64;++n) b.setSample(c,n,.6f); p.processBlock(b,m);
    set(p,"input",0); set(p,"output",0); set(p,"process",0); b.clear(); p.processBlock(b,m);
    for(int c=0;c<2;++c) for(int n=0;n<64;++n) require(b.getSample(c,n)==0,"unity after active state");
    std::cout<<"PASS: "<<checks<<" checks, 100% passed. Null difference exactly 0.0f (-inf dBFS); signed zero preserved.\n";
    return 0;
} catch(const std::exception& e) { std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n'; return 1; } }
