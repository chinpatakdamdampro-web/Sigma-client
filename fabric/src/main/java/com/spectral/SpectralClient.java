package com.spectral;

import com.spectral.gui.SpectralScreen;
import com.spectral.renderer.HitboxRenderer;
import net.fabricmc.api.ClientModInitializer;
import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientLifecycleEvents;
import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientTickEvents;
import net.fabricmc.fabric.api.client.keybinding.v1.KeyBindingHelper;
import net.fabricmc.fabric.api.client.rendering.v1.WorldRenderEvents;
import net.minecraft.client.option.KeyBinding;
import net.minecraft.client.util.InputUtil;
import org.lwjgl.glfw.GLFW;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SpectralClient implements ClientModInitializer {

    public static final String MOD_ID = "spectral";
    public static final Logger LOGGER = LoggerFactory.getLogger(MOD_ID);

    private static SpectralClient INSTANCE;
    private static KeyBinding menuKey;

    @Override
    public void onInitializeClient() {
        INSTANCE = this;
        LOGGER.info("[Spectral] Initializing...");

        // ── Native bridge (Windows / DLL only) ───────────────────────────────
        NativeBridge.loadLibrary();

        if (NativeBridge.isLoaded()) {
            LOGGER.info("[Spectral] Native bridge active — C++ overlay enabled.");
        } else {
            LOGGER.warn("[Spectral] Native bridge not loaded — Java GUI only.");
        }

        // ── Keybind: INSERT (rebindable in Options → Controls) ───────────────
        menuKey = KeyBindingHelper.registerKeyBinding(new KeyBinding(
            "key.spectral.menu",                    // translation key
            InputUtil.Type.KEYSYM,
            GLFW.GLFW_KEY_INSERT,                   // default: INSERT
            "Spectral Client"                       // category in Controls screen
        ));

        ClientTickEvents.END_CLIENT_TICK.register(client -> {
            while (menuKey.wasPressed()) {
                if (client.currentScreen instanceof SpectralScreen) {
                    client.setScreen(null);         // close if already open
                } else {
                    client.setScreen(new SpectralScreen());  // open
                }
            }
        });

        // ── Hitboxes world render hook ────────────────────────────────────────
        WorldRenderEvents.AFTER_ENTITIES.register(context -> {
            // Use SpectralScreen.isModuleActive — works with or without native bridge
            if (!SpectralScreen.isModuleActive("Hitboxes")) return;

            HitboxRenderer.render(
                context.matrixStack(),
                context.consumers(),
                context.camera(),
                context.tickCounter().getTickDelta(true)
            );
        });

        // ── Cleanup on shutdown ───────────────────────────────────────────────
        ClientLifecycleEvents.CLIENT_STOPPING.register(client -> {
            if (NativeBridge.isLoaded()) {
                LOGGER.info("[Spectral] Shutting down native bridge...");
                NativeBridge.cleanup();
            }
        });

        LOGGER.info("[SparkGalaxyClient] Ready. Press INSERT to open the menu.");
    }

    public static SpectralClient getInstance() { return INSTANCE; }
}
