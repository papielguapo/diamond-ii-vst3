#include "PluginEditor.h"
DiamondLook::DiamondLook() {
    setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffeee2ff));
    setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff191124));
    setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff20162e));
    setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff79558f));
    setColour(juce::ComboBox::textColourId,juce::Colour(0xfff0e4ff));
}
void DiamondLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&) {
    auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(17);
    float size=juce::jmin(r.getWidth(),r.getHeight()); r=r.withSizeKeepingCentre(size,size);
    const auto centre=r.getCentre(); float radius=size*.5f;
    for(int i=0;i<=40;++i) { auto a=start+(end-start)*i/40.f; g.setColour(i<=pos*40?juce::Colour(0xffd8a4ff):juce::Colour(0xff5b426d)); g.drawLine(centre.x+std::sin(a)*(radius+7),centre.y-std::cos(a)*(radius+7),centre.x+std::sin(a)*(radius+11),centre.y-std::cos(a)*(radius+11),1.4f); }
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff8c639f),r.getTopLeft(),juce::Colour(0xff18111e),r.getBottomRight(),false));
    g.fillEllipse(r); g.setColour(juce::Colour(0xffbda4cf)); g.drawEllipse(r,1.2f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff493453),r.getTopLeft(),juce::Colour(0xff211729),r.getBottomRight(),false)); g.fillEllipse(r.reduced(6));
    auto a=start+pos*(end-start); g.setColour(juce::Colour(0xfff5e1ff));
    g.drawLine(centre.x+std::sin(a)*radius*.35f,centre.y-std::cos(a)*radius*.35f,centre.x+std::sin(a)*radius*.8f,centre.y-std::cos(a)*radius*.8f,3.f);
    g.setColour(juce::Colours::white.withAlpha(.08f)); g.drawEllipse(r.reduced(10),.7f);
}
DiamondIIAudioProcessorEditor::DiamondIIAudioProcessorEditor(DiamondIIAudioProcessor& p):AudioProcessorEditor(p),processor(p) {
    setLookAndFeel(&look); addAndMakeVisible(panel); panel.setSize(980,300);
    int i=0; for(auto id:{"input","process","output"}) {
        auto& k=knobs[i]; panel.addAndMakeVisible(k); k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,110,24); k.setNumDecimalPlacesToDisplay(2); k.setTextValueSuffix(i==1?" %":" dB"); k.setName(id); k.setDoubleClickReturnValue(true,0);
        attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,k); ++i;
    }
    panel.addAndMakeVisible(brightness); brightness.addItemList({"Opal","Gold","Sapphire"},1); brightness.setName("Brightness");
    panel.addAndMakeVisible(type); type.addItemList({"Luminscnt","Iridescnt","Radiant","Luster","DarkEssnc"},1); type.setName("Type");
    panel.addAndMakeVisible(zoom); zoom.addItemList({"60 %","75 %","100 %","125 %","150 %","200 %"},1); zoom.setSelectedId(3); zoom.setName("Interface size");
    zoom.onChange=[this] { const double s[]={.6,.75,1,1.25,1.5,2}; int n=zoom.getSelectedItemIndex(); if(n>=0) setSize(juce::roundToInt(980*s[n]),juce::roundToInt(300*s[n])); };
    ba=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"brightness",brightness);
    ta=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"type",type);
    setResizable(true,true); setResizeLimits(588,180,1960,600); getConstrainer()->setFixedAspectRatio(980./300.);
    int width=p.editorWidth.load();
    if(auto* display=juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()) width=juce::jlimit(588,1960,juce::jmin(width,display->userArea.getWidth()-40,juce::roundToInt((display->userArea.getHeight()-80)*980./300.)));
    setSize(width,juce::roundToInt(width*300./980.));
}
DiamondIIAudioProcessorEditor::~DiamondIIAudioProcessorEditor() { setLookAndFeel(nullptr); }
void DiamondIIAudioProcessorEditor::resized() {
    processor.editorWidth=getWidth(); processor.editorHeight=getHeight();
    const int widths[]={588,735,980,1225,1470,1960}; int selected=0;
    for(int i=0;i<6;++i) if(getWidth()==widths[i]) selected=i+1;
    zoom.setSelectedId(selected,juce::dontSendNotification);
    if(selected==0) zoom.setText(juce::String(getWidth()*100./980.,0)+" %",juce::dontSendNotification);
    panel.setTransform(juce::AffineTransform::scale(juce::jmin(getWidth()/980.f,getHeight()/300.f)));
    for(int i=0;i<3;++i) knobs[i].setBounds(300+i*180,84,154,174);
    brightness.setBounds(53,151,181,32); type.setBounds(53,217,181,32); zoom.setBounds(852,24,94,25);
}
void DiamondIIAudioProcessorEditor::paint(juce::Graphics& actual) {
    juce::Graphics::ScopedSaveState saved(actual); auto& g=actual;
    g.addTransform(juce::AffineTransform::scale(juce::jmin(getWidth()/980.f,getHeight()/300.f)));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff4d265e),0,0,juce::Colour(0xff100d18),980,300,false)); g.fillRect(0,0,980,300);
    // Fine brushed-metal engraving, rendered procedurally at any DPI.
    juce::Random noise(71); for(int i=0;i<6500;++i) { g.setColour(juce::Colours::white.withAlpha(noise.nextFloat()*.035f)); float x=noise.nextFloat()*980,y=noise.nextFloat()*300; g.drawLine(x,y,x+noise.nextFloat()*20,y,.5f); }
    g.setColour(juce::Colour(0xffa977c6)); g.drawRoundedRectangle(12,12,956,276,9,1);
    g.setColour(juce::Colour(0xffcf9ce9)); g.drawLine(34,69,946,69,.7f);
    g.setFont(juce::FontOptions(31.f,juce::Font::bold)); g.drawText("DIAMOND II",35,20,300,38,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(11.f)); g.setColour(juce::Colour(0xffc1a6cf)); g.drawText("P A P I E L G U A P O   E D I T I O N",343,29,380,20,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(12.f,juce::Font::bold));
    for(int i=0;i<3;++i) g.drawText(juce::StringArray{"INPUT","PROCESS","OUTPUT"}[i],300+i*180,74,154,16,juce::Justification::centred);
    g.drawText("BRIGHTNESS",53,125,181,20,juce::Justification::centredLeft); g.drawText("TAPE CHARACTER",53,191,181,20,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(10.f)); g.drawText("PARALLEL HARMONIC COLOUR  /  NATIVE RATE",300,264,600,16,juce::Justification::centred);
    for(auto pt:{juce::Point<float>(25,25),{955,25},{25,275},{955,275}}) { g.setColour(juce::Colour(0xff160f1d)); g.fillEllipse(pt.x-4,pt.y-4,8,8); g.setColour(juce::Colour(0xff9673a9)); g.drawLine(pt.x-2,pt.y-2,pt.x+2,pt.y+2,1); }
}
