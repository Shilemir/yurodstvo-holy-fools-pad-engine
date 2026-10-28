// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "WaveformTests.h"
#include "../Source/CathedralReverb.h"

inline void runDSPRefinementTests(const juce::File& root)
{
    juce::String csv = "sample_rate,effect,amount,early_energy_0_1s,energy_10_12s,energy_25_30s,peak\n";
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    {
        for (int effect = 0; effect < 2; ++effect)
        {
            double maximumEarly = 0, minimumEarly = 0;
            for (float amount : { 0.0f, .25f, .5f, .75f, 1.0f })
            {
                AmbientEffects fx; fx.prepare(sr, 1);
                fx.setTargets(effect == 0 ? amount : 0, effect == 1 ? amount : 0, 1);
                for (int i = 0; i < static_cast<int>(sr * .2); ++i) fx.process(0);
                double early = 0, late = 0, end = 0; float peak = 0;
                bool finite = true;
                const auto direct = fx.process(.1f);
                waveCheck(std::abs(direct[0] - .1f * (effect == 0 ? 1-.50f*amount : 1)) < .000001f,
                    "Effect macro has the specified dry gain and no hidden compensation");
                for (int i = 1; i < static_cast<int>(sr * 30); ++i)
                {
                    const auto y = fx.process(0);
                    const double energy = y[0]*y[0] + y[1]*y[1];
                    if (i < sr) early += energy;
                    if (i >= sr*10 && i < sr*12) late += energy;
                    if (i >= sr*25) end += energy;
                    for (auto x : y) { finite = finite && std::isfinite(x); peak = std::max(peak, std::abs(x)); }
                }
                waveCheck(finite && peak < .94f, "Thirty-second stereo impulse tail stays finite and below output ceiling");
                if (amount == 0) waveCheck(early == 0 && late == 0 && end == 0, "Zero macro yields no audible wet tail");
                if (amount == .25f) minimumEarly = early;
                if (amount == 1)
                {
                    maximumEarly = early;
                    waveCheck(late > early * .00001 && end < late * 2.5,
                        "Maximum macro has audible ten-second repeats/tail and a decaying late envelope");
                    if (effect == 0) waveCheck(end > 1.0e-12, "Cathedral tail extends beyond twenty-five seconds");
                }
                csv += juce::String(sr,0)+","+(effect==0 ? "reverb" : "delay")+","+juce::String(amount,2)+","
                    +juce::String(early,12)+","+juce::String(late,12)+","+juce::String(end,12)+","+juce::String(peak,8)+"\n";
            }
            waveCheck(maximumEarly > minimumEarly*4, "High macro produces substantially stronger wet output than 25 percent");
        }
        // Stereo decorrelation, not just duplicate mono reverberation.
        CathedralReverb space; space.prepare(sr); double difference=0, total=0;
        for (int i=0; i<static_cast<int>(sr*2); ++i)
        {
            auto y=space.process({i==0?.1f:0, i==0?.1f:0},1);
            difference+=(y[0]-y[1])*(y[0]-y[1]); total+=y[0]*y[0]+y[1]*y[1];
        }
        waveCheck(total>0 && difference/total>.1, "Cathedral reflections have a genuinely different left/right response");
    }
    root.getChildFile("Cathedral-delay-impulses.csv").replaceWithText(csv);

    AmbientEffects roomOnly, echoOnly, shimmerOnly, combined;
    for (auto* fx : { &roomOnly, &echoOnly, &shimmerOnly, &combined }) fx->prepare(48000, 1);
    roomOnly.setTargets(1,0,1); echoOnly.setTargets(0,1,1);
    shimmerOnly.setShimmer(1); combined.setTargets(1,1,1); combined.setShimmer(1);
    bool parallel = true, bounded = true; double earlyCombined=0, lateCombined=0;
    for (int i=0; i<48000*60; ++i)
    {
        const float input = i<48000*4 ? static_cast<float>(.005*std::sin(i*220.0*juce::MathConstants<double>::twoPi/48000.0)) : 0.0f;
        const auto a=roomOnly.process(input), b=echoOnly.process(input), c=shimmerOnly.process(input), y=combined.process(input);
        for(size_t ch=0; ch<2; ++ch)
        {
            parallel = parallel && std::abs(y[ch]-(a[ch]+b[ch]+c[ch]-2*input))<.000001f;
            bounded = bounded && std::isfinite(y[ch]) && std::abs(y[ch])<.94f;
            if(i>=48000*4 && i<48000*8) earlyCombined+=y[ch]*y[ch];
            if(i>=48000*56) lateCombined+=y[ch]*y[ch];
        }
    }
    waveCheck(parallel,"Combined low-level effects equal the sum of their independent returns; no effect ducks another");
    waveCheck(bounded && earlyCombined>0 && lateCombined<earlyCombined*.001,
        "All three maximum effects decay after excitation and remain bounded over sixty seconds");

    // Measure each oscillator separately while a held note is bent; deliberately
    // retain a non-zero relative offset to catch accidental retuning of oscillator II.
    for (int osc=0; osc<2; ++osc)
    {
        DroneProcessor p; setPlain(p,"attack",.005f); setPlain(p,"detune",25); setPlain(p,"mix",static_cast<float>(osc));
        p.prepareToPlay(48000,256); juce::AudioBuffer<float> a(2,256);
        for (int bend : {0,8192,16383})
        {
            juce::MidiBuffer m;
            if(bend==0) m.addEvent(juce::MidiMessage::noteOn(1,69,1.0f),0);
            m.addEvent(juce::MidiMessage::pitchWheel(1,bend),0); p.processBlock(a,m);
            for(int i=0;i<30;++i) { juce::MidiBuffer empty; p.processBlock(a,empty); }
            int count=0, crossings=0; float previous=0;
            while(count<48000)
            {
                juce::MidiBuffer empty; p.processBlock(a,empty);
                for(int i=0;i<256 && count<48000;++i,++count)
                {const float x=a.getSample(0,i);if(previous<=0 && x>0)++crossings;previous=x;}
            }
            const double expected=440*std::exp2((DroneProcessor::pitchToCents(bend)+osc*25)/1200.0);
            std::cout<<"Global bend oscillator "<<osc+1<<" bend="<<bend<<" frequency="<<crossings<<" expected="<<expected<<std::endl;
            waveCheck(std::abs(crossings-expected)<1.2,"Both oscillators follow global bend while retaining 25-cent relative tuning");
        }
    }
}
