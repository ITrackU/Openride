#include "RoadMesh.h"

#include <algorithm>
#include <cmath>

namespace OpenRideCore
{
void FRoadMesh::Append(const FRoadMesh& Other)
{
    const uint32_t Base = static_cast<uint32_t>(Vertices.size());
    Vertices.insert(Vertices.end(), Other.Vertices.begin(), Other.Vertices.end());
    for (uint32_t Index : Other.Indices)
    {
        Indices.push_back(Base + Index);
    }
}

namespace
{
// Ribbon between two distances (EndDist > StartDist, may exceed the lap length: sampling wraps).
FRoadMesh BuildRibbonSegment(const FStadiumCircuit& Circuit, double StartDist, double EndDist,
                             double LateralOffset, double Width, double Z, double StepMeters)
{
    FRoadMesh Mesh;
    const double Length = EndDist - StartDist;
    if (Length <= 0.0 || Width <= 0.0 || StepMeters <= 0.0)
    {
        return Mesh;
    }
    const int Steps = std::max(1, static_cast<int>(std::ceil(Length / StepMeters)));
    const double Half = Width * 0.5;

    for (int i = 0; i <= Steps; ++i)
    {
        const double Dist = StartDist + Length * static_cast<double>(i) / Steps;
        const FCircuitSample S = Circuit.SampleAtDistance(Dist);
        // Normal = tangent rotated +90 degrees in the XY plane.
        const double NX = -S.TangentY;
        const double NY = S.TangentX;
        const double Near = LateralOffset - Half; // edge on the -normal side
        const double Far = LateralOffset + Half;  // edge on the +normal side
        // V uses the unwrapped distance so textures stay continuous across the start line.
        Mesh.Vertices.push_back({S.X + NX * Near, S.Y + NY * Near, Z, 0.0, Dist});
        Mesh.Vertices.push_back({S.X + NX * Far, S.Y + NY * Far, Z, 1.0, Dist});
    }
    for (int i = 0; i < Steps; ++i)
    {
        const uint32_t V0 = static_cast<uint32_t>(2 * i); // near, this station
        const uint32_t V1 = V0 + 1;                       // far, this station
        const uint32_t V2 = V0 + 2;                       // near, next station
        const uint32_t V3 = V0 + 3;                       // far, next station
        // (V0,V2,V1) has a positive XY cross product, i.e. faces up in Unreal's
        // clockwise-front convention. Both triangles keep that winding.
        Mesh.Indices.insert(Mesh.Indices.end(), {V0, V2, V1, V1, V2, V3});
    }
    return Mesh;
}
} // namespace

FRoadMesh BuildLoopRibbon(const FStadiumCircuit& Circuit, double LateralOffset, double Width,
                          double Z, double StepMeters)
{
    return BuildRibbonSegment(Circuit, 0.0, Circuit.GetTotalLength(), LateralOffset, Width, Z, StepMeters);
}

FRoadMesh BuildDashedRibbon(const FStadiumCircuit& Circuit, double LateralOffset, double Width,
                            double Z, double DashLength, double GapLength, double StepMeters)
{
    FRoadMesh Mesh;
    const double Total = Circuit.GetTotalLength();
    if (DashLength <= 0.0 || GapLength < 0.0)
    {
        return Mesh;
    }
    const int Count = std::max(1, static_cast<int>(std::lround(Total / (DashLength + GapLength))));
    const double Period = Total / Count;
    // Keep the dash length, shrink/grow only the gap, but never let a dash exceed the period.
    const double Dash = std::min(DashLength, Period);
    for (int i = 0; i < Count; ++i)
    {
        const double Start = Period * i;
        Mesh.Append(BuildRibbonSegment(Circuit, Start, Start + Dash, LateralOffset, Width, Z, StepMeters));
    }
    return Mesh;
}
} // namespace OpenRideCore
