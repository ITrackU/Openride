// Dependency-free tests for lap tracking. Build/run via CMakeLists.txt.
#include "LapTracker.h"

#include <cmath>
#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    const double L = 400.0;
    const double Dt = 1.0 / 60.0;

    // Standing still does not start the clock.
    {
        FLapTracker T;
        for (int i = 0; i < 600; ++i) T.Step(0.0, Dt, L);
        CHECK(T.LapTime == 0.0 && T.Laps == 0);
    }

    // Constant 10 m/s: a lap every 40 s, three laps complete after 125 s.
    {
        FLapTracker T;
        for (int i = 0; i < 60 * 125; ++i) T.Step(10.0 * Dt, Dt, L);
        CHECK(T.Laps == 3);
        CHECK(std::fabs(T.LastLapTime - 40.0) < 0.02);
        CHECK(std::fabs(T.BestLapTime - 40.0) < 0.02);
        CHECK(std::fabs(T.LapTime - 5.0) < 0.05);
        CHECK(std::fabs(T.TotalDistance - 1250.0) < 0.01);
    }

    // A huge frame spanning the line still counts exactly one lap.
    {
        FLapTracker T;
        T.Step(399.0, 39.9, L);
        T.Step(2.0, 0.2, L);
        CHECK(T.Laps == 1);
        CHECK(std::fabs(T.LastLapTime - 40.0) < 1e-6);
        CHECK(std::fabs(T.LapTime - 0.1) < 1e-6);
    }

    // Best lap keeps the minimum; Reset clears everything.
    {
        FLapTracker T;
        T.Step(400.0, 50.0, L);
        T.Step(400.0, 40.0, L);
        T.Step(400.0, 45.0, L);
        CHECK(T.Laps == 3);
        CHECK(std::fabs(T.BestLapTime - 40.0) < 1e-6);
        CHECK(std::fabs(T.LastLapTime - 45.0) < 1e-6);
        T.Reset();
        CHECK(T.Laps == 0 && T.BestLapTime < 0.0 && T.TotalDistance == 0.0);
    }

    if (Failures == 0) std::printf("All lap tracker tests passed\n");
    return Failures == 0 ? 0 : 1;
}
