#include "menu.h"
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace Spectral {

using namespace Colors;

// ── Category metadata ─────────────────────────────────────────────────────
struct CatMeta { const char* name; const char* icon; ImVec4 accent; };
static const CatMeta CATS[5] = {
    { "Combat",   "⚔",  {1.00f, 0.30f, 0.45f, 1.f} },
    { "Player",   "👤", {0.60f, 0.00f, 1.00f, 1.f} },
    { "Movement", "💨", {0.00f, 0.70f, 1.00f, 1.f} },
    { "Visual",   "👁",  {0.00f, 0.91f, 0.48f, 1.f} },
    { "Exploits", "⚡", {1.00f, 0.60f, 0.00f, 1.f} },
};
static const Category CAT_ENUM[5] = {
    Category::COMBAT, Category::PLAYER, Category::MOVEMENT,
    Category::VISUAL, Category::EXPLOITS
};

// ── Helpers ───────────────────────────────────────────────────────────────
static float clamp01(float v) { return v < 0.f ? 0.f : v > 1.f ? 1.f : v; }
static float easeOut(float t)  { return 1.f - (1.f - t)*(1.f - t); }

float Menu::smoothDT() const {
    float dt = ImGui::GetIO().DeltaTime;
    return dt > 0.1f ? 0.1f : dt;
}

// Lerp animation toward target at speed
static void animTo(float& cur, float target, float speed, float dt) {
    cur += (target - cur) * clamp01(speed * dt);
}

// ── Style ─────────────────────────────────────────────────────────────────
void Menu::applyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding    = 18.f;
    s.FrameRounding     = 8.f;
    s.GrabRounding      = 8.f;
    s.PopupRounding     = 12.f;
    s.ScrollbarRounding = 8.f;
    s.ChildRounding     = 10.f;
    s.TabRounding       = 8.f;
    s.WindowPadding     = {0.f, 0.f};
    s.FramePadding      = {8.f, 4.f};
    s.ItemSpacing       = {8.f, 6.f};
    s.ScrollbarSize     = 5.f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]           = BG_DEEP;
    c[ImGuiCol_ChildBg]            = {0.f,0.f,0.f,0.f};
    c[ImGuiCol_FrameBg]            = BG_WIDGET;
    c[ImGuiCol_FrameBgHovered]     = {0.12f,0.06f,0.24f,0.72f};
    c[ImGuiCol_FrameBgActive]      = {0.16f,0.09f,0.32f,0.90f};
    c[ImGuiCol_SliderGrab]         = PURPLE;
    c[ImGuiCol_SliderGrabActive]   = PINK;
    c[ImGuiCol_CheckMark]          = PURPLE;
    c[ImGuiCol_ScrollbarBg]        = {0.f,0.f,0.f,0.f};
    c[ImGuiCol_ScrollbarGrab]      = {0.36f,0.00f,0.60f,0.40f};
    c[ImGuiCol_ScrollbarGrabHover] = {0.50f,0.00f,0.85f,0.60f};
    c[ImGuiCol_PopupBg]            = {0.05f,0.02f,0.12f,0.97f};
    c[ImGuiCol_Border]             = {0.45f,0.00f,0.80f,0.18f};
    c[ImGuiCol_Text]               = TEXT;
    c[ImGuiCol_TextDisabled]       = TEXT_MUTE;
}

// ── Init ──────────────────────────────────────────────────────────────────
void Menu::init() {
    applyStyle();
    ModuleManager::get().init();
}

