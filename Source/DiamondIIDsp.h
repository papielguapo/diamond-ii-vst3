#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace diamond {
// Independent musical model, not a measured reconstruction of proprietary DSP.
struct Biquad {
    double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
    void reset() { z1=z2=0; }
    double tick(double x) { auto y=b0*x+z1; z1=b1*x-a1*y+z2; z2=b2*x-a2*y; return y; }
    void shelf(double fs, double fc, double db) {
        const double k=std::tan(std::numbers::pi*fc/fs), v=std::pow(10.,db/20.);
        const double q=std::sqrt(2.), n=1./(1.+q*k+k*k);
        b0=(v+std::sqrt(2.*v)*k+k*k)*n;
        b1=2.*(k*k-v)*n; b2=(v-std::sqrt(2.*v)*k+k*k)*n;
        a1=2.*(k*k-1.)*n; a2=(1.-q*k+k*k)*n;
    }
};
struct Engine {
    struct Channel { Biquad pre,post; double memory=0,env=0,dcX=0,dcY=0,previous=0; };
    std::array<Channel,2> channels;
    double rate=48000, attack=0,release=0,dcR=0,mag=0;
    int brightness=-1;
    static double pade32(double x) { x=std::clamp(x,-3.,3.); auto u=x*x; return x*(27.+u)/(27.+9.*u); }
    static double pade54(double x) { x=std::clamp(x,-4.,4.); auto u=x*x; return x*(945.+105.*u+u*u)/(945.+420.*u+15.*u*u); }
    void prepare(double fs) {
        rate=fs; attack=std::exp(-1./(.0012*fs)); release=std::exp(-1./(.045*fs));
        dcR=std::exp(-2.*std::numbers::pi*5./fs); mag=std::exp(-1./(.00003*fs));
        channels={}; brightness=-1; setBrightness(1);
    }
    void setBrightness(int b) {
        if (brightness==b) return;
        brightness=b;
        for(auto& c:channels) { c.pre.shelf(rate,2800.,b==2?3.5:0.); c.post.shelf(rate,b==0?2400.:3200.,b==0?-4.:(b==2?-1.75:.15)); }
    }
    double residual(double x, Channel& c, int type, double p) {
        const auto a=std::abs(x)>c.env?attack:release;
        c.env=a*c.env+(1-a)*std::abs(x);
        const auto old=c.memory; c.memory=(1-mag)*x+mag*old;
        const double u=x+.035*(old-x);
        constexpr double drive[]={1.18,1.5,2.15,.92,2.8};
        constexpr double even[]={.018,.095,.025,.035,.055};
        constexpr double odd[]={.065,.145,.11,.09,.18};
        double k=drive[type]; if(type==3) k+=2.35*std::min(c.env,1.)*(.35+.65*p);
        double shaped=pade54(k*u)/k;
        if(type==2 || type==4) shaped=pade32(1.4*k*shaped)/(1.4*k);
        // Chebyshev terms have their DC and linear parts removed, then are bounded.
        const double t=std::clamp(u,-1.,1.), t2=2*t*t, t3=4*t*t*t, t5=16*t*t*t*t*t-20*t*t*t;
        shaped+=even[type]*t2 + odd[type]*.1*t3 + .0005*t5;
        double delta=shaped-x;
        if(type==4) { double d=delta-c.previous; delta=c.previous+d/(1.+.35*std::abs(d)); }
        c.previous=delta;
        const double blocked=delta-c.dcX+dcR*c.dcY; c.dcX=delta; c.dcY=blocked;
        return c.post.tick(blocked);
    }
    float tick(float input,int channel,float inputDb,float process,float outputDb,int type) {
        const double inGain=inputDb==0?1.:std::pow(10.,inputDb/20.);
        const double outGain=outputDb==0?1.:std::pow(10.,outputDb/20.);
        return tickGained(input,channel,inGain,process,outGain,type,process==0 && inputDb==0 && outputDb==0);
    }
    float tickGained(float input,int channel,double inGain,float process,double outGain,int type,bool unity) {
        auto& c=channels[static_cast<size_t>(channel)];
        const double dry=static_cast<double>(input)*inGain;
        const auto wet=residual(c.pre.tick(dry),c,std::clamp(type,0,4),process);
        // Direct return preserves every finite input bit, including negative zero.
        if(unity) return input;
        return static_cast<float>((dry+process*wet)*outGain);
    }
};
}
