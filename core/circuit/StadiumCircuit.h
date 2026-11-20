// Engine-independent stadium-shaped closed loop (two straights + two semicircles).
// No Unreal dependencies so it can be unit-tested on the ARM dev machine and
// reused later by ACircuitActor to build its spline.
#pragma once

#include <vector>

namespace OpenRideCore
{
struct FCircuitSample
{
    double X = 0.0;        // metres, +X forward along the first straight
    double Y = 0.0;        // metres, +Y to the left of the start direction
    double TangentX = 1.0; // unit tangent (direction of travel)
    double TangentY = 0.0;
    double Distance = 0.0; // metres along the track, in [0, TotalLength)
};

class FStadiumCircuit
{
public:
    // Why radius instead of straight length: the radius fixes the bend a rider feels,
    // and the straight length is then derived so the total is exactly TotalLength.
    explicit FStadiumCircuit(double InTotalLength = 400.0, double InRadius = 30.0);

    double GetTotalLength() const { return TotalLength; }
    double GetRadius() const { return Radius; }
    double GetStraightLength() const { return StraightLength; }

    // Distance wraps around (closed loop); negative values are allowed.
    FCircuitSample SampleAtDistance(double Distance) const;

    // Evenly spaced samples (by distance) covering one lap; first sample is at 0.
    std::vector<FCircuitSample> GenerateSamples(int NumSamples) const;

private:
    double TotalLength;
    double Radius;
    double StraightLength;
};
} // namespace OpenRideCore
