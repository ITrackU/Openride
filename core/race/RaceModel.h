// Engine-independent race logic: spawn planning and lane-based avoidance for NPC cyclists.
// Header-only so UE (ARaceManager) and the CMake tests share it.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "../appearance/AppearanceModel.h"

namespace OpenRideCore
{
    struct FRaceSpawnParams
    {
        int Count = 8;
        uint32_t Seed = 1;
        double MinSpeed = 7.0;          // m/s
        double MaxSpeed = 11.0;         // m/s
        double LoopLength = 400.0;      // metres
        double HalfRoadWidth = 3.0;     // metres; lanes stay inside this minus EdgeMargin
        double EdgeMargin = 0.8;        // keep riders' bodies on the asphalt
    };

    struct FRaceSpawnSlot
    {
        double StartDistance = 0.0;
        double TargetSpeed = 0.0;
        double PreferredLane = 0.0;
        uint32_t AppearanceSeed = 0;
    };

    // Start offsets are spread evenly round the loop with a small jitter (so nobody spawns inside
    // another rider), speeds and preferred lanes are random but reproducible for a given seed.
    inline std::vector<FRaceSpawnSlot> PlanSpawns(const FRaceSpawnParams& P)
    {
        std::vector<FRaceSpawnSlot> Slots;
        if (P.Count <= 0 || P.LoopLength <= 0.0)
        {
            return Slots;
        }
        FSeededRandom Rng(P.Seed);
        const double Spacing = P.LoopLength / P.Count;
        const double MaxLane = std::max(0.0, P.HalfRoadWidth - P.EdgeMargin);
        for (int i = 0; i < P.Count; ++i)
        {
            FRaceSpawnSlot S;
            S.StartDistance = Spacing * i + (Rng.Next01() - 0.5) * 0.3 * Spacing;
            S.TargetSpeed = P.MinSpeed + (P.MaxSpeed - P.MinSpeed) * Rng.Next01();
            S.PreferredLane = (Rng.Next01() * 2.0 - 1.0) * MaxLane;
            // Mix so neighbouring slots do not get correlated looks from a small seed range.
            S.AppearanceSeed = P.Seed * 2654435761u + static_cast<uint32_t>(i) * 40503u + 1u;
            Slots.push_back(S);
        }
        return Slots;
    }

    struct FRaceRider
    {
        double Distance = 0.0;      // metres along the loop (may be unwrapped)
        double Speed = 0.0;         // current m/s
        double Lane = 0.0;          // current lateral offset, metres
    };

    struct FAvoidanceParams
    {
        double LoopLength = 400.0;
        double HalfRoadWidth = 3.0;
        double EdgeMargin = 0.8;
        double LookAhead = 12.0;        // react to riders this far ahead
        double LaneClearance = 1.0;     // lateral gap needed to pass without touching
        double MinGap = 2.5;            // longitudinal gap kept when stuck behind someone
    };

    struct FAvoidanceDecision
    {
        double TargetLane = 0.0;
        double TargetSpeed = 0.0;
    };

    // Forward distance from A to B going the direction of travel, in [0, LoopLength).
    inline double ForwardGap(double From, double To, double LoopLength)
    {
        double Gap = std::fmod(To - From, LoopLength);
        if (Gap < 0.0)
        {
            Gap += LoopLength;
        }
        return Gap;
    }

    // Decides lane + speed for Self, given everyone else. If a slower rider blocks the lane
    // ahead, move to the nearest lane that is free over the look-ahead window; if no lane is
    // free, match the blocker's speed (never ride through it). With nobody blocking, return to
    // the preferred lane and speed. Targets are fed to StepNpcFollower, which smooths changes.
    inline FAvoidanceDecision DecideAvoidance(const FRaceRider& Self, double PreferredLane, double PreferredSpeed,
                                              const std::vector<FRaceRider>& Others, const FAvoidanceParams& P)
    {
        const double MaxLane = std::max(0.0, P.HalfRoadWidth - P.EdgeMargin);
        const double Preferred = std::clamp(PreferredLane, -MaxLane, MaxLane);
        FAvoidanceDecision Out{Preferred, PreferredSpeed};
        if (P.LoopLength <= 0.0)
        {
            return Out;
        }

        // A rider is "relevant" if within look-ahead in front of us.
        auto IsAhead = [&](const FRaceRider& O)
        {
            const double Gap = ForwardGap(Self.Distance, O.Distance, P.LoopLength);
            return Gap > 0.0 && Gap < P.LookAhead;
        };
        auto IsBlockedAt = [&](double Lane, const FRaceRider** Blocker)
        {
            const FRaceRider* Nearest = nullptr;
            double NearestGap = 0.0;
            for (const FRaceRider& O : Others)
            {
                if (!IsAhead(O) || std::fabs(O.Lane - Lane) >= P.LaneClearance)
                {
                    continue;
                }
                const double Gap = ForwardGap(Self.Distance, O.Distance, P.LoopLength);
                if (!Nearest || Gap < NearestGap)
                {
                    Nearest = &O;
                    NearestGap = Gap;
                }
            }
            if (Blocker) { *Blocker = Nearest; }
            return Nearest != nullptr;
        };

        const FRaceRider* Blocker = nullptr;
        if (!IsBlockedAt(Preferred, &Blocker))
        {
            // Preferred lane is free, but we may still be mid-pass in another lane: only go back
            // when that is safe too, which the check above already guarantees.
            return Out;
        }
        if (Blocker->Speed >= PreferredSpeed)
        {
            // Not catching up: no need to dodge, just don't exceed their speed inside MinGap.
            const double Gap = ForwardGap(Self.Distance, Blocker->Distance, P.LoopLength);
            if (Gap < P.MinGap) { Out.TargetSpeed = std::min(PreferredSpeed, Blocker->Speed); }
            return Out;
        }

        // Search candidate lanes outwards from the preferred one, in 0.5 m steps.
        const double Step = 0.5;
        for (double Offset = Step; Offset <= 2.0 * MaxLane + 1e-9; Offset += Step)
        {
            for (const double Sign : {1.0, -1.0})
            {
                const double Lane = Preferred + Sign * Offset;
                if (Lane < -MaxLane - 1e-9 || Lane > MaxLane + 1e-9)
                {
                    continue;
                }
                if (!IsBlockedAt(Lane, nullptr))
                {
                    Out.TargetLane = Lane;
                    return Out;
                }
            }
        }

        // Boxed in: follow the blocker.
        const double Gap = ForwardGap(Self.Distance, Blocker->Distance, P.LoopLength);
        Out.TargetSpeed = Gap < P.MinGap ? std::min(PreferredSpeed, Blocker->Speed * 0.95)
                                         : std::min(PreferredSpeed, Blocker->Speed);
        return Out;
    }
}
