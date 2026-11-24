// Dependency-free tests for the cycling speed model. Build/run via CMakeLists.txt.
#include "CyclistMovement.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    const FCyclistMovementParams P;
    const double Dt = 1.0 / 60.0;

    // Throttle: speed rises, never exceeds MaxSpeed, approaches it.
    {
        FCyclistMovementState S;
        double Prev = 0.0;
        for (int i = 0; i < 60 * 120; ++i)
        {
            StepMovement(S, P, 1.0, 0.0, Dt);
            CHECK(S.Speed >= Prev - 1e-12);
            CHECK(S.Speed <= P.MaxSpeed + 1e-9);
            Prev = S.Speed;
        }
        CHECK(S.Speed > P.MaxSpeed * 0.99);
        CHECK(S.Distance > 0.0);
    }

    // Brake: stops and never goes negative (no reverse).
    {
        FCyclistMovementState S;
        S.Speed = P.MaxSpeed;
        for (int i = 0; i < 60 * 10; ++i)
        {
            StepMovement(S, P, 0.0, 1.0, Dt);
            CHECK(S.Speed >= 0.0);
        }
        CHECK(S.Speed == 0.0);
    }

    // Coasting slows down slower than braking.
    {
        FCyclistMovementState Coast, Brake;
        Coast.Speed = Brake.Speed = 10.0;
        for (int i = 0; i < 60; ++i)
        {
            StepMovement(Coast, P, 0.0, 0.0, Dt);
            StepMovement(Brake, P, 0.0, 1.0, Dt);
        }
        CHECK(Coast.Speed < 10.0);
        CHECK(Coast.Speed > Brake.Speed);
    }

    // Standing still with no input stays at rest.
    {
        FCyclistMovementState S;
        StepMovement(S, P, 0.0, 0.0, Dt);
        CHECK(S.Speed == 0.0 && S.Distance == 0.0);
    }

    // Configurable max speed is respected.
    {
        FCyclistMovementParams Slow;
        Slow.MaxSpeed = 5.0;
        FCyclistMovementState S;
        for (int i = 0; i < 60 * 60; ++i) StepMovement(S, Slow, 1.0, 0.0, Dt);
        CHECK(S.Speed <= 5.0 + 1e-9 && S.Speed > 4.9);
    }

    if (Failures == 0) std::printf("All CyclistMovement tests passed\n");
    return Failures == 0 ? 0 : 1;
}
