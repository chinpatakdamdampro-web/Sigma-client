package com.spectral;

import java.io.*;
import java.nio.file.*;

public final class NativeBridge {

    private static boolean loaded = false;

    /** True only if the native library loaded successfully. */
    public static boolean isLoaded() { return loaded; }

    // ── Library loading ────────────────────────────────────────────────────

    public static void loadLibrary() {
        // Only meaningful on Windows — skip silently on other platforms
        String os = System.getProperty("os.name", "").toLowerCase();
        if (!os.contains("win")) {
            SpectralClient.LOGGER.warn(
                "[Spectral] Native library is Windows-only (detected OS: {}). " +
                "C++ overlay will not run.", os);
            return;
        }

        try {
            String name = "spectral_client.dll";
            Path tmp = Files.createTempDirectory("spectral_native");
            File out = tmp.resolve(name).toFile();

            try (InputStream is  = NativeBridge.class.getResourceAsStream("/natives/" + name);
                 OutputStream os2 = new FileOutputStream(out)) {
                if (is == null)
                    throw new FileNotFoundException("/natives/" + name + " not in JAR");
                is.transferTo(os2);
            }

            System.load(out.getAbsolutePath());
            loaded = true;
            SpectralClient.LOGGER.info("[Spectral] Native library loaded: {}", out);

        } catch (Exception | UnsatisfiedLinkError e) {
            SpectralClient.LOGGER.error("[Spectral] Native library failed to load — " +
                "C++ overlay disabled: {}", e.getMessage());
        }
    }

    // ── Module state ───────────────────────────────────────────────────────

    public static native boolean isModuleEnabled(String name);
    public static native void    setModuleEnabled(String name, boolean enabled);

    // ── Hitboxes config ────────────────────────────────────────────────────

    public static native float[]   hitboxColor();
    public static native float     hitboxLineThickness();
    public static native boolean   hitboxFill();
    public static native float     hitboxFillOpacity();
    public static native boolean[] hitboxTargets();
    public static native float     hitboxExpandAmount();
    public static native int       hitboxStyle();

    // ── ESP config ─────────────────────────────────────────────────────────

    public static native boolean espEnabled();
    public static native float[] espPlayerColor();
    public static native float[] espMobColor();

    // ── Lifecycle ──────────────────────────────────────────────────────────

    public static native void cleanup();
}
