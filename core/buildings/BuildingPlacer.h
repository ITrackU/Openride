#pragma once
// Engine-independent building placement: walks the closed road loop, and at each step puts a
// building on the OUTSIDE of the loop, facing the road, never closer than a setback and never
// overlapping a previous building. Variants are picked by weight; sizes are random within each
// variant's range. Deterministic per seed. Used by ABuildingPlacerActor.
#include "../foliage/FoliageScatter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace OpenRideCore
{
    struct FBuildingVariantSpec
    {
        double MinWidth = 8.0, MaxWidth = 12.0;   // along the road, metres
        double MinDepth = 8.0, MaxDepth = 10.0;   // away from the road
        double MinHeight = 6.0, MaxHeight = 9.0;
        float Weight = 1.f;                        // relative pick probability
    };

    struct FBuildingParams
    {
        double Spacing = 16.0;       // distance along the loop between placement attempts
        double Setback = 12.0;       // min gap from road centre-line to nearest building edge
        double SetbackJitter = 6.0;  // extra random gap so the street line is not a ruler
        double Gap = 2.0;            // min clearance between buildings
        uint32_t Seed = 1;
        int MaxBuildings = 500;
    };

    struct FBuildingPlacement
    {
        double X = 0.0, Y = 0.0;     // footprint centre, metres
        float YawDegrees = 0.f;      // local +X (width) runs along the road
        int VariantIndex = 0;
        double Width = 0.0, Depth = 0.0, Height = 0.0;
    };

    // Positive for counter-clockwise loops (Y up).
    inline double SignedArea(const std::vector<FPoint2>& Loop)
    {
        double A = 0.0;
        for (size_t I = 0; I < Loop.size(); ++I)
        {
            const FPoint2& P = Loop[I];
            const FPoint2& Q = Loop[(I + 1) % Loop.size()];
            A += P.X * Q.Y - Q.X * P.Y;
        }
        return 0.5 * A;
    }

    inline std::vector<FBuildingPlacement> PlaceBuildings(const FBuildingParams& P,
        const std::vector<FBuildingVariantSpec>& Variants, const std::vector<FPoint2>& Loop)
    {
        std::vector<FBuildingPlacement> Out;
        if (Loop.size() < 3 || Variants.empty() || P.Spacing <= 0.0 || P.MaxBuildings <= 0)
        {
            return Out;
        }
        double TotalWeight = 0.0;
        for (const auto& V : Variants) { TotalWeight += std::max(0.f, V.Weight); }
        if (TotalWeight <= 0.0)
        {
            return Out;
        }
        // Outward normal = right of travel for CCW loops, left for CW.
        const double Side = SignedArea(Loop) >= 0.0 ? -1.0 : 1.0; // sign applied to the left normal (-Ty, Tx)

        FSeededRandom Rng(P.Seed);
        auto Lerp = [&](double A, double B) { return A + (B - A) * Rng.Next01(); };

        double Carry = 0.0; // distance already walked into the current segment's successor
        const size_t N = Loop.size();
        for (size_t I = 0; I < N && static_cast<int>(Out.size()) < P.MaxBuildings; ++I)
        {
            const FPoint2& A = Loop[I];
            const FPoint2& B = Loop[(I + 1) % N];
            const double Dx = B.X - A.X, Dy = B.Y - A.Y;
            const double Len = std::sqrt(Dx * Dx + Dy * Dy);
            if (Len <= 0.0) { continue; }
            const double Tx = Dx / Len, Ty = Dy / Len;
            const double Nx = -Ty * Side, Ny = Tx * Side;

            for (double D = Carry; D < Len && static_cast<int>(Out.size()) < P.MaxBuildings; D += P.Spacing)
            {
                // Draw every random number up front so rejecting one building does not reshuffle the rest.
                double Pick = Rng.Next01() * TotalWeight;
                const double Wr = Rng.Next01(), Dr = Rng.Next01(), Hr = Rng.Next01(), Jr = Rng.Next01();
                int Idx = 0;
                for (size_t V = 0; V < Variants.size(); ++V)
                {
                    Pick -= std::max(0.f, Variants[V].Weight);
                    if (Pick < 0.0) { Idx = static_cast<int>(V); break; }
                }
                const FBuildingVariantSpec& Spec = Variants[Idx];

                FBuildingPlacement B2;
                B2.VariantIndex = Idx;
                B2.Width = Spec.MinWidth + (Spec.MaxWidth - Spec.MinWidth) * Wr;
                B2.Depth = Spec.MinDepth + (Spec.MaxDepth - Spec.MinDepth) * Dr;
                B2.Height = Spec.MinHeight + (Spec.MaxHeight - Spec.MinHeight) * Hr;
                const double Off = P.Setback + P.SetbackJitter * Jr + B2.Depth * 0.5;
                B2.X = A.X + Tx * D + Nx * Off;
                B2.Y = A.Y + Ty * D + Ny * Off;
                B2.YawDegrees = static_cast<float>(std::atan2(Ty, Tx) * 180.0 / 3.14159265358979323846);

                // Road clearance: the nearest corner (worst case) must respect the setback, which also
                // protects against the opposite straight / curve when the loop is tight.
                const double Hw = B2.Width * 0.5, Hd = B2.Depth * 0.5;
                bool Ok = true;
                for (int Cx = -1; Cx <= 1 && Ok; Cx += 2)
                {
                    for (int Cy = -1; Cy <= 1 && Ok; Cy += 2)
                    {
                        const double Lx = Cx * Hw, Ly = Cy * Hd;
                        const double Wx = B2.X + Tx * Lx - Ty * Ly;
                        const double Wy = B2.Y + Ty * Lx + Tx * Ly;
                        Ok = DistanceToLoop(Loop, Wx, Wy) >= P.Setback;
                    }
                }
                // Conservative overlap test with bounding circles (cheap and safe; may leave slightly larger gaps).
                const double R = std::sqrt(Hw * Hw + Hd * Hd);
                for (size_t K = 0; K < Out.size() && Ok; ++K)
                {
                    const FBuildingPlacement& O = Out[K];
                    const double Ro = 0.5 * std::sqrt(O.Width * O.Width + O.Depth * O.Depth);
                    const double Ex = O.X - B2.X, Ey = O.Y - B2.Y;
                    Ok = std::sqrt(Ex * Ex + Ey * Ey) >= R + Ro + P.Gap;
                }
                if (Ok) { Out.push_back(B2); }
            }
            // Keep spacing continuous across segment boundaries.
            const double Steps = std::ceil((Len - Carry) / P.Spacing);
            Carry = Carry + Steps * P.Spacing - Len;
        }
        return Out;
    }
}
