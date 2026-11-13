#include "../Source/DiamondIIDsp.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
int main(int argc,char** argv) {
    if(argc!=9) { std::cerr<<"Usage: DspReferenceRender input.f32 output.f32 fs type processPercent brightness inputDb outputDb\n"; return 1; }
    try {
        std::ifstream input(argv[1],std::ios::binary|std::ios::ate);
        if(!input) return 2;
        auto bytes=input.tellg(); if(bytes<0 || bytes%4!=0 || bytes>400000000) return 3;
        std::vector<float> samples(static_cast<size_t>(bytes)/sizeof(float)); input.seekg(0);
        if(!input.read(reinterpret_cast<char*>(samples.data()),bytes)) return 4;
        diamond::Engine engine; engine.prepare(std::stod(argv[3])); engine.setBrightness(std::stoi(argv[6]));
        const int type=std::stoi(argv[4]); const float amount=std::stof(argv[5])*.01f;
        const double in=std::pow(10.,std::stod(argv[7])/20.),out=std::pow(10.,std::stod(argv[8])/20.);
        for(auto& sample:samples) sample=engine.tickGained(sample,0,in,amount,out,type,amount==0 && in==1 && out==1);
        std::ofstream output(argv[2],std::ios::binary); output.write(reinterpret_cast<const char*>(samples.data()),bytes);
        return output?0:5;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 6; }
}
