#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <numbers>
#include "CalibratedCurve.h"

namespace diamond {
// Independent empirical model. Calibrated on a measured subset of Phoenix II;
// it is not a bit-identical reconstruction of its proprietary DSP.
struct Biquad {
    double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
    void reset() { z1=z2=0; }
    double tick(double x) { auto y=b0*x+z1; z1=b1*x-a1*y+z2; z2=b2*x-a2*y; return y; }
    void shelf(double fs, double fc, double db, double damping=std::sqrt(2.)) {
        const double k=std::tan(std::numbers::pi*fc/fs), v=std::pow(10.,db/20.);
        const double q=damping, n=1./(1.+q*k+k*k);
        b0=(v+std::sqrt(v)*q*k+k*k)*n;
        b1=2.*(k*k-v)*n; b2=(v-std::sqrt(v)*q*k+k*k)*n;
        a1=2.*(k*k-1.)*n; a2=(1.-q*k+k*k)*n;
    }
};
struct Engine {
    struct Channel { Biquad modelPre,modelPost; double previousShaped=0; };
    std::array<Channel,2> channels;
    double rate=48000;
    int brightness=-1,activeType=-1;
    static double pade32(double x) { x=std::clamp(x,-3.,3.); auto u=x*x; return x*(27.+u)/(27.+9.*u); }
    static double pade54(double x) { x=std::clamp(x,-4.,4.); auto u=x*x; return x*(945.+105.*u+u*u)/(945.+420.*u+15.*u*u); }
    void prepare(double fs) {
        rate=fs; channels={}; brightness=-1; activeType=-1; setBrightness(1); setType(0);
    }
    void setBrightness(int b) {
        if (brightness==b) return;
        brightness=std::clamp(b,0,2);
        const int type=std::max(activeType,0); activeType=-1; setType(type);
    }
    void setType(int type) {
        type=std::clamp(type,0,4); if(type==activeType) return;
        activeType=type; const auto& p=modelCalibration[static_cast<size_t>(brightness*5+type)];
        for(auto& c:channels) {
            c.modelPre.shelf(rate,std::min(p.preFrequency,rate*.45),p.preDb,p.preDamping);
            c.modelPost.shelf(rate,std::min(p.postFrequency,rate*.45),p.postDb,p.postDamping);
        }
    }
    double residual(double x, Channel& c, int type) {
        const auto index=static_cast<size_t>(brightness*5+type);
        const double excitation=c.modelPre.tick(x);
        const double shaped=calibratedCurves[index].evaluate(excitation);
        const double square=excitation*excitation;
        const double corrected=shaped+modelCalibration[index].phaseMemory*(square/(1.+square))*(shaped-c.previousShaped);
        c.previousShaped=shaped;
        return c.modelPost.tick(corrected)-x;
    }
    float tick(float input,int channel,float inputDb,float process,float outputDb,int type) {
        const double inGain=inputDb==0?1.:std::pow(10.,inputDb/20.);
        const double outGain=outputDb==0?1.:std::pow(10.,outputDb/20.);
        return tickGained(input,channel,inGain,process,outGain,type,process==0 && inputDb==0 && outputDb==0);
    }
    float tickGained(float input,int channel,double inGain,float process,double outGain,int type,bool unity) {
        type=std::clamp(type,0,4); setType(type);
        auto& c=channels[static_cast<size_t>(channel)];
        const double dry=static_cast<double>(input)*inGain;
        const auto wet=residual(dry,c,type);
        // Direct return preserves every finite input bit, including negative zero.
        if(unity) return input;
        double result=dry+process*wet;
        if(brightness==1 && type>=3) {
            const auto& correction=processCorrection[static_cast<size_t>(type)];
            result+=4.*process*(1.-process)*(correction.dry*dry+correction.wet*wet);
        }
        return static_cast<float>(result*outGain);
    }
};
}
