package com.spectral.renderer;

import com.spectral.modules.HitboxConfig;
import net.minecraft.client.MinecraftClient;
import net.minecraft.client.render.Camera;
import net.minecraft.client.render.RenderLayer;
import net.minecraft.client.render.VertexConsumer;
import net.minecraft.client.render.VertexConsumerProvider;
import net.minecraft.client.util.math.MatrixStack;
import net.minecraft.entity.*;
import net.minecraft.entity.mob.MobEntity;
import net.minecraft.entity.passive.AnimalEntity;
import net.minecraft.entity.player.PlayerEntity;
import net.minecraft.util.math.Box;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import org.joml.Matrix4f;

/**
 * Renders entity hitboxes in the world.
 * Called every frame from SpectralClient's WorldRenderEvents.AFTER_ENTITIES hook.
 *
 * The C++ side owns the config (color, thickness, targets, etc.) and we pull it
 * once per frame via HitboxConfig.fromNative() to avoid JNI overhead in loops.
 */
public final class HitboxRenderer {

    private HitboxRenderer() {}

    public static void render(MatrixStack matrices,
                              VertexConsumerProvider consumers,
                              Camera camera,
                              float tickDelta) {

        HitboxConfig cfg = HitboxConfig.fromNative();

        MinecraftClient client = MinecraftClient.getInstance();
        if (client.world == null || consumers == null) return;

        Vec3d camPos = camera.getPos();
        VertexConsumer lines = consumers.getBuffer(RenderLayer.getLines());

        for (Entity entity : client.world.getEntities()) {
            if (entity == client.player && client.options.getPerspective().isFirstPerson()) continue;
            if (!shouldTarget(entity, cfg)) continue;

            // Interpolate render position
            double rx = MathHelper.lerp(tickDelta, entity.lastRenderX, entity.getX()) - camPos.x;
            double ry = MathHelper.lerp(tickDelta, entity.lastRenderY, entity.getY()) - camPos.y;
            double rz = MathHelper.lerp(tickDelta, entity.lastRenderZ, entity.getZ()) - camPos.z;

            Box raw = entity.getBoundingBox();
            double hw  = (raw.maxX - raw.minX) / 2.0;
            double h   = raw.maxY - raw.minY;
            double hd  = (raw.maxZ - raw.minZ) / 2.0;
            double exp = cfg.expandAmount;

            // Local AABB centred on foot position
            Box box = new Box(
                -hw - exp, -exp,    -hd - exp,
                 hw + exp,  h + exp * 2,  hd + exp
            );

            matrices.push();
            matrices.translate(rx, ry, rz);

            Matrix4f posM = matrices.peek().getPositionMatrix();

            float r = cfg.color[0], g = cfg.color[1],
                  b = cfg.color[2], a = cfg.color[3];

            // Draw outline
            drawBox(lines, posM, box, r, g, b, a);

            // Draw cross-hair at entity origin (style == Precise)
            if (cfg.style == 1) {
                drawCrossHair(lines, posM, r, g, b, a * .7f);
            }

            matrices.pop();
        }
    }

    // ── Private helpers ──────────────────────────────────────────────────

    private static boolean shouldTarget(Entity entity, HitboxConfig cfg) {
        if (entity instanceof PlayerEntity) return cfg.targetPlayers;
        if (entity instanceof AnimalEntity)  return cfg.targetAnimals;
        if (entity instanceof MobEntity)     return cfg.targetMobs;
        if (entity instanceof ItemEntity)    return cfg.targetItems;
        return false;
    }

    /** Draw all 12 edges of an AABB. */
    private static void drawBox(VertexConsumer vc, Matrix4f pm,
                                Box b, float r, float g, float cr, float a) {
        float x0 = (float)b.minX, y0 = (float)b.minY, z0 = (float)b.minZ;
        float x1 = (float)b.maxX, y1 = (float)b.maxY, z1 = (float)b.maxZ;

        // Bottom ring
        line(vc,pm, x0,y0,z0, x1,y0,z0, r,g,cr,a);
        line(vc,pm, x1,y0,z0, x1,y0,z1, r,g,cr,a);
        line(vc,pm, x1,y0,z1, x0,y0,z1, r,g,cr,a);
        line(vc,pm, x0,y0,z1, x0,y0,z0, r,g,cr,a);
        // Top ring
        line(vc,pm, x0,y1,z0, x1,y1,z0, r,g,cr,a);
        line(vc,pm, x1,y1,z0, x1,y1,z1, r,g,cr,a);
        line(vc,pm, x1,y1,z1, x0,y1,z1, r,g,cr,a);
        line(vc,pm, x0,y1,z1, x0,y1,z0, r,g,cr,a);
        // Verticals
        line(vc,pm, x0,y0,z0, x0,y1,z0, r,g,cr,a);
        line(vc,pm, x1,y0,z0, x1,y1,z0, r,g,cr,a);
        line(vc,pm, x1,y0,z1, x1,y1,z1, r,g,cr,a);
        line(vc,pm, x0,y0,z1, x0,y1,z1, r,g,cr,a);
    }

    /** Small cross-hair cross at entity foot origin (for Precise style). */
    private static void drawCrossHair(VertexConsumer vc, Matrix4f pm,
                                      float r, float g, float b, float a) {
        float s = .15f;
        line(vc,pm, -s,0,0, s,0,0, r,g,b,a);
        line(vc,pm, 0,-s,0, 0,s,0, r,g,b,a);
        line(vc,pm, 0,0,-s, 0,0,s, r,g,b,a);
    }

    /** Emit one line segment (two vertices required for RenderLayer.getLines). */
    private static void line(VertexConsumer vc, Matrix4f pm,
                             float x0, float y0, float z0,
                             float x1, float y1, float z1,
                             float r, float g, float b, float a) {
        float dx = x1-x0, dy = y1-y0, dz = z1-z0;
        float len = (float)Math.sqrt(dx*dx+dy*dy+dz*dz);
        if (len < 1e-6f) return;
        float nx = dx/len, ny = dy/len, nz = dz/len;
        vc.vertex(pm, x0, y0, z0).color(r,g,b,a).normal(nx,ny,nz);
        vc.vertex(pm, x1, y1, z1).color(r,g,b,a).normal(nx,ny,nz);
    }
}
