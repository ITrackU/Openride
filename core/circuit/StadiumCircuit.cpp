#include "StadiumCircuit.h"

#include <cmath>
#include <stdexcept>

namespace OpenRideCore
{
namespace
{
constexpr double Pi = 3.14159265358979323846;
}

FStadiumCircuit::FStadiumCircuit(double InTotalLength, double InRadius)
    : TotalLength(InTotalLength)
    , Radius(InRadius)
    , StraightLength((InTotalLength - 2.0 * Pi * InRadius) / 2.0)
{
    if (InRadius <= 0.0 || StraightLength < 0.0)
    {
        throw std::invalid_argument("Radius must be > 0 and 2*pi*R must not exceed total length");
    }
}

FCircuitSample FStadiumCircuit::SampleAtDistance(double Distance) const
{
    double D = std::fmod(Distance, TotalLength);
    if (D < 0.0)
    {
        D += TotalLength;
    }

    const double ArcLength = Pi * Radius;
    FCircuitSample S;
    S.Distance = D;

    if (D < StraightLength) // bottom straight, heading +X
    {
        S.X = D;
        S.Y = 0.0;
        S.TangentX = 1.0;
        S.TangentY = 0.0;
        return S;
    }
    D -= StraightLength;

    if (D < ArcLength) // right semicircle, counter-clockwise from -90deg to +90deg
    {
        const double A = -Pi / 2.0 + D / Radius;
        S.X = StraightLength + Radius * std::cos(A);
        S.Y = Radius + Radius * std::sin(A);
        S.TangentX = -std::sin(A);
        S.TangentY = std::cos(A);
        return S;
    }
    D -= ArcLength;

    if (D < StraightLength) // top straight, heading -X
    {
        S.X = StraightLength - D;
        S.Y = 2.0 * Radius;
        S.TangentX = -1.0;
        S.TangentY = 0.0;
        return S;
    }
    D -= StraightLength;

    // left semicircle, counter-clockwise from +90deg to +270deg
    const double A = Pi / 2.0 + D / Radius;
    S.X = Radius * std::cos(A);
    S.Y = Radius + Radius * std::sin(A);
    S.TangentX = -std::sin(A);
    S.TangentY = std::cos(A);
    return S;
}

std::vector<FCircuitSample> FStadiumCircuit::GenerateSamples(int NumSamples) const
{
    std::vector<FCircuitSample> Out;
    if (NumSamples <= 0)
    {
        return Out;
    }
    Out.reserve(NumSamples);
    for (int i = 0; i < NumSamples; ++i)
    {
        Out.push_back(SampleAtDistance(TotalLength * i / NumSamples));
    }
    return Out;
}
} // namespace OpenRideCore
