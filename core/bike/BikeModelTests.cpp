// Dependency-free tests for the bike visual model. Build/run via CMakeLists.txt.
#include "BikeModel.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    const FBikeParams P;
    const double Dt = 1.0 / 60.0;

    // One metre travelled = 1/r radians of wheel spin (check before wrapping: 1 s at 1 m/s).
    {
        FBikeState S;
        for (int i = 0; i < 60; ++i) StepBikeModel(S, P, 1.0, 0.0, Dt);
        CHECK(std::abs(S.WheelAngle - 1.0 / P.WheelRadius) < 1e-6);
    }

    // Angle stays wrapped and the wheel does not spin when stopped.
    {
        FBikeState S;
        for (int i = 0; i < 6000; ++i) StepBikeModel(S, P, 15.0, 0.0, Dt);
        CHECK(S.WheelAngle >= 0.0 && S.WheelAngle < 6.2832);
        const double Before = S.WheelAngle;
        StepBikeModel(S, P, 0.0, 0.0, Dt);
        CHECK(S.WheelAngle == Before);
    }

    // Straight riding: no steer, no lean.
    {
        FBikeState S;
        for (int i = 0; i < 120; ++i) StepBikeModel(S, P, 10.0, 0.0, Dt);
        CHECK(std::abs(S.Steer) < 1e-9 && std::abs(S.Lean) < 1e-9);
    }

    // 14 m/s on the 30 m arcs: right turn steers/leans positive, left negative, symmetric.
    {
        FBikeState R, L;
        const double Omega = 14.0 / 30.0;
        for (int i = 0; i < 300; ++i)
        {
            StepBikeModel(R, P, 14.0, Omega, Dt);
            StepBikeModel(L, P, 14.0, -Omega, Dt);
        }
        CHECK(std::abs(R.Steer - std::atan(1.0 / 30.0)) < 1e-3);
        CHECK(R.Lean > 0.3 && R.Lean <= P.MaxLeanRadians);
        CHECK(L.Steer < 0.0 && L.Lean < 0.0);
        CHECK(std::abs(R.Steer + L.Steer) < 1e-12 && std::abs(R.Lean + L.Lean) < 1e-12);
    }

    // Standing still while rotating must not produce a wild steer angle.
    {
        FBikeState S;
        for (int i = 0; i < 120; ++i) StepBikeModel(S, P, 0.0, 3.0, Dt);
        CHECK(std::abs(S.Steer) < 1e-9);
    }

    if (Failures == 0) std::printf("All bike model tests passed.\n");
    return Failures == 0 ? 0 : 1;
}
