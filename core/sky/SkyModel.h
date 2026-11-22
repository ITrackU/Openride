// Engine-independent day-time model: maps a clock hour to sun direction and lighting levels.
// Header-only so UE (ASkyLightingActor) and the CMake tests share it.
#pragma once

#include <cmath>

namespace OpenRideCore
{
    struct FSkyState
    {
        double SunElevationDeg = 0.0;  // above horizon; negative = night
        double SunAzimuthDeg = 0.0;    // 0 = +X (north in our world), clockwise seen from above
        double SunIntensity = 0.0;     // lux, direct light
        double SunTemperatureK = 6500; // warm at the horizon, neutral at noon
        double SkyLightIntensity = 0.0;// ambient scale 0..1+
        double ExposureBias = 0.0;     // EV offset so night/dusk stay readable
        bool bNight = false;
    };

    inline double Clamp01(double V) { return V < 0.0 ? 0.0 : (V > 1.0 ? 1.0 : V); }

    // Wraps any hour value into [0, 24).
    inline double WrapHours(double Hours)
    {
        double H = std::fmod(Hours, 24.0);
        if (H < 0.0) H += 24.0;
        return H;
    }

    // Simple sinusoidal sun: rises 06:00 due east, peaks at noon, sets 18:00. Not astronomically
    // exact on purpose: a predictable day length keeps the game readable.
    inline FSkyState ComputeSky(double TimeOfDayHours, double MaxElevationDeg = 60.0)
    {
        const double Pi = 3.14159265358979323846;
        const double H = WrapHours(TimeOfDayHours);
        FSkyState S;

        const double Phase = (H - 6.0) / 12.0 * Pi; // 0 at sunrise, pi at sunset
        S.SunElevationDeg = std::sin(Phase) * MaxElevationDeg;
        S.SunAzimuthDeg = std::fmod(90.0 + (H - 6.0) * 15.0 + 360.0, 360.0); // east at 6h, south at 12h, west at 18h
        S.bNight = S.SunElevationDeg < -2.0;

        // Direct light fades out across the first ~10 degrees above the horizon.
        const double Day = Clamp01((S.SunElevationDeg + 2.0) / 12.0);
        S.SunIntensity = 10.0 * Day * Day;
        // Low sun is warm (golden hour), high sun neutral.
        const double High = Clamp01(S.SunElevationDeg / 40.0);
        S.SunTemperatureK = 2200.0 + (6500.0 - 2200.0) * High;
        // Ambient never drops to zero: moonlit floor of 5%.
        S.SkyLightIntensity = 0.05 + 0.95 * Day;
        S.ExposureBias = (1.0 - Day) * 2.0; // brighten up to +2 EV at night
        return S;
    }
}
