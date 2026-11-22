// Engine-independent vertical-menu model for the pause menu: selection with wrap-around that
// skips disabled entries, and pixel hit-testing for a centred column of equally sized buttons.
// Header-only so UE and CMake tests share it.
#pragma once

#include <string>
#include <vector>

namespace OpenRideCore
{
    struct FMenuItem
    {
        std::string Label;
        bool bEnabled = true;
    };

    struct FMenuLayout
    {
        float ScreenW = 1280.f;
        float ScreenH = 720.f;
        float ButtonW = 320.f;
        float ButtonH = 48.f;
        float Gap = 12.f;
    };

    struct FRect
    {
        float X = 0.f, Y = 0.f, W = 0.f, H = 0.f;
        bool Contains(float Px, float Py) const { return Px >= X && Px < X + W && Py >= Y && Py < Y + H; }
    };

    // Rect of button Index in a column of Count buttons centred on the screen.
    inline FRect ButtonRect(const FMenuLayout& L, int Count, int Index)
    {
        const float Total = static_cast<float>(Count) * L.ButtonH + (Count > 0 ? static_cast<float>(Count - 1) * L.Gap : 0.f);
        return { (L.ScreenW - L.ButtonW) * 0.5f, (L.ScreenH - Total) * 0.5f + static_cast<float>(Index) * (L.ButtonH + L.Gap),
                 L.ButtonW, L.ButtonH };
    }

    // Index of the enabled button under the point, or -1.
    inline int HitTest(const FMenuLayout& L, const std::vector<FMenuItem>& Items, float Px, float Py)
    {
        const int N = static_cast<int>(Items.size());
        for (int i = 0; i < N; ++i)
        {
            if (Items[i].bEnabled && ButtonRect(L, N, i).Contains(Px, Py)) return i;
        }
        return -1;
    }

    // Moves the selection by Step (+1/-1) with wrap-around, skipping disabled items.
    // Returns the current index unchanged if nothing is enabled.
    inline int MoveSelection(const std::vector<FMenuItem>& Items, int Current, int Step)
    {
        const int N = static_cast<int>(Items.size());
        if (N == 0 || Step == 0) return Current;
        int Idx = Current;
        for (int n = 0; n < N; ++n)
        {
            Idx = ((Idx + Step) % N + N) % N;
            if (Items[Idx].bEnabled) return Idx;
        }
        return Current;
    }
}
