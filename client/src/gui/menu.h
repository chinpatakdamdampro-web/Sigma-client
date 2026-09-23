#pragma once
#include <imgui.h>
#include <string>
#include <unordered_map>
#include "../modules/module_manager.h"

namespace Spectral {

// ── Palette ───────────────────────────────────────────────────────────────
namespace Colors {
    // Backgrounds
    inline constexpr ImVec4 BG_DEEP    {0.028f, 0.014f, 0.067f, 0.97f};
    inline constexpr ImVec4 BG_SIDEBAR {0.017f, 0.008f, 0.046f, 1.00f};
    inline constexpr ImVec4 BG_PANEL   {0.036f, 0.018f, 0.085f, 0.96f};
    inline constexpr ImVec4 BG_MODULE  {0.073f, 0.038f, 0.154f, 0.60f};
    inline constexpr ImVec4 BG_MOD_HOV {0.110f, 0.060f, 0.220f, 0.78f};
    inline constexpr ImVec4 BG_MOD_SEL {0.148f, 0.082f, 0.290f, 0.90f};
    inline constexpr ImVec4 BG_WIDGET  {0.050f, 0.025f, 0.110f, 0.70f};

    // Accents
    inline constexpr ImVec4 PURPLE    {0.596f, 0.000f, 1.000f, 1.00f};
    inline constexpr ImVec4 PINK      {1.000f, 0.000f, 0.596f, 1.00f};
    inline constexpr ImVec4 BLUE      {0.000f, 0.467f, 1.000f, 1.00f};
    inline constexpr ImVec4 GREEN     {0.000f, 0.910f, 0.478f, 1.00f};

    // Text
    inline constexpr ImVec4 TEXT      {0.920f, 0.855f, 1.000f, 1.00f};
    inline constexpr ImVec4 TEXT_DIM  {0.520f, 0.435f, 0.700f, 0.78f};
    inline constexpr ImVec4 TEXT_MUTE {0.330f, 0.265f, 0.480f, 0.65f};

    inline ImU32 U(const ImVec4& v, float a=1.f) {
        return IM_COL32((int)(v.x*255),(int)(v.y*255),(int)(v.z*255),(int)(v.w*a*255));
    }
    inline ImVec4 lerp(const ImVec4& a, const ImVec4& b, float t) {
        return {a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t, a.w+(b.w-a.w)*t};
    }
}

// ── Menu ──────────────────────────────────────────────────────────────────
class Menu {
public:
    static Menu& get() { static Menu inst; return inst; }

    void init();
    void applyStyle();
    void render();
    void toggle()           { m_visible = !m_visible; }
    bool isVisible() const  { return m_visible; }

private:
    bool        m_visible         = false;
    int         m_selCat          = 0;        // selected category tab index
    std::string m_selModule       = "";        // selected module name for config panel

    // Per-category hover anim (0→1)
    float m_catHover[5]   = {};
    // Per-module hover + toggle anims (keyed by module name)
    std::unordered_map<std::string, float> m_modHover;
    std::unordered_map<std::string, float> m_togAnim;

    // Layout constants
    static constexpr float W_TOTAL   = 760.f;
    static constexpr float H_TOTAL   = 500.f;
    static constexpr float W_SIDEBAR = 165.f;
    static constexpr float W_LIST    = 315.f;
    static constexpr float W_CONFIG  = W_TOTAL - W_SIDEBAR - W_LIST;  // 280
    static constexpr float H_HEADER  = 72.f;

    // Sub-renders
    void renderBackground(ImDrawList* dl, ImVec2 p);
    void renderSidebar   (ImDrawList* dl, ImVec2 p, ImVec2 io_mp);
    void renderModuleList(ImDrawList* dl, ImVec2 p, ImVec2 io_mp);
    void renderConfigPanel(ImDrawList* dl, ImVec2 p);

    // Config sub-panels
    void configHitboxes (ImVec2 origin, ImVec2 size);
    void configESP      (ImVec2 origin, ImVec2 size);
    void configSpeed    (ImVec2 origin, ImVec2 size);
    void configKillAura (ImVec2 origin, ImVec2 size);

    // Custom widgets
    void drawToggle       (ImDrawList* dl, ImVec2 pos, bool on, float& anim, bool& clicked);
    void drawGlowRect     (ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float rounding, float blur=0.f);
    void styledSlider     (const char* id, const char* label, float* v, float mn, float mx, const char* fmt="%.2f");
    void styledCheck      (const char* id, const char* label, bool* v);
    void styledColorEdit  (const char* id, const char* label, float col[4]);
    void styledCombo      (const char* id, const char* label, int* v, const char* items[], int n);
    void sectionHeader    (const char* label);

    float smoothDT() const; // clamped delta for animations
};

} // namespace Spectral
