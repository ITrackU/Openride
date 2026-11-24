// Dependency-free tests for the settings model. Build/run via CMakeLists.txt.
#include "SettingsModel.h"

#include <cstdio>
#include <set>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    CHECK(ClampQuality(-5) == 0 && ClampQuality(9) == 3 && ClampQuality(2) == 2);
    CHECK(std::string(QualityName(EQualityLevel::Epic)) == "Epic");

    const auto List = DefaultResolutions();
    CHECK(NearestResolutionIndex(List, {1920, 1080}) == 2);
    CHECK(NearestResolutionIndex(List, {1900, 1000}) == 2);
    CHECK(NearestResolutionIndex(List, {640, 480}) == 0);
    CHECK(NearestResolutionIndex(List, {8000, 4000}) == 4);
    CHECK(NearestResolutionIndex({}, {1, 1}) == -1);

    FKeyBindings B = FKeyBindings::Defaults();
    CHECK(B.KeyFor("Accelerate") == "W");
    CHECK(B.KeyFor("Nope").empty());

    // Plain rebind to a free key.
    CHECK(B.Rebind("Accelerate", "Up"));
    CHECK(B.KeyFor("Accelerate") == "Up");

    // Rebinding onto a used key swaps, keeping every key unique.
    CHECK(B.Rebind("Brake", "Up"));
    CHECK(B.KeyFor("Brake") == "Up" && B.KeyFor("Accelerate") == "S");
    std::set<std::string> Keys;
    for (const auto& K : B.All()) Keys.insert(K.Key);
    CHECK(Keys.size() == B.All().size());

    CHECK(!B.Rebind("Unknown", "X"));
    CHECK(!B.Rebind("Brake", ""));

    std::printf(Failures ? "FAILED\n" : "OK\n");
    return Failures ? 1 : 0;
}
