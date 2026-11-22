#pragma once
// Engine-independent appearance randomiser: turns a seed into a pleasant, reproducible
// set of colours so every NPC is the same base model with a different look.
// Used by UCyclistAppearanceData::MakeRandom (Source/OpenRide/Cyclist).
#include <cmath>
#include <cstdint>

namespace OpenRideCore
{
    struct FRgb
    {
        float R = 1.f, G = 1.f, B = 1.f; // linear-ish 0..1, no gamma handling here
    };

    struct FAppearance
    {
        FRgb Jersey;
        FRgb Shorts;
        FRgb Helmet;
        FRgb Frame;
    };

    // Small deterministic PRNG (xorshift32) so results are identical on every platform,
    // unlike std distributions whose output is implementation defined.
    struct FSeededRandom
    {
        uint32_t State;
        explicit FSeededRandom(uint32_t Seed) : State(Seed ? Seed : 0x9E3779B9u) {}
        float Next01()
        {
            State ^= State << 13;
            State ^= State >> 17;
            State ^= State << 5;
            return static_cast<float>(State >> 8) / 16777216.f; // 24 bits -> [0,1)
        }
    };

    inline FRgb HsvToRgb(float H, float S, float V)
    {
        H = H - std::floor(H); // wrap hue to [0,1)
        const float X = H * 6.f;
        const int Sector = static_cast<int>(X);
        const float F = X - static_cast<float>(Sector);
        const float P = V * (1.f - S);
        const float Q = V * (1.f - S * F);
        const float T = V * (1.f - S * (1.f - F));
        switch (Sector % 6)
        {
        case 0: return {V, T, P};
        case 1: return {Q, V, P};
        case 2: return {P, V, T};
        case 3: return {P, Q, V};
        case 4: return {T, P, V};
        default: return {V, P, Q};
        }
    }

    // Saturated jersey, dark neutral shorts, helmet and frame in a different hue from the jersey
    // so the rider reads clearly from the chase camera.
    inline FAppearance RandomAppearance(uint32_t Seed)
    {
        FSeededRandom Rng(Seed);
        const float JerseyHue = Rng.Next01();
        FAppearance A;
        A.Jersey = HsvToRgb(JerseyHue, 0.65f + 0.3f * Rng.Next01(), 0.7f + 0.3f * Rng.Next01());
        A.Shorts = HsvToRgb(Rng.Next01(), 0.2f * Rng.Next01(), 0.08f + 0.12f * Rng.Next01());
        A.Helmet = HsvToRgb(JerseyHue + 0.3f + 0.4f * Rng.Next01(), 0.5f * Rng.Next01() + 0.3f, 0.6f + 0.4f * Rng.Next01());
        A.Frame = HsvToRgb(JerseyHue + 0.25f + 0.5f * Rng.Next01(), 0.4f + 0.5f * Rng.Next01(), 0.4f + 0.5f * Rng.Next01());
        return A;
    }
}
