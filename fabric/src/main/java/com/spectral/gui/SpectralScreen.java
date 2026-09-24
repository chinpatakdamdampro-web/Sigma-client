package com.spectral.gui;

import com.spectral.NativeBridge;
import net.minecraft.client.gui.DrawContext;
import net.minecraft.client.gui.screen.Screen;
import net.minecraft.text.Text;

import java.util.*;

/**
 * Spectral Client in-game GUI.
 *
 * Opens when the keybind is pressed (default: INSERT, rebindable in Controls).
 * Works on all platforms — uses Java/Minecraft rendering, no C++ required.
 * When the native bridge IS loaded, module toggles sync to the C++ side too.
 */
public class SpectralScreen extends Screen {

    // ── Module registry ───────────────────────────────────────────────────────
    // Java-side module state — used when native bridge isn't loaded, and as
    // the source of truth for the GUI regardless.

    private static final Map<String, List<String>> CATEGORY_MODULES = new LinkedHashMap<>();
    private static final Map<String, Boolean>      MODULE_STATES    = new HashMap<>();

    static {
        CATEGORY_MODULES.put("Combat", Arrays.asList(
            "Anti Bot","Auto Block","Auto Clicker","Auto Heal","Displace",
            "Kill Aura","Lag Range","More Knockback","Targets","Teleport Aura","Velocity"
        ));
        CATEGORY_MODULES.put("Player", Arrays.asList(
            "Anti Debuff","Anti Fireball","Anti Void Test","Auto Play","Breaker",
            "Builder","Fast Interact","Game Speed","Inventory","No Fall","Phase","Server Info"
        ));
        CATEGORY_MODULES.put("Movement", Arrays.asList(
            "Flight","Inventory Move","Long Jump","Movement Fix","No Jump Delay",
            "No Slowdown","Scaffold","Speed","Smooth","Sprint","Step","Target Strafe","Wall Climb"
        ));
        CATEGORY_MODULES.put("Visual", Arrays.asList(
            "Ambience","Block Overlay","Click GUI","Crosshair","Damage Particles",
            "ESP","Hitboxes","HUD","Item Animations","Item Physics","Kill Effects",
            "Skeleton","Trails","Tweaks"
        ));
        CATEGORY_MODULES.put("Exploits", Arrays.asList(
            "Anti FBL","Blink","Creative Tabs","Disabler","Join Claim","More Packets",
            "Player Detect","PvP Hangout Parkour","Silent Range","Simulation Test"
        ));

        for (List<String> mods : CATEGORY_MODULES.values())
            for (String m : mods)
                MODULE_STATES.put(m, false);
    }

    /**
     * Called by SpectralClient's render hook.
     * Prefers native bridge state if available, falls back to Java state.
     */
    public static boolean isModuleActive(String name) {
        if (NativeBridge.isLoaded())
            return NativeBridge.isModuleEnabled(name.toLowerCase().replace(" ", "_"));
        return MODULE_STATES.getOrDefault(name, false);
    }

    // ── Layout constants ──────────────────────────────────────────────────────
    private static final int W  = 760;
    private static final int H  = 480;
    private static final int SW = 160;   // sidebar width
    private static final int LW = 215;   // module list width
    // config panel = W - SW - LW = 385px

    // ── Colors (ARGB) ─────────────────────────────────────────────────────────
    private static final int C_BG     = 0xF2100B1E;
    private static final int C_SIDE   = 0xF2150E28;
    private static final int C_PANEL  = 0xF2120D22;
    private static final int C_PURPLE = 0xFF8B5CF6;
    private static final int C_PINK   = 0xFFEC4899;
    private static final int C_TEXT   = 0xFFE2E8F0;
    private static final int C_DIM    = 0xFF94A3B8;
    private static final int C_MUTE   = 0xFF475569;
    private static final int C_DIV    = 0x30FFFFFF;
    private static final int C_HOV    = 0x18FFFFFF;
    private static final int C_SEL    = 0x28C084FC;

    // ── State ─────────────────────────────────────────────────────────────────
    private final String[] catNames = CATEGORY_MODULES.keySet().toArray(new String[0]);
    private int    selCat  = 0;
    private String selMod  = null;
    private int    scrollY = 0;

    // Panel top-left in screen space
    private int px, py;

    // ── Constructor ───────────────────────────────────────────────────────────

    public SpectralScreen() {
        super(Text.empty());
    }

    @Override
    public boolean shouldPause() {
        return false;   // keep the game running while the menu is open
    }

    @Override
    protected void init() {
        px = (width  - W) / 2;
        py = (height - H) / 2;
    }

    // ── Render ────────────────────────────────────────────────────────────────

