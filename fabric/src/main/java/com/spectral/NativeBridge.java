package com.spectral;

import java.io.*;
import java.nio.file.*;

/**
 * All native method declarations that map 1-to-1 to JNI exports in
 * client/src/jni/jni_bridge.cpp.
 *
 * Naming convention: Java_com_spectral_NativeBridge_<methodName>
 */
public final class NativeBridge {

    // ── Library loading ────────────────────────────────────────────────────

    public static void loadLibrary() {
        try {
            String name = "spectral_client.dll";
            Path tmp  = Files.createTempDirectory("spectral_native");
            File out  = tmp.resolve(name).toFile();

            try (InputStream is  = NativeBridge.class.getResourceAsStream("/natives/" + name);
                 OutputStream os = new FileOutputStream(out)) {
                if (is == null) throw new FileNotFoundException("natives/" + name + " not found in JAR");
                is.transferTo(os);
            }

            System.load(out.getAbsolutePath());
            SpectralClient.LOGGER.info("[Spectral] Native library loaded from: {}", out.getAbsolutePath());

        } catch (Exception e) {
            SpectralClient.LOGGER.error("[Spectral] Failed to load native library: {}", e.getMessage(), e);
        }
    }

    // ── Module state ───────────────────────────────────────────────────────

    /** Returns true if the named module is currently enabled in the C++ GUI. */
    public static native boolean isModuleEnabled(String name);

    /** Toggle a module from the Java side (rarely needed — GUI owns toggles). */
    public static native void setModuleEnabled(String name, boolean enabled);

    // ── Hitboxes config ────────────────────────────────────────────────────

    /** RGBA float[4] array for hitbox line/fill color. */
    public static native float[] hitboxColor();

    /** Width of rendered hitbox lines (0.5 – 5.0). */
    public static native float hitboxLineThickness();

    /** Whether to draw a translucent fill inside each hitbox. */
    public static native boolean hitboxFill();

    /** Opacity of the fill (0.0 – 1.0). */
    public static native float hitboxFillOpacity();

    /**
     * Which entity categories to target.
     * Returns boolean[4]: [players, mobs, animals, items]
     */
    public static native boolean[] hitboxTargets();

    /** Distance to expand the AABB outward on every axis (can be negative). */
    public static native float hitboxExpandAmount();

    /**
     * Box style:
     *   0 = Default  (exact AABB)
     *   1 = Precise  (per-limb skeleton boxes for players)
     *   2 = Expanded (adds expand on top of AABB)
     */
    public static native int hitboxStyle();

    // ── ESP config ─────────────────────────────────────────────────────────

    public static native boolean espEnabled();
    public static native float[] espPlayerColor();
    public static native float[] espMobColor();

    // ── Lifecycle ─────────────────────────────────────────────────────────

    /** Called when the game shuts down so C++ can release resources. */
    public static native void cleanup();
}
