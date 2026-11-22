// Engine-independent cycling speed model. Header-only so UE and the CMake tests share it.
#pragma once

#include <algorithm>

namespace OpenRideCore
{
    struct FCyclistMovementParams
    {
        double MaxSpeed = 14.0;      // m/s (~50 km/h)
        double Acceleration = 2.0;   // m/s^2 while pedalling at standstill
        double BrakeDecel = 6.0;     // m/s^2 while braking
        double CoastDecel = 0.4;     // m/s^2 rolling/air drag when neither pedalling nor braking
    };

    struct FCyclistMovementState
    {
        double Speed = 0.0;          // m/s, never negative (no reverse in viewer mode)
        double Distance = 0.0;       // metres travelled along the circuit (unwrapped)
    };

    // Throttle and Brake are in [0, 1]. Acceleration tapers off linearly towards MaxSpeed
    // so top speed is approached smoothly instead of hitting a hard clamp.
    inline void StepMovement(FCyclistMovementState& State, const FCyclistMovementParams& Params,
                             double Throttle, double Brake, double DeltaSeconds)
    {
        Throttle = std::clamp(Throttle, 0.0, 1.0);
        Brake = std::clamp(Brake, 0.0, 1.0);

        double Accel = 0.0;
        if (Params.MaxSpeed > 0.0)
        {
            const double Headroom = std::max(0.0, 1.0 - State.Speed / Params.MaxSpeed);
            Accel += Throttle * Params.Acceleration * Headroom;
        }
        if (Throttle <= 0.0 && Brake <= 0.0)
        {
            Accel -= Params.CoastDecel;
        }
        Accel -= Brake * Params.BrakeDecel;

        State.Speed = std::clamp(State.Speed + Accel * DeltaSeconds, 0.0, std::max(0.0, Params.MaxSpeed));
        State.Distance += State.Speed * DeltaSeconds;
    }
}
