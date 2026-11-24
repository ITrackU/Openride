// Dependency-free tests for building placement. Build/run via CMakeLists.txt.
#include "BuildingPlacer.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

// Rounded-ish loop: rectangle 100 x 40 sampled densely along edges, CCW or CW.
static std::vector<FPoint2> Rect(bool Ccw)
{
    std::vector<FPoint2> L;
    auto Edge = [&](double X0, double Y0, double X1, double Y1)
    {
        for (int I = 0; I < 20; ++I) { const double T = I / 20.0; L.push_back({X0 + (X1 - X0) * T, Y0 + (Y1 - Y0) * T}); }
    };
    Edge(-50, -20, 50, -20); Edge(50, -20, 50, 20); Edge(50, 20, -50, 20); Edge(-50, 20, -50, -20);
    if (!Ccw) { std::reverse(L.begin(), L.end()); }
    return L;
}

static bool Inside(double X, double Y) { return X > -50 && X < 50 && Y > -20 && Y < 20; }

int main()
{
    CHECK(SignedArea(Rect(true)) > 0 && SignedArea(Rect(false)) < 0);

    const std::vector<FBuildingVariantSpec> Variants = {{8, 12, 8, 10, 6, 9, 1.f}, {14, 20, 10, 14, 10, 20, 1.f}};
    FBuildingParams P;

    for (bool Ccw : {true, false})
    {
        const auto Loop = Rect(Ccw);
        const auto R = PlaceBuildings(P, Variants, Loop);
        CHECK(R.size() > 5);
        for (size_t I = 0; I < R.size(); ++I)
        {
            const auto& B = R[I];
            // All on the outside, never inside the loop, always beyond the setback.
            CHECK(!Inside(B.X, B.Y));
            CHECK(DistanceToLoop(Loop, B.X, B.Y) >= P.Setback);
            CHECK(B.VariantIndex >= 0 && B.VariantIndex < 2);
            const auto& V = Variants[B.VariantIndex];
            CHECK(B.Width >= V.MinWidth && B.Width <= V.MaxWidth && B.Height >= V.MinHeight && B.Height <= V.MaxHeight);
            // No overlaps (bounding circles).
            for (size_t J = I + 1; J < R.size(); ++J)
            {
                const double D = std::hypot(R[J].X - B.X, R[J].Y - B.Y);
                const double Rs = 0.5 * std::hypot(B.Width, B.Depth) + 0.5 * std::hypot(R[J].Width, R[J].Depth);
                CHECK(D >= Rs);
            }
        }
    }

    // Deterministic per seed, different across seeds, both variants used.
    {
        const auto A = PlaceBuildings(P, Variants, Rect(true));
        const auto B = PlaceBuildings(P, Variants, Rect(true));
        FBuildingParams Q = P; Q.Seed = 7;
        const auto C = PlaceBuildings(Q, Variants, Rect(true));
        CHECK(A.size() == B.size() && A[3].X == B[3].X && A[3].Width == B[3].Width);
        CHECK(A[3].Width != C[3].Width || A[3].X != C[3].X);
        bool Seen[2] = {false, false};
        for (const auto& X : A) { Seen[X.VariantIndex] = true; }
        CHECK(Seen[0] && Seen[1]);
    }

    // Cap and degenerate input.
    {
        FBuildingParams Cap = P; Cap.MaxBuildings = 3;
        CHECK(PlaceBuildings(Cap, Variants, Rect(true)).size() <= 3);
        CHECK(PlaceBuildings(P, {}, Rect(true)).empty());
        CHECK(PlaceBuildings(P, Variants, {}).empty());
        std::vector<FBuildingVariantSpec> Zero = {{8, 12, 8, 10, 6, 9, 0.f}};
        CHECK(PlaceBuildings(P, Zero, Rect(true)).empty());
    }

    std::printf(Failures ? "FAILED (%d)\n" : "OK\n", Failures);
    return Failures ? 1 : 0;
}
