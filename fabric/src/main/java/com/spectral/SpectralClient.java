package com.spectral;

import com.spectral.renderer.HitboxRenderer;
import net.fabricmc.api.ClientModInitializer;
import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientLifecycleEvents;
import net.fabricmc.fabric.api.client.rendering.v1.WorldRenderEvents;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SpectralClient implements ClientModInitializer {

    public static final String MOD_ID = "spectral";
    public static final Logger LOGGER = LoggerFactory.getLogger(MOD_ID);

    private static SpectralClient INSTANCE;

    @Override
    public void onInitializeClient() {
        INSTANCE = this;
        LOGGER.info("[Spectral] Initializing…");

        // Load the C++ DLL via JNI
        NativeBridge.loadLibrary();

        // ── Hitboxes render hook ───────────────────────────────────────────
        // AFTER_ENTITIES fires after all entity rendering so our boxes appear on top.
        WorldRenderEvents.AFTER_ENTITIES.register(context -> {
            if (!NativeBridge.isModuleEnabled("hitboxes")) return;
            HitboxRenderer.render(
                context.matrixStack(),
                context.consumers(),
                context.camera(),
                context.tickCounter().getTickDelta(true)
            );
        });

        // Cleanup native resources on client shutdown
        ClientLifecycleEvents.CLIENT_STOPPING.register(client -> {
            LOGGER.info("[Spectral] Shutting down native bridge…");
            NativeBridge.cleanup();
        });

        LOGGER.info("[SparkGalaxyClient] Ready.");
    }

    public static SpectralClient getInstance() { return INSTANCE; }
}