// ── Main render ───────────────────────────────────────────────────────────
void Menu::render() {
    if (!m_visible) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = {io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f};

    ImGui::SetNextWindowSize({W_TOTAL, H_TOTAL}, ImGuiCond_Always);
    ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, {0.5f, 0.5f});

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBringToDisplayFront;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 20.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f,0.f});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, BG_DEEP);

    if (ImGui::Begin("##spectral_main", nullptr, flags)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 wp     = ImGui::GetWindowPos();
        ImVec2 mp     = io.MousePos;

        renderBackground(dl, wp);
        renderSidebar   (dl, wp, mp);
        renderModuleList(dl, wp, mp);
        renderConfigPanel(dl, wp);
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// ── Background ────────────────────────────────────────────────────────────
void Menu::renderBackground(ImDrawList* dl, ImVec2 p) {
    // Gradient base
    dl->AddRectFilledMultiColor(
        p, {p.x+W_TOTAL, p.y+H_TOTAL},
        IM_COL32(7,3,18,248), IM_COL32(11,5,26,248),
        IM_COL32(5,2,14,248), IM_COL32(9,4,22,248)
    );

    // Ambient glow orbs (simulate background blur)
    dl->AddCircleFilled({p.x+90,  p.y+80},  220, IM_COL32(140,0,240,10));
    dl->AddCircleFilled({p.x+680, p.y+420}, 180, IM_COL32(255,0,140,8));
    dl->AddCircleFilled({p.x+400, p.y+250}, 140, IM_COL32(0,100,255,6));

    // Top accent line
    float t = (float)ImGui::GetTime();
    float pulse = 0.55f + 0.35f * sinf(t * 1.4f);
    dl->AddRectFilled(
        {p.x+W_SIDEBAR, p.y}, {p.x+W_TOTAL, p.y+1.f},
        IM_COL32(155,0,255,(int)(pulse*90))
    );

    // Sidebar divider
    dl->AddRectFilled(
        {p.x+W_SIDEBAR-1.f, p.y+10.f},
        {p.x+W_SIDEBAR,    p.y+H_TOTAL-10.f},
        IM_COL32(100,0,180,40)
    );

    // Module/config panel divider
    dl->AddRectFilled(
        {p.x+W_SIDEBAR+W_LIST-1.f, p.y+10.f},
        {p.x+W_SIDEBAR+W_LIST,    p.y+H_TOTAL-10.f},
        IM_COL32(100,0,180,30)
    );

    // Outer glow border
    dl->AddRect({p.x,p.y}, {p.x+W_TOTAL, p.y+H_TOTAL},
        IM_COL32(130,0,220,45), 20.f, 0, 1.2f);
}

// ── Sidebar ───────────────────────────────────────────────────────────────
void Menu::renderSidebar(ImDrawList* dl, ImVec2 p, ImVec2 mp) {
    float dt = smoothDT();

    // Sidebar background
    dl->AddRectFilled(p, {p.x+W_SIDEBAR, p.y+H_TOTAL}, U(BG_SIDEBAR), 20.f,
        ImDrawFlags_RoundCornersLeft);

    // Logo
    float lcy = p.y + H_HEADER * 0.5f;
    // Hex icon
    dl->AddRectFilled({p.x+14, lcy-18}, {p.x+50, lcy+18},
        IM_COL32(90,0,160,55), 10.f);
    dl->AddRect({p.x+14, lcy-18}, {p.x+50, lcy+18},
        IM_COL32(155,0,255,90), 10.f, 0, 1.f);
    ImGui::SetCursorScreenPos({p.x+20.f, lcy-10.f});
    ImGui::TextColored(PURPLE, "⬡");

    // Title
    ImGui::SetCursorScreenPos({p.x+56.f, lcy-14.f});
    ImGui::TextColored(TEXT, "Spectral");
    ImGui::SetCursorScreenPos({p.x+56.f, lcy+1.f});
    ImGui::TextColored(TEXT_MUTE, "v1.0  1.21.1");

    // Category buttons
    for (int i = 0; i < 5; i++) {
        float bx = p.x + 10.f;
        float by = p.y + H_HEADER + 8.f + i * 52.f;
        float bw = W_SIDEBAR - 20.f;
        float bh = 42.f;

        bool hov = mp.x >= bx && mp.x <= bx+bw && mp.y >= by && mp.y <= by+bh;
        animTo(m_catHover[i], hov ? 1.f : 0.f, 10.f, dt);

        bool active = m_selCat == i;
        float t = active ? 1.f : easeOut(m_catHover[i]);
        ImVec4 accent = CATS[i].accent;

        // Background
        if (t > 0.01f) {
            ImVec4 bg = lerp(BG_SIDEBAR, {accent.x*.25f, accent.y*.25f, accent.z*.25f, .85f}, t);
            dl->AddRectFilled({bx,by},{bx+bw,by+bh}, U(bg), 10.f);
        }

        // Active left bar
        if (active) {
            dl->AddRectFilled({bx,by+6.f},{bx+3.f,by+bh-6.f},
                U(accent), 2.f);
            // Glow behind bar
            dl->AddRectFilled({bx,by+6.f},{bx+14.f,by+bh-6.f},
                IM_COL32((int)(accent.x*255),(int)(accent.y*255),(int)(accent.z*255),28));
        }

        // Icon
        ImGui::SetCursorScreenPos({bx+10.f, by+12.f});
        ImGui::TextColored(active ? accent : TEXT_DIM, "%s", CATS[i].icon);

        // Label
        ImGui::SetCursorScreenPos({bx+34.f, by+14.f});
        ImGui::TextColored(active ? TEXT : TEXT_DIM, "%s", CATS[i].name);

        // Invisible click target
        ImGui::SetCursorScreenPos({bx, by});
        ImGui::InvisibleButton(("##cat"+std::to_string(i)).c_str(), {bw, bh});
        if (ImGui::IsItemClicked()) m_selCat = i;
    }

    // Bottom info
    ImGui::SetCursorScreenPos({p.x+12.f, p.y+H_TOTAL-24.f});
    ImGui::TextColored(TEXT_MUTE, "INSERT  toggle");
}

// ── Module list ───────────────────────────────────────────────────────────
void Menu::renderModuleList(ImDrawList* dl, ImVec2 p, ImVec2 mp) {
    float dt  = smoothDT();
    float lx  = p.x + W_SIDEBAR + 8.f;
    float lw  = W_LIST - 16.f;

    // Search bar area (decorative only — fully functional if you wire ImGui::InputText)
    dl->AddRectFilled({lx, p.y+10.f},{lx+lw, p.y+42.f},
        IM_COL32(20,10,45,120), 9.f);
    dl->AddRect({lx, p.y+10.f},{lx+lw, p.y+42.f},
        IM_COL32(100,0,180,40), 9.f, 0, 1.f);
    ImGui::SetCursorScreenPos({lx+10.f, p.y+22.f});
    ImGui::TextColored(TEXT_MUTE, "🔍  Search modules…");

    // Scrollable child
    ImGui::SetCursorScreenPos({lx, p.y + 50.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0,0,0,0));
    ImGui::BeginChild("##modlist", {lw, H_TOTAL - 60.f}, false,
        ImGuiWindowFlags_NoScrollbar);

    auto& mm = ModuleManager::get();
    const auto& names = mm.byCategory(CAT_ENUM[m_selCat]);
    float my = ImGui::GetCursorScreenPos().y;

    for (const auto& name : names) {
        Module* mod = mm.getModule(name);
        if (!mod) continue;

        float bx   = ImGui::GetCursorScreenPos().x;
        float by   = my;
        my        += 46.f;
        float bw   = lw;
        float bh   = 40.f;

        bool hov = mp.x >= bx && mp.x <= bx+bw && mp.y >= by && mp.y <= by+bh
                   && m_visible;
        bool sel = (m_selModule == name);

        auto& ha = m_modHover[name];
        animTo(ha, hov ? 1.f : 0.f, 10.f, dt);

        // Background
        ImVec4 bg = sel
            ? BG_MOD_SEL
            : lerp(BG_MODULE, BG_MOD_HOV, easeOut(ha));
        dl->AddRectFilled({bx,by},{bx+bw,by+bh}, U(bg), 9.f);

        // Left accent line when enabled
        if (mod->enabled) {
            dl->AddRectFilled({bx,by+5.f},{bx+2.5f,by+bh-5.f},
                U(CATS[m_selCat].accent), 2.f);
        }

        // Module name
        ImVec4 namecol = mod->enabled ? TEXT : TEXT_DIM;
        ImGui::SetCursorScreenPos({bx+12.f, by+7.f});
        ImGui::TextColored(namecol, "%s", name.c_str());

        // Description (small)
        ImGui::SetCursorScreenPos({bx+12.f, by+23.f});
        ImGui::TextColored(TEXT_MUTE, "%s", mod->description.c_str());

        // Toggle switch
        auto& ta = m_togAnim[name];
        float tgt = mod->enabled ? 1.f : 0.f;
        animTo(ta, tgt, 12.f, dt);

        bool togClicked = false;
        ImVec2 tpos = {bx + bw - 52.f, by + bh*0.5f - 10.f};
        drawToggle(dl, tpos, mod->enabled, ta, togClicked);
        if (togClicked) {
            mm.setEnabled(name, !mod->enabled);
        }

        // Invisible click for selection (whole row, excluding toggle area)
        ImGui::SetCursorScreenPos({bx, by});
        ImGui::InvisibleButton(("##mod_"+name).c_str(), {bw - 58.f, bh});
        if (ImGui::IsItemClicked()) {
            m_selModule = (m_selModule == name) ? "" : name;
        }

        // Hover glow
        if (ha > 0.02f) {
            dl->AddRect({bx,by},{bx+bw,by+bh},
                IM_COL32(140,0,240,(int)(ha*30)), 9.f, 0, 1.f);
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ── Config panel ──────────────────────────────────────────────────────────
void Menu::renderConfigPanel(ImDrawList* dl, ImVec2 p) {
    float cx  = p.x + W_SIDEBAR + W_LIST;
    float cy  = p.y;
    float cw  = W_CONFIG;
    float ch  = H_TOTAL;

    // Panel header
    dl->AddRectFilled({cx,cy},{cx+cw,cy+48.f}, IM_COL32(15,6,38,200));

    Module* mod = m_selModule.empty() ? nullptr
                 : ModuleManager::get().getModule(m_selModule);

    if (!mod) {
        // Placeholder
        ImGui::SetCursorScreenPos({cx + cw*0.5f - 60.f, cy + ch*0.5f - 20.f});
        ImGui::TextColored(TEXT_MUTE, "Select a module");
        ImGui::SetCursorScreenPos({cx + cw*0.5f - 75.f, cy + ch*0.5f + 2.f});
        ImGui::TextColored(TEXT_MUTE, "to configure it here.");
        return;
    }

    // Header: module name
    ImGui::SetCursorScreenPos({cx+14.f, cy+10.f});
    ImGui::TextColored(TEXT, "%s", mod->name.c_str());
    ImGui::SetCursorScreenPos({cx+14.f, cy+27.f});
    ImGui::TextColored(TEXT_MUTE, "%s", mod->description.c_str());

    // Scrollable config area
    ImGui::SetCursorScreenPos({cx+8.f, cy+54.f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0,0,0,0));
    ImGui::BeginChild("##cfg", {cw-16.f, ch-62.f}, false,
        ImGuiWindowFlags_NoScrollbar);

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 size   = {cw-16.f, ch-62.f};

    // Route to the right config panel
    if      (mod->name == "Hitboxes")  configHitboxes (origin, size);
    else if (mod->name == "ESP")       configESP      (origin, size);
    else if (mod->name == "Speed")     configSpeed    (origin, size);
    else if (mod->name == "Kill Aura") configKillAura (origin, size);
    else {
        ImGui::SetCursorScreenPos({origin.x+8.f, origin.y+12.f});
        ImGui::TextColored(TEXT_MUTE, "No configuration options.");
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ── Hitboxes config ───────────────────────────────────────────────────────
void Menu::configHitboxes(ImVec2 o, ImVec2 sz) {
    auto& cfg = ModuleManager::get().hitboxes;

    sectionHeader("Color & Line");
    styledColorEdit("##hb_col",   "Box color",      cfg.color);
    styledSlider   ("##hb_thick", "Line thickness", &cfg.lineThickness, 0.5f, 5.0f, "%.1f px");

    sectionHeader("Fill");
    styledCheck("##hb_fill",     "Enable fill",   &cfg.fill);
    if (cfg.fill) {
        styledSlider("##hb_fopa", "Fill opacity", &cfg.fillOpacity, 0.0f, 0.5f, "%.2f");
    }

    sectionHeader("Targets");
    styledCheck("##hb_tp", "Players",  &cfg.targetPlayers);
    styledCheck("##hb_tm", "Mobs",     &cfg.targetMobs);
    styledCheck("##hb_ta", "Animals",  &cfg.targetAnimals);
    styledCheck("##hb_ti", "Items",    &cfg.targetItems);

    sectionHeader("Size & Style");
    styledSlider("##hb_exp", "Expand",  &cfg.expandAmount, -0.25f, 0.75f, "%+.2f m");

    const char* styles[] = { "Default", "Precise", "Expanded" };
    styledCombo("##hb_sty", "Box style", &cfg.style, styles, 3);
}

// ── ESP config ────────────────────────────────────────────────────────────
void Menu::configESP(ImVec2 o, ImVec2 sz) {
    auto& cfg = ModuleManager::get().esp;
    sectionHeader("Colors");
    styledColorEdit("##esp_pc", "Player color", cfg.playerColor);
    styledColorEdit("##esp_mc", "Mob color",    cfg.mobColor);
    sectionHeader("Display");
    styledCheck("##esp_hp",  "Show health",   &cfg.showHealth);
    styledCheck("##esp_dst", "Show distance", &cfg.showDistance);
}

// ── Speed config ──────────────────────────────────────────────────────────
void Menu::configSpeed(ImVec2 o, ImVec2 sz) {
    auto& cfg = ModuleManager::get().speed;
    sectionHeader("Speed");
    styledSlider("##spd_v", "Multiplier", &cfg.speed, 0.1f, 2.0f, "%.2f x");
    const char* modes[] = { "Minbox", "Strafe", "BHop" };
    styledCombo("##spd_m", "Mode", &cfg.mode, modes, 3);
}

// ── Kill Aura config ──────────────────────────────────────────────────────
void Menu::configKillAura(ImVec2 o, ImVec2 sz) {
    auto& cfg = ModuleManager::get().killAura;
    sectionHeader("Combat");
    styledSlider("##ka_rng", "Range",   &cfg.range, 2.5f, 6.5f, "%.1f m");
    styledSlider("##ka_cps", "CPS",     (float*)&cfg.cps, 1.f, 20.f, "%.0f");
    sectionHeader("Targets");
    styledCheck("##ka_tp", "Players", &cfg.targetPlayers);
    styledCheck("##ka_tm", "Mobs",    &cfg.targetMobs);
    sectionHeader("Misc");
    styledCheck("##ka_st", "Single target", &cfg.singleTarget);
}

// ── Custom widgets ────────────────────────────────────────────────────────
void Menu::drawToggle(ImDrawList* dl, ImVec2 pos, bool on, float& anim, bool& clicked) {
    float tw = 38.f, th = 20.f;
    ImVec2 a = pos, b = {pos.x+tw, pos.y+th};

    // Track bg color
    ImU32 bgOn  = IM_COL32(140,0,240,220);
    ImU32 bgOff = IM_COL32(35,18,68,200);
    ImU32 bg    = IM_COL32(
        (int)(18 + anim*(140-18)),
        0,
        (int)(40 + anim*(240-40)),
        200
    );
    dl->AddRectFilled(a, b, bg, th*.5f);

    // Glow when on
    if (anim > 0.05f) {
        dl->AddRectFilled(a, b, IM_COL32(140,0,240,(int)(anim*35)), th*.5f);
    }

    // Knob
    float knobX = pos.x + 2.f + (tw - th) * anim;
    float knobR = th*.5f - 2.f;
    dl->AddCircleFilled({knobX + knobR + 2.f, pos.y + th*.5f},
        knobR, IM_COL32(240,230,255,255));

    // Click detection
    ImGui::SetCursorScreenPos(a);
    ImGui::InvisibleButton(("##tg_"+std::to_string((uintptr_t)&anim)).c_str(), {tw, th});
    clicked = ImGui::IsItemClicked();
}

void Menu::sectionHeader(const char* label) {
    ImGui::Spacing();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cp = ImGui::GetCursorScreenPos();
    ImGui::TextColored(PURPLE, "%s", label);
    // Underline
    float tw = ImGui::CalcTextSize(label).x;
    dl->AddLine({cp.x, cp.y+18.f},{cp.x+tw, cp.y+18.f},
        IM_COL32(140,0,240,60), 1.f);
    ImGui::Spacing();
}

void Menu::styledSlider(const char* id, const char* label, float* v, float mn, float mx, const char* fmt) {
    ImGui::TextColored(TEXT_DIM, "%s", label);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::SliderFloat(id, v, mn, mx, fmt);
}

void Menu::styledCheck(const char* id, const char* label, bool* v) {
    ImGui::PushStyleColor(ImGuiCol_CheckMark, PURPLE);
    ImGui::Checkbox((std::string(label)+"##"+id).c_str(), v);
    ImGui::PopStyleColor();
}

void Menu::styledColorEdit(const char* id, const char* label, float col[4]) {
    ImGui::TextColored(TEXT_DIM, "%s", label);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::ColorEdit4(id, col, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar);
}

void Menu::styledCombo(const char* id, const char* label, int* v, const char* items[], int n) {
    ImGui::TextColored(TEXT_DIM, "%s", label);
    ImGui::SetNextItemWidth(-1.f);
    const char* cur = (*v >= 0 && *v < n) ? items[*v] : "…";
    if (ImGui::BeginCombo(id, cur)) {
        for (int i = 0; i < n; i++) {
            bool sel = (*v == i);
            if (ImGui::Selectable(items[i], sel)) *v = i;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

} // namespace Spectral
