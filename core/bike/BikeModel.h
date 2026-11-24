// Engine-independent bike visuals: wheel spin, steering and lean from motion.
// Header-only so ABikeActor and the CMake tests share it.
#pragma once

#include <algorithm>
#include <cmath>

namespace OpenRideCore
{
    struct FBikeParams
    {
        double WheelRadius = 0.34;     // m (700c with tyre)
        double Wheelbase = 1.0;        // m
        double MaxSteerRadians = 0.5;  // visual clamp for the fork
        double MaxLeanRadians = 0.52;  // ~30 deg frame roll
        double MinSteerSpeed = 0.5;    // m/s, below this the fork stays straight (yaw rate / speed is ill-conditioned)
        double Gravity = 9.81;
        double SmoothingTime = 0.1;    // s
    };

    struct FBikeState
    {
        double WheelAngle = 0.0; // rad, accumulated spin in [0, 2*pi); same for both wheels (no slip)
        double Steer = 0.0;      // rad, fork yaw; positive = right
        double Lean = 0.0;       // rad, frame roll; positive = right
    };

    // YawRate in rad/s, positive = turning right.
    inline void StepBikeModel(FBikeState& S, const FBikeParams& P, double Speed, double YawRate, double Dt)
    {
        if (Dt <= 0.0) return;
        Speed = std::max(0.0, Speed);
        const double TwoPi = 6.283185307179586;

        // Rolling without slip: angular speed = v / r. Wrap so precision does not degrade over long rides.
        if (P.WheelRadius > 0.0)
            S.WheelAngle = std::fmod(S.WheelAngle + Speed / P.WheelRadius * Dt, TwoPi);

        // Bicycle kinematics: tan(steer) = wheelbase * curvature, curvature = yawRate / v.
        const double TargetSteer = Speed >= P.MinSteerSpeed
            ? std::clamp(std::atan(P.Wheelbase * YawRate / Speed), -P.MaxSteerRadians, P.MaxSteerRadians) : 0.0;
        const double TargetLean = std::clamp(std::atan(Speed * YawRate / P.Gravity), -P.MaxLeanRadians, P.MaxLeanRadians);

        const double Alpha = P.SmoothingTime > 0.0 ? 1.0 - std::exp(-Dt / P.SmoothingTime) : 1.0;
        S.Steer += (TargetSteer - S.Steer) * Alpha;
        S.Lean += (TargetLean - S.Lean) * Alpha;
    }
}
