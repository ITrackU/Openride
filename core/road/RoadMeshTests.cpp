// Dependency-free tests for the road geometry. Build/run via CMakeLists.txt.
#include "RoadMesh.h"

#include <cmath>
#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

// Sum of triangle areas; also verifies every triangle faces up (positive XY cross product).
static double TotalArea(const FRoadMesh& Mesh, bool& AllUp)
{
    double Area = 0.0;
    AllUp = true;
    for (size_t i = 0; i + 2 < Mesh.Indices.size(); i += 3)
    {
        const FRoadVertex& A = Mesh.Vertices[Mesh.Indices[i]];
        const FRoadVertex& B = Mesh.Vertices[Mesh.Indices[i + 1]];
        const FRoadVertex& C = Mesh.Vertices[Mesh.Indices[i + 2]];
        const double Cross = (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
        if (Cross <= 0.0)
        {
            AllUp = false;
        }
        Area += 0.5 * Cross;
    }
    return Area;
}

int main()
{
    const FStadiumCircuit Circuit; // 400 m, R = 30 m

    // Road: area of a ribbon around the centreline = width * length (curvature cancels
    // out for a centred ribbon on a closed loop), within chord-approximation error.
    {
        const FRoadMesh Road = BuildLoopRibbon(Circuit, 0.0, 6.0, 0.0, 0.5);
        bool Up = false;
        const double Area = TotalArea(Road, Up);
        CHECK(Up);
        CHECK(std::fabs(Area - 6.0 * 400.0) < 12.0); // < 0.5 %
        CHECK(Road.Indices.size() % 3 == 0);
        for (uint32_t I : Road.Indices) { CHECK(I < Road.Vertices.size()); }
    }

    // Wrong inputs give empty meshes instead of crashing.
    CHECK(BuildLoopRibbon(Circuit, 0.0, 0.0, 0.0, 0.5).Vertices.empty());
    CHECK(BuildLoopRibbon(Circuit, 0.0, 6.0, 0.0, 0.0).Vertices.empty());
    CHECK(BuildDashedRibbon(Circuit, 0.0, 0.15, 0.0, 0.0, 3.0, 0.5).Vertices.empty());

    // Dashes: 3 m dash + 3 m gap on 400 m -> round(66.7) = 67 dashes, each 3 m long.
    {
        const FRoadMesh Dashes = BuildDashedRibbon(Circuit, 0.0, 0.15, 0.01, 3.0, 3.0, 0.5);
        bool Up = false;
        const double Area = TotalArea(Dashes, Up);
        CHECK(Up);
        CHECK(std::fabs(Area - 67 * 3.0 * 0.15) < 0.5);
        CHECK(Dashes.Vertices.front().Z == 0.01);
    }

    // Lateral offset moves the ribbon sideways: at the start (heading +X) the normal is +Y.
    {
        const FRoadMesh Edge = BuildLoopRibbon(Circuit, 2.5, 0.2, 0.0, 0.5);
        CHECK(std::fabs(Edge.Vertices[0].Y - 2.4) < 1e-9);
        CHECK(std::fabs(Edge.Vertices[1].Y - 2.6) < 1e-9);
    }

    if (Failures == 0) { std::printf("All road mesh tests passed\n"); }
    return Failures == 0 ? 0 : 1;
}
