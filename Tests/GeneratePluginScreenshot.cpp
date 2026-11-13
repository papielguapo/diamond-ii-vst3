#include "../Source/PluginEditor.h"
#include <iostream>
int main(int argc,char** argv) {
    juce::ScopedJuceInitialiser_GUI gui;
    DiamondIIAudioProcessor p;
    auto set=[&](const char* id,float value) { auto* param=p.state.getParameter(id); param->setValueNotifyingHost(param->convertTo0to1(value)); };
    set("input",2.5f); set("process",48); set("output",-1.2f); set("brightness",1); set("type",1);
    DiamondIIAudioProcessorEditor editor(p); editor.setSize(980,300);
    auto image=editor.createComponentSnapshot(editor.getLocalBounds(),true,2.f);
    juce::File file(argc>1?juce::String(argv[1]):juce::File::getCurrentWorkingDirectory().getChildFile("assets/screenshot.png").getFullPathName());
    file.getParentDirectory().createDirectory();
    { juce::FileOutputStream stream(file); if(!stream.openedOk()) return 1; stream.setPosition(0); stream.truncate(); juce::PNGImageFormat png; if(!png.writeImageToStream(image,stream)) return 2; }
    std::cout<<file.getFullPathName()<<" "<<image.getWidth()<<"x"<<image.getHeight()<<" "<<file.getSize()<<" bytes\n";
    if(image.getWidth()!=1960 || image.getHeight()!=600 || file.getSize()<=50*1024) return 3;
    for(auto size:{751,939,1252,1565,1878,2504}) { editor.setSize(size,juce::roundToInt(size*380./1252.)); auto preview=editor.createComponentSnapshot(editor.getLocalBounds()); juce::FileOutputStream out(file.getSiblingFile("screenshot-"+juce::String(size)+".png")); out.setPosition(0); out.truncate(); juce::PNGImageFormat{}.writeImageToStream(preview,out); }
    DiamondIIAudioProcessor defaults;
    DiamondIIAudioProcessorEditor reference(defaults); reference.setSize(1252,380);
    auto defaultImage=reference.createComponentSnapshot(reference.getLocalBounds());
    juce::FileOutputStream defaultFile(file.getSiblingFile("screenshot-default.png")); defaultFile.setPosition(0); defaultFile.truncate();
    if(!juce::PNGImageFormat{}.writeImageToStream(defaultImage,defaultFile)) return 4;
    return 0;
}
