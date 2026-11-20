// Dependency-free tests: build and run with `cmake` (see CMakeLists.txt) or
// `g++ -std=c++17 StadiumCircuit.cpp StadiumCircuitTests.cpp && ./a.out`.
#include "StadiumCircuit.h"

#include <cmath>
#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

static double Dist(const FCircuitSample& A, const FCircuitSample& B)
{
    return std::hypot(A.X - B.X, A.Y - B.Y);
}

int main()
{
    const FStadiumCircuit Circuit; // 400 m, R = 30 m

    // Total length: sum of two straights and two semicircles.
    const double Analytic = 2.0 * Circuit.GetStraightLength() + 2.0 * 3.14159265358979323846 * Circuit.GetRadius();
    CHECK(std::fabs(Analytic - 400.0) <= 0.1);

    // Polyline length of a dense sampling must also be 400 m +/- 0.1 m.
    const int N = 40000;
    const auto Samples = Circuit.GenerateSamples(N);
    CHECK(static_cast<int>(Samples.size()) == N);
    double Polyline = 0.0;
    for (int i = 0; i < N; ++i)
    {
        Polyline += Dist(Samples[i], Samples[(i + 1) % N]); // wraps: loop is closed
    }
    CHECK(std::fabs(Polyline - 400.0) <= 0.1);

    // Closed loop: distance 400 maps back to the start, and wrapping works both ways.
    CHECK(Dist(Circuit.SampleAtDistance(0.0), Circuit.SampleAtDistance(400.0)) < 1e-6);
    CHECK(Dist(Circuit.SampleAtDistance(-10.0), Circuit.SampleAtDistance(390.0)) < 1e-6);

    // Tangents are unit length and continuous across segment joins (no kinks).
    for (const auto& S : Samples)
    {
        CHECK(std::fabs(std::hypot(S.TangentX, S.TangentY) - 1.0) < 1e-9);
    }
    const double Joins[] = {Circuit.GetStraightLength(),
                            Circuit.GetStraightLength() + 3.14159265358979323846 * Circuit.GetRadius(),
                            2.0 * Circuit.GetStraightLength() + 3.14159265358979323846 * Circuit.GetRadius()};
    for (double J : Joins)
    {
        const auto Before = Circuit.SampleAtDistance(J - 1e-6);
        const auto After = Circuit.SampleAtDistance(J + 1e-6);
        CHECK(Dist(Before, After) < 1e-4);
        CHECK(std::fabs(Before.TangentX - After.TangentX) < 1e-4);
        CHECK(std::fabs(Before.TangentY - After.TangentY) < 1e-4);
    }

    // Tangent matches finite-difference direction of travel.
    for (double D = 0.0; D < 400.0; D += 7.3)
    {
        const auto A = Circuit.SampleAtDistance(D);
        const auto B = Circuit.SampleAtDistance(D + 1e-4);
        CHECK(std::fabs((B.X - A.X) / 1e-4 - A.TangentX) < 1e-3);
        CHECK(std::fabs((B.Y - A.Y) / 1e-4 - A.TangentY) < 1e-3);
    }

    // Invalid geometry is rejected.
    bool Threw = false;
    try { FStadiumCircuit Bad(400.0, 100.0); } catch (...) { Threw = true; }
    CHECK(Threw);

    std::printf(Failures ? "%d failure(s)\n" : "All tests passed\n", Failures);
    return Failures ? 1 : 0;
}
