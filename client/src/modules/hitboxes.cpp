#include "module_manager.h"

namespace Spectral {

void ModuleManager::init() {
    using C = Category;

    // ── Combat ─────────────────────────────────────────────────────────────
    reg("Anti Bot",        "Ignore bot players in combat targeting",         C::COMBAT);
    reg("Auto Block",      "Automatically block with sword",                  C::COMBAT);
    reg("Auto Clicker",    "Simulate rapid mouse clicks",                     C::COMBAT);
    reg("Auto Heal",       "Auto-use food when below threshold",              C::COMBAT);
    reg("Displace",        "Push entities with packet manipulation",          C::COMBAT);
    reg("Kill Aura",       "Automatically attack nearby entities",            C::COMBAT);
    reg("Lag Range",       "Extend hit-range during lag compensation",        C::COMBAT);
    reg("More Knockback",  "Increase knockback on hit",                       C::COMBAT);
    reg("Targets",         "Configure target priority list",                  C::COMBAT);
    reg("Teleport Aura",   "Teleport-based hit detection bypass",             C::COMBAT);
    reg("Velocity",        "Reduce incoming knockback",                       C::COMBAT);

    // ── Player ─────────────────────────────────────────────────────────────
    reg("Anti Debuff",     "Cancel negative potion effects",                  C::PLAYER);
    reg("Anti Fireball",   "Deflect fireballs automatically",                 C::PLAYER);
    reg("Anti Void Test",  "Prevent anti-cheat void fall tests",              C::PLAYER);
    reg("Auto Play",       "Automate basic gameplay actions",                 C::PLAYER);
    reg("Breaker",         "Instant block breaking",                          C::PLAYER);
    reg("Builder",         "Auto-place scaffold blocks",                      C::PLAYER);
    reg("Fast Interact",   "Remove interact cooldown",                        C::PLAYER);
    reg("Game Speed",      "Client-side tick speed multiplier",               C::PLAYER);
    reg("Inventory",       "Auto-sort and manage inventory",                  C::PLAYER);
    reg("No Fall",         "Cancel fall damage packets",                      C::PLAYER);
    reg("Phase",           "Phase through certain block types",               C::PLAYER);
    reg("Server Info",     "Display live server diagnostics on HUD",          C::PLAYER);

    // ── Movement ───────────────────────────────────────────────────────────
    reg("Flight",          "Enable creative-style flight",                    C::MOVEMENT);
    reg("Inventory Move",  "Allow movement with inventory open",              C::MOVEMENT);
    reg("Long Jump",       "Extend jump horizontal distance",                 C::MOVEMENT);
    reg("Movement Fix",    "Fix diagonal speed reduction",                    C::MOVEMENT);
    reg("No Jump Delay",   "Remove jump re-trigger delay",                    C::MOVEMENT);
    reg("No Slowdown",     "Remove item-use movement penalty",                C::MOVEMENT);
    reg("Scaffold",        "Automatically place blocks beneath feet",         C::MOVEMENT);
    reg("Speed",           "Increase movement speed",                         C::MOVEMENT);
    reg("Smooth",          "Interpolate movement for fluidity",               C::MOVEMENT);
    reg("Sprint",          "Auto-sprint at all times",                        C::MOVEMENT);
    reg("Step",            "Step up blocks without jumping",                  C::MOVEMENT);
    reg("Target Strafe",   "Auto-strafe around the current target",           C::MOVEMENT);
    reg("Wall Climb",      "Climb vertical surfaces",                         C::MOVEMENT);

    // ── Visual ─────────────────────────────────────────────────────────────
    reg("Ambience",        "Custom ambient sound & fog effects",              C::VISUAL);
    reg("Block Overlay",   "Tinted block selection highlight",                C::VISUAL);
    reg("Click GUI",       "Click-through GUI module browser",                C::VISUAL);
    reg("Crosshair",       "Custom crosshair styles",                         C::VISUAL);
    reg("Damage Particles","Enhanced damage particle effects",                C::VISUAL);
    reg("ESP",             "Entity outlines through walls",                   C::VISUAL);
    reg("Hitboxes",        "Render entity collision bounding boxes",          C::VISUAL);
    reg("HUD",             "Heads-up display overlay",                        C::VISUAL);
    reg("Item Animations", "Custom hand item sway animations",                C::VISUAL);
    reg("Item Physics",    "Dropped items with physics",                      C::VISUAL);
    reg("Kill Effects",    "Particle burst on kill",                          C::VISUAL);
    reg("Skeleton",        "Render entity joint skeletons",                   C::VISUAL);
    reg("Trails",          "Particle trail behind player",                    C::VISUAL);
    reg("Tweaks",          "Miscellaneous visual tweaks",                     C::VISUAL);

    // ── Exploits ───────────────────────────────────────────────────────────
    reg("Anti FBL",          "Bypass FastBreak limiter on servers",           C::EXPLOITS);
    reg("Blink",             "Hold packets then release burst",               C::EXPLOITS);
    reg("Creative Tabs",     "Access creative inventory tabs in survival",    C::EXPLOITS);
    reg("Disabler",          "Delay/drop anti-cheat check packets",           C::EXPLOITS);
    reg("Join Claim",        "Auto-claim protection on world join",           C::EXPLOITS);
    reg("More Packets",      "Send extra movement packets per tick",          C::EXPLOITS);
    reg("Player Detect",     "Locate tab-hidden players via packets",         C::EXPLOITS);
    reg("PvP Hangout Parkour","PvP region boundary exploits",                 C::EXPLOITS);
    reg("Silent Range",      "Extend reach without visible arm swing",        C::EXPLOITS);
    reg("Simulation Test",   "Probe server-side simulation limits",           C::EXPLOITS);
}

} // namespace Spectral
