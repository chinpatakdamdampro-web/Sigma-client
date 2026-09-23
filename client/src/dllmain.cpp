/**
 * dllmain.cpp
 *
 * Entry point for spectral_client.dll.
 *
 * Flow:
 *   DLL_PROCESS_ATTACH
 *     └─ CreateThread ──► loaderThread
 *                              └─ Hook::get().init()
 *                                    ├─ waits for the GLFW30 window
 *                                    ├─ installs wglSwapBuffers hook
 *                                    └─ installs WndProc hook
 *
 *   DLL_PROCESS_DETACH
 *     └─ Hook::get().shutdown()
 *          ├─ restores WndProc
 *          ├─ removes MINHook hooks
 *          └─ destroys ImGui context
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "hook/hook.h"

// ── Loader thread ─────────────────────────────────────────────────────────────

static DWORD WINAPI loaderThread(LPVOID) {
    // Short delay so Minecraft's JVM finishes loading before we probe windows
    Sleep(1500);

    if (!Spectral::Hook::get().init()) {
        // Init failed — nothing to clean up; thread exits silently
        return 1;
    }

    // Hook::init() returns as soon as hooks are installed.
    // From here on, rendering is driven by our wglSwapBuffers hook,
    // so this thread has nothing left to do.
    return 0;
}

// ── DllMain ───────────────────────────────────────────────────────────────────

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason) {

    case DLL_PROCESS_ATTACH:
        // Disable per-thread attach/detach notifications for performance
        DisableThreadLibraryCalls(hModule);

        // Spawn the loader on a new thread so DllMain returns immediately
        // (blocking DllMain causes deadlocks on Windows)
        CreateThread(
            nullptr,            // default security
            0,                  // default stack size
            loaderThread,
            nullptr,            // no argument needed
            0,                  // run immediately
            nullptr             // don't need thread ID
        );
        break;

    case DLL_PROCESS_DETACH:
        // Tear down hooks + ImGui.  Called when the JVM unloads the DLL or
        // when the process exits.  Hook::shutdown() is safe to call even if
        // init() never succeeded (all guards check m_ready / m_imguiInit).
        Spectral::Hook::get().shutdown();
        break;

    default:
        break;
    }

    return TRUE;
}
