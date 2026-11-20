// Engine-independent road / lane-marking geometry generator.
// Produces flat ribbons (vertices + triangles) along an FStadiumCircuit so the
// geometry can be unit-tested on the ARM dev machine; ACircuitActor only copies
// the result into a UProceduralMeshComponent.
#pragma once

#include <cstdint>
#include <vector>

#include "../circuit/StadiumCircuit.h"

namespace OpenRideCore
{
struct FRoadVertex
{
    double X = 0.0; // metres, same frame as FCircuitSample
    double Y = 0.0;
    double Z = 0.0;
    double U = 0.0; // 0..1 across the ribbon
    double V = 0.0; // metres along the track (material tiles per metre)
};

struct FRoadMesh
{
    std::vector<FRoadVertex> Vertices;
    std::vector<uint32_t> Indices; // triangle list, front face is up (+Z) in Unreal's winding

    // Appends another mesh, re-basing its indices.
    void Append(const FRoadMesh& Other);
};

// Continuous ribbon around the whole lap.
// LateralOffset: metres from the centreline, positive = +normal side (see .cpp).
// StepMeters: sampling step; smaller = smoother curves.
FRoadMesh BuildLoopRibbon(const FStadiumCircuit& Circuit, double LateralOffset, double Width,
                          double Z, double StepMeters);

// Dashed ribbon around the lap. The period is adjusted slightly so an integer number
// of dashes fits the lap exactly (no visible seam at the start line).
FRoadMesh BuildDashedRibbon(const FStadiumCircuit& Circuit, double LateralOffset, double Width,
                            double Z, double DashLength, double GapLength, double StepMeters);
} // namespace OpenRideCore
