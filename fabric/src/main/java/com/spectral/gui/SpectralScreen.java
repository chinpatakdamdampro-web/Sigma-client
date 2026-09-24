package com.spectral.gui;

import com.spectral.NativeBridge;
import net.minecraft.client.gui.DrawContext;
import net.minecraft.client.gui.screen.Screen;
import net.minecraft.text.Text;

import java.util.*;

public class SpectralScreen extends Screen {

    // ── Module registry ───────────────────────────────────────────────────────
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

    public static boolean isModuleActive(String name) {
        if (NativeBridge.isLoaded())
            return NativeBridge.isModuleEnabled(name.toLowerCase().replace(" ", "_"));
        return MODULE_STATES.getOrDefault(name, false);
    }

    // ── Colors (ARGB) ─────────────────────────────────────────────────────────
    // Background layers
    private static final int BG_OVERLAY  = 0xCC050210;   // full-screen dim
    private static final int BG_PANEL    = 0xFF0D0A1F;   // main panel
    private static final int BG_HEADER   = 0xFF120E2B;   // header bar
    private static final int BG_SIDEBAR  = 0xFF0F0B22;   // sidebar
    private static final int BG_MOD_EVEN = 0xFF0D0A1F;   // module row even
    private static final int BG_MOD_ODD  = 0xFF110E24;   // module row odd
    private static final int BG_HOVER    = 0x25C084FC;   // row hover
    private static final int BG_CAT_ACT  = 0x35C084FC;   // active category
    private static final int BG_CAT_HOV  = 0x18FFFFFF;   // hovered category

    // Accents
    private static final int PURPLE      = 0xFF8B5CF6;
    private static final int PINK        = 0xFFEC4899;
    private static final int PURPLE_DIM  = 0xFF6D3FD4;
    private static final int GREEN       = 0xFF22C55E;

    // Text
    private static final int TEXT_WHITE  = 0xFFE2E8F0;
    private static final int TEXT_DIM    = 0xFF94A3B8;
    private static final int TEXT_MUTE   = 0xFF475569;
    private static final int TEXT_PURPLE = 0xFFBBA7FF;

    // Borders / dividers
    private static final int DIV         = 0xFF1E1A38;
    private static final int BORDER      = 0xFF2D2850;

    // ── Layout ────────────────────────────────────────────────────────────────
    private static final int HEADER_H = 28;
    private static final int FOOTER_H = 22;
    private static final int SIDEBAR_W = 130;
    private static final int CAT_ITEM_H = 38;
    private static final int MOD_ITEM_H = 36;

    private int panelX, panelY, panelW, panelH;

    private final String[] catNames = CATEGORY_MODULES.keySet().toArray(new String[0]);
    private int    selCat  = 0;
    private String selMod  = null;
    private int    scrollY = 0;

    public SpectralScreen() { super(Text.empty()); }

    // Prevent Minecraft's blur shader and dark overlay.
    // In 1.21.1, renderBackground() is what triggers both — making it a no-op
    // stops the blur from ever being scheduled.
    @Override
    public void renderBackground(DrawContext ctx, int mouseX, int mouseY, float delta) { }

    @Override
    public boolean shouldPause() { return false; }

    @Override
    protected void init() {
        // Use ~90% of screen width/height so it feels full
        panelW = Math.min(width  - 20, 820);
        panelH = Math.min(height - 20, 520);
        panelX = (width  - panelW) / 2;
        panelY = (height - panelH) / 2;
        scrollY = 0;
    }

    // ── Main render ───────────────────────────────────────────────────────────
    @Override
    public void render(DrawContext ctx, int mx, int my, float delta) {

        // 1. Full-screen dark overlay (our own, not MC's blur)
        ctx.fill(0, 0, width, height, BG_OVERLAY);

        // 2. Outer border glow (1px bright border around panel)
        ctx.fill(panelX - 1, panelY - 1, panelX + panelW + 1, panelY + panelH + 1, BORDER);

        // 3. Main panel background
        ctx.fill(panelX, panelY, panelX + panelW, panelY + panelH, BG_PANEL);

        // 4. Two-pixel top accent: purple left half, pink right half
        ctx.fill(panelX,           panelY, panelX + panelW / 2, panelY + 2, PURPLE);
        ctx.fill(panelX + panelW / 2, panelY, panelX + panelW, panelY + 2, PINK);

        renderHeader(ctx);
        renderSidebar(ctx, mx, my);
        renderModuleList(ctx, mx, my);
        renderFooter(ctx);

        // Let Screen render any registered widgets (none currently)
        super.render(ctx, mx, my, delta);
    }

