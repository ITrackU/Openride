// Engine-independent lap/distance/timing bookkeeping for the HUD. Header-only so UE and CMake tests share it.
#pragma once

#include <cmath>

namespace OpenRideCore
{
    struct FLapTracker
    {
        double TotalDistance = 0.0;   // metres ridden since start (unwrapped, never decreases)
        int Laps = 0;                 // completed laps
        double LapTime = 0.0;         // seconds on the current lap
        double LastLapTime = -1.0;    // seconds of the last completed lap, <0 if none yet
        double BestLapTime = -1.0;    // <0 if none yet

        // Advance is the distance covered this frame (>= 0). Fed with the movement step rather
        // than the wrapped circuit distance so lap crossings cannot be missed or double counted.
        // Timing is only advanced while moving or after the first metre, so a rider standing at
        // the start line does not burn lap time.
        void Step(double Advance, double DeltaSeconds, double LoopLength)
        {
            if (Advance < 0.0) Advance = 0.0;
            if (DeltaSeconds < 0.0) DeltaSeconds = 0.0;
            if (TotalDistance <= 0.0 && Advance <= 0.0) return; // not started yet

            const double PrevTotal = TotalDistance;
            TotalDistance += Advance;
            LapTime += DeltaSeconds;

            if (LoopLength <= 0.0) return;
            const int NewLaps = static_cast<int>(std::floor(TotalDistance / LoopLength));
            if (NewLaps > Laps)
            {
                // Interpolate the exact crossing instant inside this frame; the remainder
                // of the frame belongs to the new lap.
                const double CrossDist = NewLaps * LoopLength;
                const double Frac = Advance > 0.0 ? (CrossDist - PrevTotal) / Advance : 1.0;
                const double Overshoot = DeltaSeconds * (1.0 - Frac);
                LastLapTime = LapTime - Overshoot;
                if (BestLapTime < 0.0 || LastLapTime < BestLapTime) BestLapTime = LastLapTime;
                Laps = NewLaps;
                LapTime = Overshoot;
            }
        }

        void Reset() { *this = FLapTracker(); }
    };
}
