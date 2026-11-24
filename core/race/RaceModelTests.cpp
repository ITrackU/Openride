// Dependency-free tests for the race model. Build/run via CMakeLists.txt.
#include "RaceModel.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    // Spawn plan: count, ranges, reproducibility, distinct seeds.
    {
        FRaceSpawnParams P;
        P.Count = 10;
        const auto A = PlanSpawns(P);
        const auto B = PlanSpawns(P);
        CHECK(A.size() == 10);
        bool Same = true;
        for (size_t i = 0; i < A.size(); ++i)
        {
            CHECK(A[i].TargetSpeed >= P.MinSpeed && A[i].TargetSpeed <= P.MaxSpeed);
            CHECK(std::fabs(A[i].PreferredLane) <= P.HalfRoadWidth - P.EdgeMargin + 1e-9);
            Same = Same && A[i].StartDistance == B[i].StartDistance && A[i].AppearanceSeed == B[i].AppearanceSeed;
            for (size_t j = i + 1; j < A.size(); ++j)
            {
                CHECK(A[i].AppearanceSeed != A[j].AppearanceSeed);
                // Jitter is 30% of spacing, so start points stay at least 0.7*spacing apart.
                CHECK(std::fabs(A[i].StartDistance - A[j].StartDistance) > 1.0
                      || std::fabs(std::fabs(A[i].StartDistance - A[j].StartDistance) - P.LoopLength) > 1.0);
            }
        }
        CHECK(Same);
        CHECK(PlanSpawns(FRaceSpawnParams{0}).empty());
    }

    // Forward gap wraps round the loop.
    CHECK(std::fabs(ForwardGap(390.0, 10.0, 400.0) - 20.0) < 1e-9);
    CHECK(std::fabs(ForwardGap(10.0, 390.0, 400.0) - 380.0) < 1e-9);

    FAvoidanceParams AP;

    // Free road: keep preferred lane and speed.
    {
        FRaceRider Self{0.0, 10.0, 0.0};
        auto D = DecideAvoidance(Self, 1.0, 10.0, {}, AP);
        CHECK(D.TargetLane == 1.0 && D.TargetSpeed == 10.0);
    }

    // Slower rider ahead in same lane: move to a free lane, keep speed.
    {
        FRaceRider Self{0.0, 10.0, 0.0};
        std::vector<FRaceRider> O{{8.0, 7.0, 0.0}};
        auto D = DecideAvoidance(Self, 0.0, 10.0, O, AP);
        CHECK(std::fabs(D.TargetLane) >= AP.LaneClearance - 1e-9);
        CHECK(D.TargetSpeed == 10.0);
    }

    // Blocker across the loop seam is still seen.
    {
        FRaceRider Self{395.0, 10.0, 0.0};
        std::vector<FRaceRider> O{{3.0, 7.0, 0.0}};
        auto D = DecideAvoidance(Self, 0.0, 10.0, O, AP);
        CHECK(std::fabs(D.TargetLane) >= AP.LaneClearance - 1e-9);
    }

    // Rider ahead but already in another lane: no reaction.
    {
        FRaceRider Self{0.0, 10.0, 0.0};
        std::vector<FRaceRider> O{{8.0, 7.0, 2.0}};
        auto D = DecideAvoidance(Self, 0.0, 10.0, O, AP);
        CHECK(D.TargetLane == 0.0 && D.TargetSpeed == 10.0);
    }

    // Behind us or beyond look-ahead: ignored.
    {
        FRaceRider Self{100.0, 10.0, 0.0};
        std::vector<FRaceRider> O{{95.0, 5.0, 0.0}, {150.0, 5.0, 0.0}};
        auto D = DecideAvoidance(Self, 0.0, 10.0, O, AP);
        CHECK(D.TargetLane == 0.0 && D.TargetSpeed == 10.0);
    }

    // Boxed in (a wall of slow riders across all lanes): slow to their speed, never faster.
    {
        FRaceRider Self{0.0, 10.0, 0.0};
        std::vector<FRaceRider> O;
        for (double L = -2.2; L <= 2.2; L += 0.8) { O.push_back({6.0, 6.0, L}); }
        auto D = DecideAvoidance(Self, 0.0, 10.0, O, AP);
        CHECK(D.TargetSpeed <= 6.0 + 1e-9);
    }

    // Target lane always stays on the road.
    {
        FRaceRider Self{0.0, 10.0, 2.2};
        std::vector<FRaceRider> O{{5.0, 6.0, 2.2}};
        auto D = DecideAvoidance(Self, 2.2, 10.0, O, AP);
        CHECK(std::fabs(D.TargetLane) <= AP.HalfRoadWidth - AP.EdgeMargin + 1e-9);
    }

    if (Failures == 0) { std::printf("All race model tests passed\n"); }
    return Failures == 0 ? 0 : 1;
}
