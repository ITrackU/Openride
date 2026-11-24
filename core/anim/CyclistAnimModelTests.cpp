// Dependency-free tests for the animation input model. Build/run via CMakeLists.txt.
#include "CyclistAnimModel.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    const FCyclistAnimParams P;
    const double Dt = 1.0 / 60.0;

    // Standstill: idle, no cadence.
    {
        FCyclistAnimState S;
        for (int i = 0; i < 120; ++i) StepAnimModel(S, P, 0.0, 0.0, Dt);
        CHECK(!S.bPedaling);
        CHECK(S.Cadence < 1e-6);
        CHECK(std::abs(S.Lean) < 1e-9);
    }

    // Steady 10 m/s: cadence = 10 / 6.5 * 60 ~ 92 rpm, pedalling, no sprint (< 100).
    {
        FCyclistAnimState S;
        for (int i = 0; i < 300; ++i) StepAnimModel(S, P, 10.0, 0.0, Dt);
        CHECK(S.bPedaling);
        CHECK(std::abs(S.Cadence - 10.0 / 6.5 * 60.0) < 1.0);
        CHECK(!S.bSprinting);
    }

    // Steady 12 m/s: cadence ~ 111 rpm -> sprinting; cadence clamped to MaxCadence.
    {
        FCyclistAnimState S;
        for (int i = 0; i < 300; ++i) StepAnimModel(S, P, 12.0, 0.0, Dt);
        CHECK(S.bSprinting);
        for (int i = 0; i < 300; ++i) StepAnimModel(S, P, 30.0, 0.0, Dt);
        CHECK(S.Cadence <= P.MaxCadence + 1e-9);
    }

    // Coasting: speed falls at 0.4 m/s^2 -> cadence decays to 0, sprint off.
    {
        FCyclistAnimState S;
        double V = 12.0;
        for (int i = 0; i < 300; ++i) StepAnimModel(S, P, V, 0.0, Dt);
        for (int i = 0; i < 180; ++i) { V -= 0.4 * Dt; StepAnimModel(S, P, V, 0.0, Dt); }
        CHECK(!S.bPedaling);
        CHECK(S.Cadence < 1.0);
        CHECK(!S.bSprinting);
    }

    // Cornering: right turn leans positive, left negative, bounded to [-1, 1], symmetric.
    {
        FCyclistAnimState R, L;
        const double Omega = 14.0 / 30.0; // 14 m/s on the 30 m radius arcs
        for (int i = 0; i < 300; ++i)
        {
            StepAnimModel(R, P, 14.0, Omega, Dt);
            StepAnimModel(L, P, 14.0, -Omega, Dt);
        }
        CHECK(R.Lean > 0.5 && R.Lean <= 1.0);
        CHECK(L.Lean < -0.5 && L.Lean >= -1.0);
        CHECK(std::abs(R.Lean + L.Lean) < 1e-9);
    }

    // Frame-rate independence: same motion at 30 and 120 fps gives similar cadence.
    {
        FCyclistAnimState A, B;
        for (int i = 0; i < 30 * 5; ++i) StepAnimModel(A, P, 8.0, 0.0, 1.0 / 30.0);
        for (int i = 0; i < 120 * 5; ++i) StepAnimModel(B, P, 8.0, 0.0, 1.0 / 120.0);
        CHECK(std::abs(A.Cadence - B.Cadence) < 1.0);
    }

    if (Failures == 0) std::printf("All anim model tests passed.\n");
    return Failures == 0 ? 0 : 1;
}
