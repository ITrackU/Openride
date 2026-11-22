// Engine-independent settings model for the settings menu: resolution list, graphics quality
// presets and key-binding table with conflict handling. Header-only so UE and CMake tests share it.
// Keys are plain strings (UE FKey names such as "W", "SpaceBar") so the core stays engine-free.
#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace OpenRideCore
{
    struct FResolution
    {
        int Width = 1280;
        int Height = 720;
        bool operator==(const FResolution& O) const { return Width == O.Width && Height == O.Height; }
    };

    // Matches UE's sg.* scalability levels 0..3 (Low..Epic); the UE side maps Level to sg.* cvars.
    enum class EQualityLevel { Low = 0, Medium = 1, High = 2, Epic = 3 };

    inline int ClampQuality(int Level) { return std::max(0, std::min(3, Level)); }

    inline const char* QualityName(EQualityLevel L)
    {
        switch (L)
        {
        case EQualityLevel::Low: return "Low";
        case EQualityLevel::Medium: return "Medium";
        case EQualityLevel::High: return "High";
        default: return "Epic";
        }
    }

    // Fixed list of offered windowed/fullscreen resolutions; the UE side may append the
    // display's native one. Kept short so the menu stays a simple cycle button.
    inline std::vector<FResolution> DefaultResolutions()
    {
        return { {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160} };
    }

    // Index of the closest offered resolution (by pixel count), so a saved value that is no
    // longer offered still maps to something sensible. Returns -1 for an empty list.
    inline int NearestResolutionIndex(const std::vector<FResolution>& List, const FResolution& Want)
    {
        int Best = -1;
        long long BestDiff = 0;
        const long long WantPx = static_cast<long long>(Want.Width) * Want.Height;
        for (size_t i = 0; i < List.size(); ++i)
        {
            const long long Px = static_cast<long long>(List[i].Width) * List[i].Height;
            const long long Diff = Px > WantPx ? Px - WantPx : WantPx - Px;
            if (Best < 0 || Diff < BestDiff) { Best = static_cast<int>(i); BestDiff = Diff; }
        }
        return Best;
    }

    struct FKeyBinding
    {
        std::string Action;   // e.g. "Accelerate"
        std::string Key;      // e.g. "W"
    };

    class FKeyBindings
    {
    public:
        static FKeyBindings Defaults()
        {
            FKeyBindings B;
            B.Bindings = { {"Accelerate", "W"}, {"Brake", "S"}, {"CycleCamera", "C"}, {"Pause", "Escape"} };
            return B;
        }

        const std::vector<FKeyBinding>& All() const { return Bindings; }

        // Empty string if the action is unknown.
        std::string KeyFor(const std::string& Action) const
        {
            for (const FKeyBinding& B : Bindings) if (B.Action == Action) return B.Key;
            return std::string();
        }

        // Rebinds Action to Key. If another action already uses Key the two swap keys, so no
        // action is ever left unbound and no key is bound twice. False if Action is unknown.
        bool Rebind(const std::string& Action, const std::string& Key)
        {
            FKeyBinding* Target = Find(Action);
            if (!Target || Key.empty()) return false;
            for (FKeyBinding& B : Bindings)
            {
                if (&B != Target && B.Key == Key) { B.Key = Target->Key; break; }
            }
            Target->Key = Key;
            return true;
        }

    private:
        FKeyBinding* Find(const std::string& Action)
        {
            for (FKeyBinding& B : Bindings) if (B.Action == Action) return &B;
            return nullptr;
        }
        std::vector<FKeyBinding> Bindings;
    };
}
