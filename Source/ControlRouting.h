#pragma once
#include <array>

namespace yuro
{
// Parameter CCs have precedence over every pad mode, including learned maps.
// They are independent of the pad channel and pad note/CC base.
inline constexpr std::array<int, 10> controlCC { 74, 71, 76, 77, 93, 18, 82, 83, 85, 17 };
inline constexpr std::array<int, 10> dawControlCC { 86, 87, 89, 90, 110, 111, 14, 15, 30, 31 };
inline constexpr std::array<const char*, 10> controlNames { "E1 SHIMMER", "E2 OSC MIX", "E3 ATTACK", "E4 RELEASE", "E5 REVERB", "E6 DELAY", "F1 MASTER", "F2 BASS", "F3 MID", "F4 TREBLE" };
inline int controlIndex(int cc) noexcept
{
    for (size_t i = 0; i < controlCC.size(); ++i)
        if (controlCC[i] == cc || dawControlCC[i] == cc) return static_cast<int>(i);
    return -1;
}
inline bool protectedController(int cc) noexcept { return controlIndex(cc) >= 0 || cc == 1 || cc == 64 || cc >= 120; }
}
