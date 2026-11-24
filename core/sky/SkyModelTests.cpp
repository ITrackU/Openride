// Dependency-free tests for the day-time model. Build/run via CMakeLists.txt.
#include "SkyModel.h"

#include <cmath>
#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    // Noon: sun at max elevation, due south, bright and neutral.
    const FSkyState Noon = ComputeSky(12.0);
    CHECK(std::fabs(Noon.SunElevationDeg - 60.0) < 1e-9);
    CHECK(std::fabs(Noon.SunAzimuthDeg - 180.0) < 1e-9);
    CHECK(Noon.SunIntensity > 9.9 && !Noon.bNight);
    CHECK(Noon.SunTemperatureK > 6000.0);

    // Sunrise/sunset: on the horizon, east/west, warm.
    const FSkyState Rise = ComputeSky(6.0);
    CHECK(std::fabs(Rise.SunElevationDeg) < 1e-9);
    CHECK(std::fabs(Rise.SunAzimuthDeg - 90.0) < 1e-9);
    CHECK(Rise.SunTemperatureK < 2300.0);
    CHECK(std::fabs(ComputeSky(18.0).SunAzimuthDeg - 270.0) < 1e-9);

    // Midnight: dark but not black, exposure boosted.
    const FSkyState Night = ComputeSky(0.0);
    CHECK(Night.bNight && Night.SunIntensity == 0.0);
    CHECK(Night.SkyLightIntensity >= 0.05 && Night.SkyLightIntensity < 0.1);
    CHECK(Night.ExposureBias > 1.9);

    // Wrapping: 25h == 1h, -1h == 23h.
    CHECK(std::fabs(ComputeSky(25.0).SunElevationDeg - ComputeSky(1.0).SunElevationDeg) < 1e-9);
    CHECK(std::fabs(ComputeSky(-1.0).SunElevationDeg - ComputeSky(23.0).SunElevationDeg) < 1e-9);

    // Intensity is monotonic from sunrise to noon.
    double Prev = -1.0;
    for (double H = 5.0; H <= 12.0; H += 0.25)
    {
        const double I = ComputeSky(H).SunIntensity;
        CHECK(I >= Prev);
        Prev = I;
    }

    if (Failures == 0) std::printf("SkyModelTests: all passed\n");
    return Failures == 0 ? 0 : 1;
}
