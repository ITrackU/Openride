// Dependency-free tests for the foliage scatter. Build/run via CMakeLists.txt.
#include "FoliageScatter.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

static std::vector<FPoint2> Square()
{
    return {{-20, -20}, {20, -20}, {20, 20}, {-20, 20}};
}

int main()
{
    // Distance to loop.
    {
        const auto L = Square();
        CHECK(std::abs(DistanceToLoop(L, 0, -20)) < 1e-9);
        CHECK(std::abs(DistanceToLoop(L, 0, 0) - 20.0) < 1e-9);
        CHECK(std::abs(DistanceToLoop(L, 30, 0) - 10.0) < 1e-9);
        CHECK(std::abs(DistanceToLoop(L, 23, 24) - 5.0) < 1e-9); // corner distance
    }

    FFoliageParams P;
    P.HalfExtentX = 50; P.HalfExtentY = 40; P.DensityPerSqm = 1.0; P.RoadClearance = 5.0;

    // Count roughly density * area, none inside clearance, all inside bounds, scale in range.
    {
        const auto R = ScatterFoliage(P, Square());
        const double Full = 100.0 * 80.0 * P.DensityPerSqm;
        CHECK(R.size() > 0 && R.size() < Full);
        for (const auto& I : R)
        {
            CHECK(DistanceToLoop(Square(), I.X, I.Y) >= 5.0);
            CHECK(I.X >= -50 && I.X <= 50 && I.Y >= -40 && I.Y <= 40);
            CHECK(I.Scale >= P.MinScale && I.Scale <= P.MaxScale);
            CHECK(I.YawDegrees >= 0.f && I.YawDegrees < 360.f);
        }
    }

    // No road -> full coverage near density * area; density scales count.
    {
        const auto Lo = ScatterFoliage(P, {});
        CHECK(std::abs(static_cast<double>(Lo.size()) - 8000.0) < 400.0);
        FFoliageParams Q = P; Q.DensityPerSqm = 0.25;
        CHECK(ScatterFoliage(Q, {}).size() < Lo.size() / 3);
    }

    // Deterministic per seed, different across seeds.
    {
        const auto A = ScatterFoliage(P, Square());
        const auto B = ScatterFoliage(P, Square());
        FFoliageParams Q = P; Q.Seed = 2;
        const auto C = ScatterFoliage(Q, Square());
        CHECK(A.size() == B.size() && A[10].X == B[10].X && A[10].YawDegrees == B[10].YawDegrees);
        CHECK(A[10].X != C[10].X);
    }

    // Degenerate input and cap.
    {
        FFoliageParams Z = P; Z.DensityPerSqm = 0.0;
        CHECK(ScatterFoliage(Z, {}).empty());
        FFoliageParams Cap = P; Cap.DensityPerSqm = 1000.0; Cap.MaxInstances = 5000;
        CHECK(ScatterFoliage(Cap, {}).size() <= 5000);
    }

    std::printf(Failures ? "FAILED (%d)\n" : "OK\n", Failures);
    return Failures ? 1 : 0;
}
