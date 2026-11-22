// Engine-independent NPC lane-following model. Header-only so UE and the CMake tests share it.
#pragma once

#include <algorithm>
#include <cmath>

namespace OpenRideCore
{
    struct FNpcFollowerParams
    {
        double TargetSpeed = 9.0;       // m/s the NPC tries to hold
        double Acceleration = 1.5;      // m/s^2 when speeding up
        double Deceleration = 3.0;      // m/s^2 when slowing down
        double TargetLane = 0.0;        // metres from the centre line, positive = right of travel
        double LateralSpeed = 0.8;      // max m/s of sideways drift, so lane changes look smooth
    };

    struct FNpcFollowerState
    {
        double Speed = 0.0;             // m/s, never negative
        double Distance = 0.0;          // metres along the circuit (unwrapped)
        double Lane = 0.0;              // current lateral offset in metres
    };

    // Speed and lane move towards their targets at bounded rates (never snap), so changing
    // TargetSpeed / TargetLane at runtime (e.g. for overtaking) stays visually smooth.
    inline void StepNpcFollower(FNpcFollowerState& State, const FNpcFollowerParams& Params, double DeltaSeconds)
    {
        const double Target = std::max(0.0, Params.TargetSpeed);
        if (State.Speed < Target)
        {
            State.Speed = std::min(Target, State.Speed + Params.Acceleration * DeltaSeconds);
        }
        else
        {
            State.Speed = std::max(Target, State.Speed - Params.Deceleration * DeltaSeconds);
        }
        State.Distance += State.Speed * DeltaSeconds;

        const double MaxStep = std::max(0.0, Params.LateralSpeed) * DeltaSeconds;
        const double Delta = Params.TargetLane - State.Lane;
        State.Lane += std::clamp(Delta, -MaxStep, MaxStep);
    }
}
