// Dependency-free tests for the menu model. Build/run via CMakeLists.txt.
#include "MenuModel.h"

#include <cstdio>

using namespace OpenRideCore;

static int Failures = 0;
#define CHECK(Cond) do { if (!(Cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #Cond); ++Failures; } } while (0)

int main()
{
    std::vector<FMenuItem> Items = { {"Resume", true}, {"Settings", false}, {"Quit", true} };

    // Wrap-around and skipping the disabled middle entry.
    CHECK(MoveSelection(Items, 0, 1) == 2);
    CHECK(MoveSelection(Items, 2, 1) == 0);
    CHECK(MoveSelection(Items, 0, -1) == 2);
    CHECK(MoveSelection(Items, 2, -1) == 0);
    Items[1].bEnabled = true;
    CHECK(MoveSelection(Items, 0, 1) == 1);
    CHECK(MoveSelection({}, 0, 1) == 0);
    std::vector<FMenuItem> None = { {"A", false} };
    CHECK(MoveSelection(None, 0, 1) == 0);

    // Layout: 3 buttons centred; middle button centre is the screen centre.
    FMenuLayout L;
    const FRect Mid = ButtonRect(L, 3, 1);
    CHECK(Mid.X + Mid.W * 0.5f == L.ScreenW * 0.5f);
    CHECK(Mid.Y + Mid.H * 0.5f == L.ScreenH * 0.5f);

    // Hit-testing: inside, outside, and disabled entries never hit.
    CHECK(HitTest(L, Items, L.ScreenW * 0.5f, L.ScreenH * 0.5f) == 1);
    CHECK(HitTest(L, Items, 1.f, 1.f) == -1);
    Items[1].bEnabled = false;
    CHECK(HitTest(L, Items, L.ScreenW * 0.5f, L.ScreenH * 0.5f) == -1);
    const FRect Last = ButtonRect(L, 3, 2);
    CHECK(HitTest(L, Items, Last.X + 1.f, Last.Y + 1.f) == 2);
    CHECK(HitTest(L, Items, Last.X + Last.W, Last.Y + 1.f) == -1); // right edge exclusive

    if (Failures == 0) std::printf("All MenuModel tests passed\n");
    return Failures == 0 ? 0 : 1;
}
