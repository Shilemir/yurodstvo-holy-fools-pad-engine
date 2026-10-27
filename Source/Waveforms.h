#pragma once
#include <array>
#include <cmath>
#include <algorithm>

namespace yuro
{
inline constexpr std::array<const char*, 4> waveformNames { "Sine", "Triangle", "Sawtooth", "Square" };
inline constexpr std::array<const char*, 2> waveformIds { "osc1Waveform", "osc2Waveform" };

// Immutable, octave-banked Fourier tables. Created once before rendering;
// sample lookup is bounded and allocation-free. Triangle uses odd 1/n^2
// harmonics, saw 1/n, square odd 1/n. Added partials stay below 0.45 * sample rate.
class WaveTables
{
public:
    static constexpr int size = 4096, levels = 11;
    using Table = std::array<float, size + 1>;
    WaveTables()
    {
        constexpr double pi = 3.14159265358979323846;
        for (int level = 0; level < levels; ++level)
        {
            const int harmonics = 1 << level;
            for (int i = 0; i < size; ++i)
            {
                const double angle = 2 * pi * i / size;
                const double sine = std::sin(angle), cosine = std::cos(angle);
                double sn = sine, cn = cosine, triangle = 0, saw = 0, square = 0;
                for (int n = 1; n <= harmonics; ++n)
                {
                    saw -= (2 / pi) * sn / n;
                    if ((n & 1) != 0)
                    {
                        square += (4 / pi) * sn / n;
                        triangle += (8 / (pi * pi)) * (((n / 2) & 1) ? -1 : 1) * sn / (n * n);
                    }
                    const double next = sn * cosine + cn * sine;
                    cn = cn * cosine - sn * sine; sn = next;
                }
                tables[0][static_cast<size_t>(level)][static_cast<size_t>(i)] = static_cast<float>(triangle);
                tables[1][static_cast<size_t>(level)][static_cast<size_t>(i)] = static_cast<float>(saw);
                tables[2][static_cast<size_t>(level)][static_cast<size_t>(i)] = static_cast<float>(square);
            }
            // Match useful RMS levels, with an explicit peak budget so an
            // ordinary 8-voice chord remains below the existing soft knee.
            for (auto& shape : tables)
            {
                auto& table = shape[static_cast<size_t>(level)];
                double squares = 0; float peak = 0;
                for (int i = 0; i < size; ++i)
                {
                    const float value = table[static_cast<size_t>(i)];
                    squares += value * value; peak = std::max(peak, std::abs(value));
                }
                const float gain = std::min(0.65f / static_cast<float>(std::sqrt(squares / size)), 1.15f / peak);
                for (int i = 0; i < size; ++i) table[static_cast<size_t>(i)] *= gain;
                table[size] = table[0];
            }
        }
    }
    float sample(int shape, double phase, double increment) const noexcept
    {
        if (shape == 0) return static_cast<float>(std::sin(phase * 6.28318530717958647692));
        const double allowance = 0.45 / increment;
        int exponent = 0;
        std::frexp(std::max(1.0, allowance), &exponent);
        const int high = std::clamp(exponent - 1, 0, levels - 1);
        const int low = std::max(0, high - 1);
        const float blend = static_cast<float>(std::clamp(allowance / (1 << high) - 1.0, 0.0, 1.0));
        const auto& bank = tables[static_cast<size_t>(std::clamp(shape - 1, 0, 2))];
        const double position = phase * size;
        const auto index = static_cast<size_t>(position);
        const float fraction = static_cast<float>(position - static_cast<double>(index));
        const auto read = [&](int level)
        {
            const auto& t = bank[static_cast<size_t>(level)];
            return t[index] + fraction * (t[index + 1] - t[index]);
        };
        const float a = read(low), b = read(high);
        return a + blend * (b - a);
    }
private:
    std::array<std::array<Table, levels>, 3> tables {};
};
}
