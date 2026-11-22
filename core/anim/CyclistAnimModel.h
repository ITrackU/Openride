// Engine-independent derivation of animation inputs (cadence, lean, sprint) from motion.
// Header-only so UCyclistAnimInstance and the CMake tests share it.
#pragma once

#include <algorithm>
#include <cmath>

namespace OpenRideCore
{
    struct FCyclistAnimParams
    {
        double GearDevelopment = 6.5;   // metres travelled per crank turn (fixed gear; no shifting yet)
        double MaxCadence = 120.0;      // rpm clamp
        double MinPedalSpeed = 0.5;     // m/s, below this the rider is idle
        double CoastAccelThreshold = -0.2; // m/s^2, decelerating harder than this = not pedalling
        double SprintOnCadence = 100.0; // rpm, enter standing sprint
        double SprintOffCadence = 90.0; // rpm, leave it (hysteresis avoids flicker)
        double MaxLeanRadians = 0.61;   // ~35 deg of body lean maps to Lean = +-1
        double Gravity = 9.81;          // m/s^2
        double SmoothingTime = 0.15;    // s, low-pass time constant for accel/cadence/lean
    };

    struct FCyclistAnimState
    {
        double Speed = 0.0;      // m/s
        double Cadence = 0.0;    // rpm, 0 when coasting or stopped
        double Lean = 0.0;       // -1 (left) .. +1 (right)
        bool bSprinting = false;
        bool bPedaling = false;

        // Internal filter state.
        double SmoothedAccel = 0.0;
        bool bHasPrevSpeed = false;
    };

    // YawRate in rad/s, positive = turning right (matches Unreal's yaw convention).
    inline void StepAnimModel(FCyclistAnimState& S, const FCyclistAnimParams& P,
                              double Speed, double YawRate, double Dt)
    {
        if (Dt <= 0.0) return;
        Speed = std::max(0.0, Speed);

        // Low-pass factor; exact for any Dt so it is frame-rate independent.
        const double Alpha = P.SmoothingTime > 0.0 ? 1.0 - std::exp(-Dt / P.SmoothingTime) : 1.0;

        // Acceleration is derived from speed so the model works for player and NPCs alike.
        const double RawAccel = S.bHasPrevSpeed ? (Speed - S.Speed) / Dt : 0.0;
        S.bHasPrevSpeed = true;
        S.SmoothedAccel += (RawAccel - S.SmoothedAccel) * Alpha;
        S.Speed = Speed;

        S.bPedaling = Speed >= P.MinPedalSpeed && S.SmoothedAccel > P.CoastAccelThreshold;

        const double TargetCadence = S.bPedaling && P.GearDevelopment > 0.0
            ? std::min(P.MaxCadence, Speed / P.GearDevelopment * 60.0) : 0.0;
        S.Cadence += (TargetCadence - S.Cadence) * Alpha;

        if (!S.bPedaling) S.bSprinting = false;
        else if (S.Cadence >= P.SprintOnCadence) S.bSprinting = true;
        else if (S.Cadence <= P.SprintOffCadence) S.bSprinting = false;

        // Banking angle for a lateral acceleration a = v * omega: atan(a / g).
        const double LateralAccel = Speed * YawRate;
        const double Angle = std::atan(LateralAccel / P.Gravity);
        const double TargetLean = P.MaxLeanRadians > 0.0
            ? std::clamp(Angle / P.MaxLeanRadians, -1.0, 1.0) : 0.0;
        S.Lean += (TargetLean - S.Lean) * Alpha;
    }
}
