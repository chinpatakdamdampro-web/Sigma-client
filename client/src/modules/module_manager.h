#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace Spectral {

// ── Hitboxes config ────────────────────────────────────────────────────────
struct HitboxConfig {
    bool  enabled       = false;
    float color[4]      = { 1.0f, 0.18f, 0.55f, 0.92f }; // hot-pink default
    float lineThickness = 1.5f;
    bool  fill          = true;
    float fillOpacity   = 0.07f;
    bool  targetPlayers = true;
    bool  targetMobs    = true;
    bool  targetAnimals = false;
    bool  targetItems   = false;
    float expandAmount  = 0.0f;
    int   style         = 0;  // 0=Default 1=Precise 2=Expanded
};

// ── ESP config ─────────────────────────────────────────────────────────────
struct EspConfig {
    bool  enabled          = false;
    float playerColor[4]   = { 0.59f, 0.0f, 1.0f, 1.0f };
    float mobColor[4]      = { 1.0f, 0.35f, 0.0f, 1.0f };
    bool  showHealth       = true;
    bool  showDistance     = true;
};

// ── Speed config ───────────────────────────────────────────────────────────
struct SpeedConfig {
    bool  enabled = false;
    float speed   = 0.28f;
    int   mode    = 0; // 0=Minbox 1=Strafe 2=BHop
};

// ── Kill Aura config ───────────────────────────────────────────────────────
struct KillAuraConfig {
    bool  enabled     = false;
    float range       = 3.5f;
    int   cps         = 12;
    bool  targetPlayers = true;
    bool  targetMobs    = true;
    bool  singleTarget  = false;
};

// ── Generic module ─────────────────────────────────────────────────────────
enum class Category { COMBAT, PLAYER, MOVEMENT, VISUAL, EXPLOITS };

struct Module {
    std::string  name;
    std::string  description;
    Category     category;
    bool         enabled  = false;
    bool         keybind  = false;
    int          key      = 0;

    Module() = default;
    Module(std::string n, std::string d, Category c)
        : name(std::move(n)), description(std::move(d)), category(c) {}
};

// ── Manager ────────────────────────────────────────────────────────────────
class ModuleManager {
public:
    static ModuleManager& get() {
        static ModuleManager inst;
        return inst;
    }

    // Per-module configs (only priority ones have dedicated structs)
    HitboxConfig  hitboxes;
    EspConfig     esp;
    SpeedConfig   speed;
    KillAuraConfig killAura;

    bool isEnabled(const std::string& name) const {
        auto it = m_map.find(name);
        return it != m_map.end() && it->second.enabled;
    }

    void setEnabled(const std::string& name, bool v) {
        auto it = m_map.find(name);
        if (it != m_map.end()) it->second.enabled = v;
        // Sync priority modules
        if (name == "Hitboxes")  hitboxes.enabled = v;
        if (name == "ESP")       esp.enabled      = v;
        if (name == "Speed")     speed.enabled     = v;
        if (name == "Kill Aura") killAura.enabled  = v;
    }

    Module* getModule(const std::string& name) {
        auto it = m_map.find(name);
        return it != m_map.end() ? &it->second : nullptr;
    }

    const std::vector<std::string>& byCategory(Category c) const { return m_cat[static_cast<int>(c)]; }

    void init();

private:
    std::unordered_map<std::string, Module> m_map;
    std::vector<std::string>                m_cat[5];

    void reg(const char* name, const char* desc, Category c) {
        m_map.emplace(name, Module{name, desc, c});
        m_cat[static_cast<int>(c)].push_back(name);
    }
};

} // namespace Spectral
