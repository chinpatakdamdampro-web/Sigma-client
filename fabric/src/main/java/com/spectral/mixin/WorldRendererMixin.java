package com.spectral.mixin;

import net.minecraft.client.render.WorldRenderer;
import org.spongepowered.asm.mixin.Mixin;

/**
 * Reserved mixin for WorldRenderer.
 * Current modules (Hitboxes, ESP) use Fabric's WorldRenderEvents API,
 * which is cleaner and does not require hooks here.
 *
 * Add @Inject / @Redirect targets here only when the event API is insufficient.
 */
@Mixin(WorldRenderer.class)
public abstract class WorldRendererMixin {
    // intentionally empty — see SpectralClient for event registrations
}
