package com.spectral.modules;

import com.spectral.NativeBridge;

/**
 * Snapshot of the C++ HitboxConfig pulled once per render frame via JNI.
 * Avoids spamming JNI calls inside the rendering loop.
 */
public final class HitboxConfig {

    public final float[] color;          // RGBA
    public final float   lineThickness;
    public final boolean fill;
    public final float   fillOpacity;
    public final boolean targetPlayers;
    public final boolean targetMobs;
    public final boolean targetAnimals;
    public final boolean targetItems;
    public final float   expandAmount;
    public final int     style;          // 0=Default, 1=Precise, 2=Expanded

    private HitboxConfig() {
        this.color         = NativeBridge.hitboxColor();
        this.lineThickness = NativeBridge.hitboxLineThickness();
        this.fill          = NativeBridge.hitboxFill();
        this.fillOpacity   = NativeBridge.hitboxFillOpacity();
        boolean[] targets  = NativeBridge.hitboxTargets();
        this.targetPlayers = targets[0];
        this.targetMobs    = targets[1];
        this.targetAnimals = targets[2];
        this.targetItems   = targets[3];
        this.expandAmount  = NativeBridge.hitboxExpandAmount();
        this.style         = NativeBridge.hitboxStyle();
    }

    /** Pull a fresh snapshot from the C++ side. */
    public static HitboxConfig fromNative() { return new HitboxConfig(); }
}