    @Override
    public void render(DrawContext ctx, int mx, int my, float delta) {
        // World-dimming overlay
        ctx.fill(0, 0, width, height, 0x88000000);

        // ── Panel background ─────────────────────────────────────────────────
        ctx.fill(px, py, px+W, py+H, C_BG);

        // Thin top accent line (purple → pink)
        ctx.fill(px,        py, px+W/2,    py+2, C_PURPLE);
        ctx.fill(px+W/2,    py, px+W,      py+2, C_PINK);

        // ── Sidebar ──────────────────────────────────────────────────────────
        ctx.fill(px, py, px+SW, py+H, C_SIDE);
        ctx.fill(px+SW, py, px+SW+1, py+H, C_DIV);

        // Logo
        ctx.drawText(textRenderer, "✦ SPECTRAL", px+14, py+14, C_PURPLE, true);
        ctx.drawText(textRenderer, "v1.0.0", px+14, py+26, C_MUTE, false);
        ctx.fill(px+10, py+42, px+SW-10, py+43, C_DIV);

        // Category buttons
        for (int i = 0; i < catNames.length; i++) {
            renderCategoryButton(ctx, i, mx, my);
        }

        // ── Module list ───────────────────────────────────────────────────────
        renderModuleList(ctx, mx, my);
        ctx.fill(px+SW+LW, py, px+SW+LW+1, py+H, C_DIV);

        // ── Config panel ──────────────────────────────────────────────────────
        renderConfigPanel(ctx, mx, my);

        // ── Footer ───────────────────────────────────────────────────────────
        ctx.fill(px, py+H-20, px+W, py+H-19, C_DIV);
        ctx.drawText(textRenderer, "INSERT / ESC to close  •  Click module to toggle",
            px+10, py+H-13, C_MUTE, false);

        String nativeTxt = NativeBridge.isLoaded() ? "● Native" : "● Java-only";
        int    nativeCol = NativeBridge.isLoaded() ? C_PURPLE : 0xFFEF4444;
        int    nativeX   = px + W - textRenderer.getWidth(nativeTxt) - 10;
        ctx.drawText(textRenderer, nativeTxt, nativeX, py+H-13, nativeCol, false);

        super.render(ctx, mx, my, delta);
    }

    private void renderCategoryButton(DrawContext ctx, int i, int mx, int my) {
        int bx = px+8, by = py+52 + i*44;
        int bw = SW-16, bh = 36;
        boolean active  = i == selCat;
        boolean hovered = mx>=bx && mx<=bx+bw && my>=by && my<=by+bh;

        if (active)       ctx.fill(bx, by, bx+bw, by+bh, C_SEL);
        else if (hovered) ctx.fill(bx, by, bx+bw, by+bh, C_HOV);

        if (active) ctx.fill(bx, by+4, bx+3, by+bh-4, C_PURPLE);

        int col = active ? C_TEXT : (hovered ? C_TEXT : C_DIM);
        ctx.drawText(textRenderer, catNames[i], bx+12, by+12, col, false);

        // Badge: count of enabled modules in this category
        long enabled = CATEGORY_MODULES.get(catNames[i]).stream()
            .filter(m -> MODULE_STATES.getOrDefault(m, false)).count();
        if (enabled > 0) {
            String badge = String.valueOf(enabled);
            int bLen = textRenderer.getWidth(badge);
            int badgeX = bx + bw - bLen - 10;
            ctx.fill(badgeX-4, by+9, badgeX+bLen+4, by+27, C_PURPLE);
            ctx.drawText(textRenderer, badge, badgeX, by+12, 0xFFFFFFFF, false);
        }
    }

    private void renderModuleList(DrawContext ctx, int mx, int my) {
        List<String> mods = CATEGORY_MODULES.get(catNames[selCat]);
        int lx     = px + SW + 6;
        int lRight = px + SW + LW - 6;
        int top    = py + 8;
        int bot    = py + H - 22;

        ctx.enableScissor(px+SW, py, px+SW+LW, py+H);

        for (int i = 0; i < mods.size(); i++) {
            String mod     = mods.get(i);
            boolean active = MODULE_STATES.getOrDefault(mod, false);
            int iy = top + i * 40 - scrollY;
            if (iy + 36 < top || iy > bot) continue;

            boolean hovered  = mx>=lx && mx<=lRight && my>=iy && my<=iy+36;
            boolean selected = mod.equals(selMod);

            if (hovered || selected) ctx.fill(lx, iy, lRight, iy+36, C_HOV);
            if (active)              ctx.fill(lx, iy, lx+3,   iy+36, C_PURPLE);

            ctx.drawText(textRenderer, mod, lx+10, iy+7,  active ? C_TEXT   : C_DIM,  false);
            ctx.drawText(textRenderer, active ? "ON" : "OFF",
                                              lx+10, iy+20, active ? C_PURPLE : C_MUTE, false);
        }

        ctx.disableScissor();

        // Scroll indicator dots if list overflows
        int totalH = mods.size() * 40;
        int visible = bot - top;
        if (totalH > visible) {
            float frac  = (float) scrollY / (totalH - visible);
            int   barH  = Math.max(20, visible * visible / totalH);
            int   barY  = py + 8 + (int)((H - 30 - barH) * frac);
            ctx.fill(px+SW+LW-5, barY, px+SW+LW-2, barY+barH, C_MUTE);
        }
    }

