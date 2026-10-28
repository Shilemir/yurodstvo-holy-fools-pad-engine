// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace yuro
{
// Deterministic, local PRNG. No shared random engine, allocation or locks.
struct OrganicDrift
{
    uint32_t seed = 1;
    float from = 0, to = 0;
    int elapsed = 0, length = 1;
    float random()
    {
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        return static_cast<float>(seed >> 8) / 8388607.5f - 1;
    }
    void reset(uint32_t value, double sr)
    {
        seed = value ? value : 1; from = random(); to = random(); elapsed = 0;
        length = static_cast<int>(sr * (2.0 + 2.4 * (random()+1)));
    }
    float next(double sr)
    {
        if (++elapsed >= length)
        {
            from = to; to = random(); elapsed = 0;
            length = static_cast<int>(sr * (1.7 + 2.65 * (random()+1)));
        }
        const float t = static_cast<float>(elapsed) / static_cast<float>(length);
        return from + (to-from) * t*t*(3-2*t);
    }
};

// Six damped complex poles per voice: harmonic + bell-like inharmonic modes.
// A normalized continuous drive plus an onset strike excites actual resonators.
class SacredResonance
{
public:
    inline static constexpr std::array<double, 6> ratios { 1.0, 2.01, 2.756, 4.07, 5.43, 6.8 };
    void reset(bool initialStrike=true) { modes = {}; tick = 0; initialized = false; onset=initialStrike; burstPosition=burstLength=0; }
    void strike(float strength,float colour,double sr)
    {
        burstLength=std::max(1,static_cast<int>(sr*.02)); burstPosition=0;
        burstStrength=strength; burstColour=colour;
        for(auto& m:modes){m.burstRe=1;m.burstIm=0;}
    }
    float process(float excitation, double frequency, double sr, float amount)
    {
        constexpr std::array<float,6> weights { .65f, .55f, .42f, .32f, .22f, .16f };
        if (tick++ % 64 == 0)
            for (size_t i=0; i<modes.size(); ++i)
            {
                auto& m=modes[i];
                const double hz=frequency*ratios[i];
                // Suppress modes beyond 42% of sample rate; never fold them down.
                const float audible=static_cast<float>(std::clamp((.44-hz/sr)/.06,0.0,1.0));
                const double decay=(4.0+8.0*amount)/(1.0+.22*static_cast<double>(i));
                const double radius=std::exp(-6.907755278982137/(sr*decay));
                const double angle=6.283185307179586*std::min(hz,sr*.42)/sr;
                m.targetRe=static_cast<float>(radius*std::cos(angle));
                m.targetIm=static_cast<float>(radius*std::sin(angle));
                m.drive=static_cast<float>(1-radius)*audible;
                m.stepAudible=(audible-m.audible)/64;
                if (!initialized)
                {
                    m.re=m.targetRe; m.im=m.targetIm;
                    m.real=onset ? .12f*weights[i]*audible : 0;
                    m.audible=audible; m.stepAudible=0;
                }
                m.stepRe=(m.targetRe-m.re)/64; m.stepIm=(m.targetIm-m.im)/64;
            }
        initialized=true;
        const float window=burstPosition<burstLength ? static_cast<float>(2*std::pow(std::sin(3.141592653589793*burstPosition/burstLength),2)/burstLength) : 0;
        float output=0;
        for(size_t i=0; i<modes.size(); ++i)
        {
            auto& m=modes[i]; m.re+=m.stepRe; m.im+=m.stepIm; m.audible+=m.stepAudible;
            const float real=m.re*m.real-m.im*m.imaginary + excitation*m.drive;
            m.imaginary=m.im*m.real+m.re*m.imaginary; m.real=real;
            if(window>0)
            {
                const float gain=.12f*weights[i]*burstStrength*(1+burstColour*static_cast<float>(i)/10)*window*m.audible;
                m.real+=gain*m.burstRe; m.imaginary+=gain*m.burstIm;
            }
            if(burstPosition<burstLength)
            {
                const float next=m.re*m.burstRe-m.im*m.burstIm;
                m.burstIm=m.im*m.burstRe+m.re*m.burstIm;m.burstRe=next;
            }
            output+=weights[i]*m.imaginary*m.audible;
        }
        if(burstPosition<burstLength)++burstPosition;
        return output;
    }
private:
    struct Mode
    {
        float real=0, imaginary=0, re=0, im=0, targetRe=0, targetIm=0;
        float stepRe=0, stepIm=0, drive=0, audible=0, stepAudible=0;
        float burstRe=1,burstIm=0;
    };
    std::array<Mode,6> modes {};
    unsigned tick=0;
    bool initialized=false;
    bool onset=true;
    int burstPosition=0,burstLength=0;
    float burstStrength=0,burstColour=0;
};
}
