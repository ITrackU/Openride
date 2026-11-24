#pragma once
// Engine-independent foliage scatter: jittered-grid placement of grass/bush instances in a
// rectangle, keeping a clear strip around the road. Jittered grid (not pure random) gives
// even coverage without clumps/bald patches at any density. Deterministic per seed.
// Used by AFoliageActor (Source/OpenRide/Environment), which feeds a HISM component.
#include "../appearance/AppearanceModel.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace OpenRideCore
{
    struct FPoint2
    {
        double X = 0.0, Y = 0.0; // metres
    };

    struct FFoliageInstance
    {
        double X = 0.0, Y = 0.0; // metres
        float YawDegrees = 0.f;
        float Scale = 1.f;
    };

    struct FFoliageParams
    {
        double CenterX = 0.0, CenterY = 0.0;       // metres
        double HalfExtentX = 100.0, HalfExtentY = 80.0;
        double DensityPerSqm = 2.0;                // instances per m^2 before clearance culling
        double RoadClearance = 6.0;                // keep this far from the centre-line (half road + verge)
        float MinScale = 0.8f, MaxScale = 1.3f;
        uint32_t Seed = 1;
        int MaxInstances = 200000;                 // hard cap so a bad density cannot freeze the editor
    };

    // Distance from P to the closed polyline (last point connects back to first).
    inline double DistanceToLoop(const std::vector<FPoint2>& Loop, double X, double Y)
    {
        double Best = 1e30;
        const size_t N = Loop.size();
        for (size_t I = 0; I < N; ++I)
        {
            const FPoint2& A = Loop[I];
            const FPoint2& B = Loop[(I + 1) % N];
            const double Dx = B.X - A.X, Dy = B.Y - A.Y;
            const double Len2 = Dx * Dx + Dy * Dy;
            double T = Len2 > 0.0 ? ((X - A.X) * Dx + (Y - A.Y) * Dy) / Len2 : 0.0;
            T = std::clamp(T, 0.0, 1.0);
            const double Px = A.X + T * Dx - X, Py = A.Y + T * Dy - Y;
            Best = std::min(Best, Px * Px + Py * Py);
        }
        return std::sqrt(Best);
    }

    inline std::vector<FFoliageInstance> ScatterFoliage(const FFoliageParams& P, const std::vector<FPoint2>& RoadLoop)
    {
        std::vector<FFoliageInstance> Out;
        if (P.DensityPerSqm <= 0.0 || P.HalfExtentX <= 0.0 || P.HalfExtentY <= 0.0 || P.MaxInstances <= 0)
        {
            return Out;
        }
        // One instance per cell; cell side from density.
        double Cell = 1.0 / std::sqrt(P.DensityPerSqm);
        // Enlarge the cell if the cap would be exceeded, so coverage stays even instead of truncated.
        const double Area = 4.0 * P.HalfExtentX * P.HalfExtentY;
        const double MinCell = std::sqrt(Area / static_cast<double>(P.MaxInstances));
        Cell = std::max(Cell, MinCell);

        const int NX = std::max(1, static_cast<int>(std::floor(2.0 * P.HalfExtentX / Cell)));
        const int NY = std::max(1, static_cast<int>(std::floor(2.0 * P.HalfExtentY / Cell)));
        const double CellX = 2.0 * P.HalfExtentX / NX;
        const double CellY = 2.0 * P.HalfExtentY / NY;

        FSeededRandom Rng(P.Seed);
        Out.reserve(static_cast<size_t>(NX) * NY);
        for (int J = 0; J < NY; ++J)
        {
            for (int I = 0; I < NX; ++I)
            {
                // Draw all random numbers before culling so a changed clearance does not reshuffle the rest.
                const double Jx = Rng.Next01(), Jy = Rng.Next01();
                const float Yaw = Rng.Next01() * 360.f;
                const float S = Rng.Next01();
                FFoliageInstance Inst;
                Inst.X = P.CenterX - P.HalfExtentX + (I + Jx) * CellX;
                Inst.Y = P.CenterY - P.HalfExtentY + (J + Jy) * CellY;
                if (!RoadLoop.empty() && DistanceToLoop(RoadLoop, Inst.X, Inst.Y) < P.RoadClearance)
                {
                    continue;
                }
                Inst.YawDegrees = Yaw;
                Inst.Scale = P.MinScale + (P.MaxScale - P.MinScale) * S;
                Out.push_back(Inst);
            }
        }
        return Out;
    }
}
