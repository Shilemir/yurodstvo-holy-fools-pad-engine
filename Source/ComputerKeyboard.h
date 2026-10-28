// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cctype>

// Message-thread key ownership. Audio receives a bounded per-note target snapshot.
// A held key is inhibited after focus/range/lifecycle cancellation until released.
class ComputerKeyboard
{
public:
    inline static constexpr std::array<int,13> keys {'A','W','S','E','D','F','T','G','Y','H','U','J','K'};
    using Keys = std::array<bool,13>;
    using Notes = std::array<bool,128>;
    static int index(int key)
    {
        if(key<0||key>127)return -1;
        for(size_t i=0;i<keys.size();++i)if(keys[i]==std::toupper(key))return static_cast<int>(i);
        return -1;
    }
    const Notes& update(const Keys& down,int first,bool enabled)
    {
        if(!enabled || first!=base || !wasEnabled)
        {
            sounding.fill(false);blocked=down;
        }
        for(size_t i=0;i<down.size();++i)
        {
            if(!down[i])blocked[i]=false;
            const int note=first+static_cast<int>(i);
            if(note>=0&&note<128)sounding[static_cast<size_t>(note)]=enabled&&down[i]&&!blocked[i];
        }
        base=first;wasEnabled=enabled;return sounding;
    }
    void cancel(const Keys& down) { sounding.fill(false);blocked=down;wasEnabled=false; }
    const Notes& notes() const {return sounding;}
private:
    Notes sounding {};
    Keys blocked {};
    int base=-1;
    bool wasEnabled=false;
};
