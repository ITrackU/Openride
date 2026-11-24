// Dependency-free tests for the NPC follower model. Build/run via CMakeLists.txt.
#include "NpcFollower.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    const double Dt = 1.0 / 60.0;

    // Reaches target speed without overshoot, and distance grows monotonically.
    {
        FNpcFollowerParams P;
        FNpcFollowerState S;
        double PrevDist = 0.0;
        for (int i = 0; i < 60 * 20; ++i)
        {
            StepNpcFollower(S, P, Dt);
            CHECK(S.Speed <= P.TargetSpeed + 1e-9);
            CHECK(S.Distance >= PrevDist);
            PrevDist = S.Distance;
        }
        CHECK(std::fabs(S.Speed - P.TargetSpeed) < 1e-9);
    }

    // Slows down to a lower target and never goes negative (even with a zero/negative target).
    {
        FNpcFollowerParams P;
        P.TargetSpeed = -5.0;
        FNpcFollowerState S;
        S.Speed = 10.0;
        for (int i = 0; i < 60 * 10; ++i)
        {
            StepNpcFollower(S, P, Dt);
            CHECK(S.Speed >= 0.0);
        }
        CHECK(S.Speed == 0.0);
    }

    // Lane drifts at no more than LateralSpeed and settles exactly on the target.
    {
        FNpcFollowerParams P;
        P.TargetLane = 2.0;
        FNpcFollowerState S;
        double Prev = 0.0;
        for (int i = 0; i < 60 * 10; ++i)
        {
            StepNpcFollower(S, P, Dt);
            CHECK(std::fabs(S.Lane - Prev) <= P.LateralSpeed * Dt + 1e-12);
            CHECK(S.Lane <= P.TargetLane + 1e-12);
            Prev = S.Lane;
        }
        CHECK(S.Lane == P.TargetLane);

        P.TargetLane = -1.0; // move back across the centre line
        for (int i = 0; i < 60 * 10; ++i) StepNpcFollower(S, P, Dt);
        CHECK(S.Lane == -1.0);
    }

    // Distance covered at constant speed matches v * t.
    {
        FNpcFollowerParams P;
        FNpcFollowerState S;
        S.Speed = P.TargetSpeed;
        for (int i = 0; i < 60 * 10; ++i) StepNpcFollower(S, P, Dt);
        CHECK(std::fabs(S.Distance - P.TargetSpeed * 10.0) < 1e-6);
    }

    if (Failures == 0) std::printf("All NpcFollower tests passed\n");
    return Failures == 0 ? 0 : 1;
}