    // ── Header ────────────────────────────────────────────────────────────────
    private void renderHeader(DrawContext ctx) {
        int hx = panelX, hy = panelY + 2;
        ctx.fill(hx, hy, hx + panelW, hy + HEADER_H, BG_HEADER);
        // Bottom border on header
        ctx.fill(hx, hy + HEADER_H, hx + panelW, hy + HEADER_H + 1, DIV);

        // Logo (bold with shadow for depth)
        ctx.drawText(textRenderer, "✦ SPECTRAL", hx + 12, hy + 9, PURPLE, true);

        // Right side: native status
        boolean native_ = NativeBridge.isLoaded();
        String  status   = native_ ? "● Native active" : "● Java mode";
        int     statusC  = native_ ? GREEN : TEXT_DIM;
        int     statusX  = hx + panelW - textRenderer.getWidth(status) - 12;
        ctx.drawText(textRenderer, status, statusX, hy + 9, statusC, false);
    }

    // ── Sidebar ───────────────────────────────────────────────────────────────
    private void renderSidebar(DrawContext ctx, int mx, int my) {
        int sx  = panelX;
        int sy  = panelY + 2 + HEADER_H + 1;
        int sh  = panelH - HEADER_H - FOOTER_H - 3;

        ctx.fill(sx, sy, sx + SIDEBAR_W, sy + sh, BG_SIDEBAR);
        // Right border
        ctx.fill(sx + SIDEBAR_W, sy, sx + SIDEBAR_W + 1, sy + sh, DIV);

        // Section label
        ctx.drawText(textRenderer, "CATEGORIES", sx + 10, sy + 8, TEXT_MUTE, false);

        for (int i = 0; i < catNames.length; i++) {
            int bx = sx + 6;
            int by = sy + 22 + i * CAT_ITEM_H;
            int bw = SIDEBAR_W - 12;
            int bh = CAT_ITEM_H - 4;

            boolean active  = i == selCat;
            boolean hovered = mx >= bx && mx < bx + bw && my >= by && my < by + bh;

            // Button background
            if (active)       ctx.fill(bx, by, bx + bw, by + bh, BG_CAT_ACT);
            else if (hovered) ctx.fill(bx, by, bx + bw, by + bh, BG_CAT_HOV);

            // Left accent bar for active category
            if (active) ctx.fill(bx, by + 3, bx + 3, by + bh - 3, PURPLE);

            // Category name
            int col = active ? TEXT_PURPLE : (hovered ? TEXT_WHITE : TEXT_DIM);
            ctx.drawText(textRenderer, catNames[i], bx + 10, by + (bh - 8) / 2, col, false);

            // Enabled-module count badge
            long count = CATEGORY_MODULES.get(catNames[i]).stream()
                .filter(m -> MODULE_STATES.getOrDefault(m, false)).count();
            if (count > 0) {
                String badge  = String.valueOf(count);
                int badgeW    = textRenderer.getWidth(badge) + 8;
                int badgeX    = bx + bw - badgeW - 2;
                int badgeY    = by + (bh - 10) / 2;
                ctx.fill(badgeX, badgeY, badgeX + badgeW, badgeY + 12, PURPLE_DIM);
                ctx.drawText(textRenderer, badge, badgeX + 4, badgeY + 2, TEXT_WHITE, false);
            }
        }
    }

