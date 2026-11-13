#include "PluginEditor.h"
#include <BinaryData.h>
DiamondLook::DiamondLook() {
    skin=juce::ImageCache::getFromMemory(BinaryData::skinreference_png,BinaryData::skinreference_pngSize);
    setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);
    setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff777777));
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::black);
    setColour(juce::TextEditor::backgroundColourId,juce::Colours::black);
    setColour(juce::TextEditor::textColourId,juce::Colours::white);
}
juce::Slider::SliderLayout DiamondLook::getSliderLayout(juce::Slider& s) {
    return {juce::Rectangle<int>(0,0,s.getWidth(),112),juce::Rectangle<int>(0,186,s.getWidth(),37)};
}
void DiamondLook::drawRotarySlider(juce::Graphics& g,int,int,int,int,float pos,float,float,juce::Slider& slider) {
    // The supplied artwork is the actual texture, preserving its lettering and knobs.
    const float sx=(float)slider.getProperties()["sx"],cx=(float)slider.getProperties()["cx"]-sx;
    const float cy=(float)slider.getProperties()["cy"]-143.f;
    const float neutral=(float)slider.getProperties()["neutral"],sweep=(float)slider.getProperties()["sweep"];
    juce::Graphics::ScopedSaveState save(g); juce::Path clip;
    clip.addEllipse(cx-52.5f,cy-52.5f,105.f,105.f); g.reduceClipRegion(clip);
    const float rotation=slider.getName()=="type"
        ? DiamondIIAudioProcessorEditor::typePointerAngle(juce::roundToInt(pos*4))-std::atan2(21.5f,25.5f)
        : (pos-neutral)*sweep;
    g.addTransform(juce::AffineTransform::rotation(rotation,cx,cy));
    g.drawImageAt(skin,-juce::roundToInt(sx),-143);
}
DiamondIIAudioProcessorEditor::DiamondIIAudioProcessorEditor(DiamondIIAudioProcessor& p):AudioProcessorEditor(p),processor(p) {
    setLookAndFeel(&look); addAndMakeVisible(panel); panel.setSize(1252,380); panel.setInterceptsMouseClicks(false,true);
    const float xs[]={92,324,556,788,992},cx[]={161.5f,393.5f,625.5f,858.f,1062.5f};
    const char* ids[]={"input","process","output","brightness","type"};
    for(int i=0;i<5;++i) {
        auto& k=knobs[i]; panel.addAndMakeVisible(k); k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,140,37); k.setName(ids[i]);
        k.getProperties().set("sx",xs[i]); k.getProperties().set("cx",cx[i]); k.getProperties().set("cy",i<3?199.5f:195.5f);
        k.getProperties().set("neutral",i==1||i==4?0.f:.5f); k.getProperties().set("sweep",i==3?1.3f:juce::MathConstants<float>::pi*1.5f);
        attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k);
        k.textFromValueFunction=[i](double value) {
            if(i==3) return juce::StringArray{"Opal","Gold","Sapphire"}[juce::jlimit(0,2,juce::roundToInt(value))];
            if(i==4) return juce::StringArray{"Luminscnt","Iridescnt","Radiant","Luster","DarkEssnc"}[juce::jlimit(0,4,juce::roundToInt(value))];
            if(value==0 && i!=1) return juce::String("-0.00"); return juce::String(value,2);
        };
        k.valueFromTextFunction=[i](const juce::String& text) {
            const juce::StringArray names=i==3?juce::StringArray{"Opal","Gold","Sapphire"}:juce::StringArray{"Luminscnt","Iridescnt","Radiant","Luster","DarkEssnc"};
            if(i>=3) return (double)juce::jmax(0,names.indexOf(text,true)); return text.getDoubleValue();
        };
        k.onValueChange=[this]{ repaint(); };
        k.updateText();
        k.setDoubleClickReturnValue(true,i==3?1:0);
        k.setTooltip("Drag to adjust; double-click to reset. Right-click the panel for interface size.");
    }
    const juce::StringArray typeNames{"Luminescent","Iridescent","Radiant","Luster","Dark Essence"};
    for(int i=0;i<5;++i) {
        auto& row=typeRows[i]; panel.addAndMakeVisible(row); row.setName(typeNames[i]); row.setComponentID("type-row-"+juce::String(i));
        row.setTooltip("Select "+typeNames[i]); row.onClick=[this,i] {
            auto* parameter=processor.state.getParameter("type"); parameter->beginChangeGesture(); parameter->setValueNotifyingHost(parameter->convertTo0to1((float)i)); parameter->endChangeGesture();
        };
    }
    knobs[4].setRotaryParameters(typePointerAngle(0)+juce::MathConstants<float>::twoPi,typePointerAngle(4)+juce::MathConstants<float>::twoPi,true);
    knobs[4].setMouseDragSensitivity(140);
    setResizable(true,true); setResizeLimits(313,95,2504,760); getConstrainer()->setFixedAspectRatio(1252./380.);
    int width=p.editorWidth.load();
    if(auto* display=juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()) width=juce::jlimit(313,2504,juce::jmin(width,display->userArea.getWidth()-40,juce::roundToInt((display->userArea.getHeight()-80)*1252./380.)));
    setSize(width,juce::roundToInt(width*380./1252.));
}
DiamondIIAudioProcessorEditor::~DiamondIIAudioProcessorEditor() { setLookAndFeel(nullptr); }
void DiamondIIAudioProcessorEditor::resized() {
    processor.editorWidth=getWidth(); processor.editorHeight=getHeight();
    const float scale=juce::jmin(getWidth()/1252.f,getHeight()/380.f);
    panel.setTransform(juce::AffineTransform::scale(scale).translated((getWidth()-1252*scale)*.5f,(getHeight()-380*scale)*.5f));
    const int xs[]={92,324,556,788,992}; for(int i=0;i<5;++i) knobs[i].setBounds(xs[i],143,140,223);
    for(int i=0;i<5;++i) typeRows[i].setBounds(1121,123+i*26,128,25);
}
void DiamondIIAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black); juce::Graphics::ScopedSaveState saved(g);
    const float scale=juce::jmin(getWidth()/1252.f,getHeight()/380.f);
    g.addTransform(juce::AffineTransform::scale(scale).translated((getWidth()-1252*scale)*.5f,(getHeight()-380*scale)*.5f));
    g.drawImageAt(look.skin,0,0);
    // Keep the reference's exact type lettering and its outer border.
    // Only erase the original selection underline, then move it to the active type.
    g.drawImage(look.skin,1124,142,125,5,6,142,76,5);
    const int chosen=juce::roundToInt((float)knobs[4].getValue());
    g.setColour(juce::Colour(0xff64168a)); g.drawLine(1124.f,143.f+chosen*26.f,1249.f,143.f+chosen*26.f,1.f);
}
void DiamondIIAudioProcessorEditor::mouseDown(const juce::MouseEvent& event) {
    if(!event.mods.isPopupMenu()) return;
    juce::PopupMenu menu; menu.addSectionHeader("Interface size");
    const int percentages[]={25,33,50,60,75,100,125,150,200}; for(int i=0;i<9;++i) menu.addItem(i+1,juce::String(percentages[i])+" %");
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),[safe=juce::Component::SafePointer<DiamondIIAudioProcessorEditor>(this)](int result) {
        if(!safe || result<1 || result>9) return; const double sizes[]={.25,1./3.,.5,.6,.75,1,1.25,1.5,2};
        safe->setSize(juce::roundToInt(1252*sizes[result-1]),juce::roundToInt(380*sizes[result-1]));
    });
}
