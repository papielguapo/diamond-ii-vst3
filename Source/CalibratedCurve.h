#pragma once
#include "CalibrationData.h"
#include "ResidualCalibration.h"
#include <array>
#include <algorithm>
#include <cmath>

namespace diamond {
// Constructed once at module load, never on the audio callback. The measured
// transfer functions are evaluated via cubic Hermite tables during processing.
struct CurveTable {
    static constexpr size_t intervals=8192;
    static constexpr double maximum=32.,step=maximum/intervals;
    std::array<float,intervals+1> values{},slopes{};
    explicit CurveTable(const ModelCalibration& p) {
        if(p.residualModel) {
            std::copy(darkGoldResidualValues.begin(),darkGoldResidualValues.end(),values.begin());
            std::copy(darkGoldResidualSlopes.begin(),darkGoldResidualSlopes.end(),slopes.begin());
            return;
        }
        for(size_t i=0;i<=intervals;++i) {
            const double x=i*step,t=1.+std::pow(p.drive*x,p.exponent);
            const double scale=p.gain*std::pow(t,-p.tailExponent/p.exponent);
            values[i]=static_cast<float>(x*scale);
            slopes[i]=static_cast<float>(step*scale*(1.-p.tailExponent+p.tailExponent/t));
        }
    }
    double evaluate(double input) const noexcept {
        const double x=std::min(std::abs(input),maximum),position=x/step;
        const auto index=std::min(static_cast<size_t>(position),intervals-1);
        const double t=position-index,t2=t*t,t3=t2*t;
        const double value=(2*t3-3*t2+1)*values[index]+(t3-2*t2+t)*slopes[index]
            +(-2*t3+3*t2)*values[index+1]+(t3-t2)*slopes[index+1];
        return std::signbit(input)?-value:value;
    }
};
inline const std::array<CurveTable,15> calibratedCurves{
    CurveTable(modelCalibration[0]),
    CurveTable(modelCalibration[1]),
    CurveTable(modelCalibration[2]),
    CurveTable(modelCalibration[3]),
    CurveTable(modelCalibration[4]),
    CurveTable(modelCalibration[5]),
    CurveTable(modelCalibration[6]),
    CurveTable(modelCalibration[7]),
    CurveTable(modelCalibration[8]),
    CurveTable(modelCalibration[9]),
    CurveTable(modelCalibration[10]),
    CurveTable(modelCalibration[11]),
    CurveTable(modelCalibration[12]),
    CurveTable(modelCalibration[13]),
    CurveTable(modelCalibration[14])};
inline const CurveTable lowAmountDarkGoldCurve(lowAmountDarkGold);
}