    private void renderConfigPanel(DrawContext ctx, int mx, int my) {
        ctx.fill(px+SW+LW+1, py, px+W, py+H, C_PANEL);
        int cpx = px + SW + LW + 16;
        int cpy = py;

        if (selMod == null) {
            String hint = "Select a module";
            int hx = cpx + (W - SW - LW - textRenderer.getWidth(hint)) / 2;
            ctx.drawText(textRenderer, hint, hx, cpy + H/2, C_MUTE, false);
            return;
        }

        boolean en = MODULE_STATES.getOrDefault(selMod, false);

        // Module name + state
        ctx.drawText(textRenderer, selMod, cpx, cpy+18, C_TEXT, true);
        ctx.fill(cpx, cpy+32, cpx+textRenderer.getWidth(selMod), cpy+33, en ? C_PURPLE : C_MUTE);

        // Toggle pill button
        int btnX = cpx, btnY = cpy+42, btnW = 90, btnH = 22;
        boolean hovBtn = mx>=btnX && mx<=btnX+btnW && my>=btnY && my<=btnY+btnH;
        ctx.fill(btnX, btnY, btnX+btnW, btnY+btnH, en ? C_PURPLE : (hovBtn ? C_SEL : 0x30FFFFFF));
        String btnLbl = en ? "● Enabled" : "○ Disabled";
        ctx.drawText(textRenderer, btnLbl, btnX+8, btnY+7, en ? 0xFFFFFFFF : C_DIM, false);

        ctx.fill(cpx, cpy+74, px+W-16, cpy+75, C_DIV);
        ctx.drawText(textRenderer, "Config coming soon", cpx, cpy+84, C_MUTE, false);
    }

    // ── Input ─────────────────────────────────────────────────────────────────

    @Override
    public boolean mouseClicked(double mx, double my, int button) {
        if (button != 0) return super.mouseClicked(mx, my, button);
        int ix = (int) mx, iy = (int) my;

        // Category buttons
        for (int i = 0; i < catNames.length; i++) {
            int bx = px+8, by = py+52 + i*44;
            if (ix>=bx && ix<=bx+SW-16 && iy>=by && iy<=by+36) {
                selCat  = i;
                selMod  = null;
                scrollY = 0;
                return true;
            }
        }

        // Toggle button in config panel
        if (selMod != null) {
            int btnX = px+SW+LW+16, btnY = py+42;
            if (ix>=btnX && ix<=btnX+90 && iy>=btnY && iy<=btnY+22) {
                toggleModule(selMod);
                return true;
            }
        }

        // Module list clicks
        List<String> mods = CATEGORY_MODULES.get(catNames[selCat]);
        int lx = px+SW+6, lRight = px+SW+LW-6;
        for (int i = 0; i < mods.size(); i++) {
            String mod = mods.get(i);
            int modY = py+8 + i*40 - scrollY;
            if (ix>=lx && ix<=lRight && iy>=modY && iy<=modY+36) {
                if (mod.equals(selMod)) {
                    toggleModule(mod);   // double-click area also toggles
                } else {
                    selMod = mod;
                }
                return true;
            }
        }

        return super.mouseClicked(mx, my, button);
    }

    @Override
    public boolean mouseScrolled(double mx, double my, double hAmt, double vAmt) {
        // Only scroll when mouse is over the module list
        if (mx >= px+SW && mx <= px+SW+LW) {
            List<String> mods = CATEGORY_MODULES.get(catNames[selCat]);
            int maxScroll = Math.max(0, mods.size() * 40 - (H - 30));
            scrollY = Math.max(0, Math.min(maxScroll, scrollY - (int)(vAmt * 14)));
        }
        return true;
    }

    // ── Helpers ───────────────────────────────────────────────────────────────

    private void toggleModule(String name) {
        boolean next = !MODULE_STATES.getOrDefault(name, false);
        MODULE_STATES.put(name, next);

        // Sync to C++ if the native bridge is active
        if (NativeBridge.isLoaded()) {
            NativeBridge.setModuleEnabled(name.toLowerCase().replace(" ", "_"), next);
        }
    }
}