    // ── Module list ───────────────────────────────────────────────────────────
    private void renderModuleList(DrawContext ctx, int mx, int my) {
        int lx  = panelX + SIDEBAR_W + 1;
        int ly  = panelY + 2 + HEADER_H + 1;
        int lw  = panelW - SIDEBAR_W - 1;
        int lh  = panelH - HEADER_H - FOOTER_H - 3;

        List<String> mods = CATEGORY_MODULES.get(catNames[selCat]);

        // Header bar showing category name
        ctx.fill(lx, ly, lx + lw, ly + 20, BG_HEADER);
        ctx.drawText(textRenderer, catNames[selCat].toUpperCase(),
            lx + 10, ly + 6, TEXT_MUTE, false);
        ctx.fill(lx, ly + 20, lx + lw, ly + 21, DIV);

        // Clip to module area
        int clipTop    = ly + 21;
        int clipBottom = ly + lh;
        ctx.enableScissor(lx, clipTop, lx + lw, clipBottom);

        for (int i = 0; i < mods.size(); i++) {
            String  mod     = mods.get(i);
            boolean enabled = MODULE_STATES.getOrDefault(mod, false);
            int     iy      = clipTop + i * MOD_ITEM_H - scrollY;

            if (iy + MOD_ITEM_H < clipTop || iy > clipBottom) continue;

            boolean hovered  = mx >= lx && mx < lx + lw && my >= iy && my < iy + MOD_ITEM_H;
            boolean selected = mod.equals(selMod);

            // Row background (alternating + hover/select tint)
            int rowBg = (i % 2 == 0) ? BG_MOD_EVEN : BG_MOD_ODD;
            ctx.fill(lx, iy, lx + lw, iy + MOD_ITEM_H, rowBg);
            if (hovered || selected)
                ctx.fill(lx, iy, lx + lw, iy + MOD_ITEM_H, BG_HOVER);

            // Left accent stripe if enabled
            if (enabled) ctx.fill(lx, iy, lx + 3, iy + MOD_ITEM_H, PURPLE);

            // Module name
            ctx.drawText(textRenderer, mod,
                lx + 12, iy + 8, enabled ? TEXT_WHITE : TEXT_DIM, false);

            // ON / OFF pill on the right
            String pill  = enabled ? "ON"  : "OFF";
            int    pillC = enabled ? PURPLE : TEXT_MUTE;
            int    pillW = textRenderer.getWidth(pill) + 14;
            int    pillX = lx + lw - pillW - 8;
            int    pillY = iy + (MOD_ITEM_H - 14) / 2;
            ctx.fill(pillX, pillY, pillX + pillW, pillY + 14,
                enabled ? 0x40C084FC : 0x25FFFFFF);
            ctx.drawText(textRenderer, pill, pillX + 7, pillY + 3, pillC, false);

            // Row divider
            ctx.fill(lx + 4, iy + MOD_ITEM_H - 1, lx + lw - 4, iy + MOD_ITEM_H, DIV);
        }

        ctx.disableScissor();

        // Scrollbar
        int totalH  = mods.size() * MOD_ITEM_H;
        int viewH   = clipBottom - clipTop;
        if (totalH > viewH) {
            float frac  = (float) scrollY / (totalH - viewH);
            int   barH  = Math.max(24, viewH * viewH / totalH);
            int   barY  = clipTop + (int)((viewH - barH) * frac);
            ctx.fill(lx + lw - 4, clipTop,  lx + lw - 2, clipBottom, 0x20FFFFFF);
            ctx.fill(lx + lw - 4, barY, lx + lw - 2, barY + barH, PURPLE_DIM);
        }
    }

    // ── Footer ────────────────────────────────────────────────────────────────
    private void renderFooter(DrawContext ctx) {
        int fy = panelY + panelH - FOOTER_H;
        ctx.fill(panelX, fy, panelX + panelW, fy + 1, DIV);
        ctx.fill(panelX, fy + 1, panelX + panelW, panelY + panelH, BG_HEADER);
        ctx.drawText(textRenderer,
            "Click module to toggle  •  Scroll to browse  •  ESC / INSERT to close",
            panelX + 10, fy + 7, TEXT_MUTE, false);
    }

    // ── Input ─────────────────────────────────────────────────────────────────
    @Override
    public boolean mouseClicked(double mx, double my, int button) {
        if (button != 0) return super.mouseClicked(mx, my, button);
        int ix = (int) mx, iy = (int) my;

        // Category sidebar clicks
        int sx = panelX + 6;
        int sy = panelY + 2 + HEADER_H + 1 + 22;
        for (int i = 0; i < catNames.length; i++) {
            int by = sy + i * CAT_ITEM_H;
            if (ix >= sx && ix < sx + SIDEBAR_W - 12 && iy >= by && iy < by + CAT_ITEM_H - 4) {
                selCat  = i;
                selMod  = null;
                scrollY = 0;
                return true;
            }
        }

        // Module list clicks
        int lx       = panelX + SIDEBAR_W + 1;
        int clipTop  = panelY + 2 + HEADER_H + 22;
        int clipBot  = panelY + panelH - FOOTER_H;
        int lw       = panelW - SIDEBAR_W - 1;
        if (ix >= lx && ix < lx + lw && iy >= clipTop && iy < clipBot) {
            List<String> mods = CATEGORY_MODULES.get(catNames[selCat]);
            int rel = iy - clipTop + scrollY;
            int idx = rel / MOD_ITEM_H;
            if (idx >= 0 && idx < mods.size()) {
                String mod = mods.get(idx);
                selMod = mod;
                boolean next = !MODULE_STATES.getOrDefault(mod, false);
                MODULE_STATES.put(mod, next);
                if (NativeBridge.isLoaded())
                    NativeBridge.setModuleEnabled(mod.toLowerCase().replace(" ","_"), next);
                return true;
            }
        }

        return super.mouseClicked(mx, my, button);
    }

    @Override
    public boolean mouseScrolled(double mx, double my, double hAmt, double vAmt) {
        int lx = panelX + SIDEBAR_W + 1;
        if (mx >= lx) {
            List<String> mods  = CATEGORY_MODULES.get(catNames[selCat]);
            int viewH   = panelH - HEADER_H - FOOTER_H - 24;
            int maxScroll = Math.max(0, mods.size() * MOD_ITEM_H - viewH);
            scrollY = (int) Math.max(0, Math.min(maxScroll, scrollY - vAmt * 18));
        }
        return true;
    }
}
