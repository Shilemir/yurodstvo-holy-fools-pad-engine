// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 the YURODSTVO project owner and contributors.
#pragma once
#include <array>

namespace yuro
{
// Parameter CCs have precedence over every pad mode, including learned maps.
// They are independent of the pad channel and pad note/CC base.
inline constexpr std::array<int, 12> controlCC { 74, 71, 76, 77, 93, 18, 82, 83, 85, 17, 19, 16 };
inline constexpr std::array<int, 12> dawControlCC { 86, 87, 89, 90, 110, 111, 14, 15, 30, 31, 116, 117 };
inline constexpr std::array<const char*, 12> controlNames { "E1 DEVOTION", "E2 DUALISM", "E3 PATIENCE", "E4 REMEMBRANCE", "E5 SANCTUARY", "E6 SOLITUDE", "F1 MASTER", "F2 HELL", "F3 EARTH", "F4 HEAVEN", "E7 HARMONY", "E8 MADNESS" };
inline int controlIndex(int cc) noexcept
{
    for (size_t i = 0; i < controlCC.size(); ++i)
        if (controlCC[i] == cc || dawControlCC[i] == cc) return static_cast<int>(i);
    return -1;
}
inline bool protectedController(int cc) noexcept { return controlIndex(cc) >= 0 || cc == 1 || cc == 64 || cc >= 120; }
}
