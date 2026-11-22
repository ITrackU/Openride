// Dependency-free tests for the appearance randomiser. Build/run via CMakeLists.txt.
#include "AppearanceModel.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

static bool InRange(const FRgb& C)
{
    return C.R >= 0.f && C.R <= 1.f && C.G >= 0.f && C.G <= 1.f && C.B >= 0.f && C.B <= 1.f;
}

int main()
{
    // HSV primaries.
    {
        const FRgb Red = HsvToRgb(0.f, 1.f, 1.f);
        CHECK(Red.R == 1.f && Red.G == 0.f && Red.B == 0.f);
        const FRgb Green = HsvToRgb(1.f / 3.f, 1.f, 1.f);
        CHECK(Green.G > 0.99f && Green.R < 0.01f);
        const FRgb Wrapped = HsvToRgb(1.f, 1.f, 1.f); // hue 1 wraps to 0
        CHECK(Wrapped.R == 1.f && Wrapped.G == 0.f);
        const FRgb Grey = HsvToRgb(0.4f, 0.f, 0.5f);
        CHECK(Grey.R == 0.5f && Grey.G == 0.5f && Grey.B == 0.5f);
    }

    // Same seed -> same look; different seeds -> different look.
    {
        const FAppearance A = RandomAppearance(42);
        const FAppearance B = RandomAppearance(42);
        const FAppearance C = RandomAppearance(43);
        CHECK(A.Jersey.R == B.Jersey.R && A.Jersey.G == B.Jersey.G && A.Frame.B == B.Frame.B);
        CHECK(A.Jersey.R != C.Jersey.R || A.Jersey.G != C.Jersey.G || A.Jersey.B != C.Jersey.B);
    }

    // All colours valid for many seeds (including seed 0, which must not lock the PRNG).
    {
        for (uint32_t Seed = 0; Seed < 2000; ++Seed)
        {
            const FAppearance A = RandomAppearance(Seed);
            CHECK(InRange(A.Jersey) && InRange(A.Shorts) && InRange(A.Helmet) && InRange(A.Frame));
        }
        FSeededRandom Zero(0);
        CHECK(Zero.Next01() != Zero.Next01());
    }

    std::printf(Failures ? "FAILED (%d)\n" : "OK\n", Failures);
    return Failures ? 1 : 0;
}
