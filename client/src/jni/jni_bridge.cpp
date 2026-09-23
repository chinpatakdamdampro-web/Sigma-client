/**
 * jni_bridge.cpp
 *
 * Every function here is exported as a JNI symbol matching:
 *   Java_com_spectral_NativeBridge_<methodName>
 *
 * The Java side (NativeBridge.java) declares each as `static native`.
 * The JVM calls these from the render / game-logic threads as needed.
 */

#include <jni.h>
#include "../modules/module_manager.h"

using namespace Spectral;

// Convenience macro for the JNI mangled prefix
#define JEXPORT(ret, method) \
    extern "C" JNIEXPORT ret JNICALL Java_com_spectral_NativeBridge_##method

// ── Module state ───────────────────────────────────────────────────────────

JEXPORT(jboolean, isModuleEnabled)(JNIEnv* env, jclass, jstring jname) {
    const char* name = env->GetStringUTFChars(jname, nullptr);
    bool result = ModuleManager::get().isEnabled(name);
    env->ReleaseStringUTFChars(jname, name);
    return (jboolean)result;
}

JEXPORT(void, setModuleEnabled)(JNIEnv* env, jclass, jstring jname, jboolean enabled) {
    const char* name = env->GetStringUTFChars(jname, nullptr);
    ModuleManager::get().setEnabled(name, (bool)enabled);
    env->ReleaseStringUTFChars(jname, name);
}

// ── Hitboxes ───────────────────────────────────────────────────────────────

JEXPORT(jfloatArray, hitboxColor)(JNIEnv* env, jclass) {
    auto& cfg = ModuleManager::get().hitboxes;
    jfloatArray arr = env->NewFloatArray(4);
    env->SetFloatArrayRegion(arr, 0, 4, cfg.color);
    return arr;
}

JEXPORT(jfloat, hitboxLineThickness)(JNIEnv*, jclass) {
    return (jfloat)ModuleManager::get().hitboxes.lineThickness;
}

JEXPORT(jboolean, hitboxFill)(JNIEnv*, jclass) {
    return (jboolean)ModuleManager::get().hitboxes.fill;
}

JEXPORT(jfloat, hitboxFillOpacity)(JNIEnv*, jclass) {
    return (jfloat)ModuleManager::get().hitboxes.fillOpacity;
}

JEXPORT(jbooleanArray, hitboxTargets)(JNIEnv* env, jclass) {
    auto& cfg = ModuleManager::get().hitboxes;
    jboolean buf[4] = {
        (jboolean)cfg.targetPlayers,
        (jboolean)cfg.targetMobs,
        (jboolean)cfg.targetAnimals,
        (jboolean)cfg.targetItems
    };
    jbooleanArray arr = env->NewBooleanArray(4);
    env->SetBooleanArrayRegion(arr, 0, 4, buf);
    return arr;
}

JEXPORT(jfloat, hitboxExpandAmount)(JNIEnv*, jclass) {
    return (jfloat)ModuleManager::get().hitboxes.expandAmount;
}

JEXPORT(jint, hitboxStyle)(JNIEnv*, jclass) {
    return (jint)ModuleManager::get().hitboxes.style;
}

// ── ESP ────────────────────────────────────────────────────────────────────

JEXPORT(jboolean, espEnabled)(JNIEnv*, jclass) {
    return (jboolean)ModuleManager::get().esp.enabled;
}

JEXPORT(jfloatArray, espPlayerColor)(JNIEnv* env, jclass) {
    auto& cfg = ModuleManager::get().esp;
    jfloatArray arr = env->NewFloatArray(4);
    env->SetFloatArrayRegion(arr, 0, 4, cfg.playerColor);
    return arr;
}

JEXPORT(jfloatArray, espMobColor)(JNIEnv* env, jclass) {
    auto& cfg = ModuleManager::get().esp;
    jfloatArray arr = env->NewFloatArray(4);
    env->SetFloatArrayRegion(arr, 0, 4, cfg.mobColor);
    return arr;
}

// ── Lifecycle ──────────────────────────────────────────────────────────────

JEXPORT(void, cleanup)(JNIEnv*, jclass) {
    // Called on game shutdown — nothing to clean up in module manager
    // Hook removal is handled by hook.cpp / DLL_PROCESS_DETACH
}
