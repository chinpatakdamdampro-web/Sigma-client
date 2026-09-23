#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Spectral {

// ── Hook ──────────────────────────────────────────────────────────────────────
//
// Responsibilities:
//   • MINHook init + teardown
//   • Hook wglSwapBuffers (opengl32.dll) for ImGui frame injection
//   • Hook WndProc (GLFW30 window) for input forwarding + INSERT toggle
//
class Hook {
public:
    static Hook& get() { static Hook inst; return inst; }

    // Called once from the loader thread after DLL attach.
    // Blocks until Minecraft's OpenGL window is ready, then installs hooks.
    bool init();

    // Called from DLL_PROCESS_DETACH. Removes all hooks cleanly.
    void shutdown();

    // True after init() succeeds.
    bool isReady() const { return m_ready; }

private:
    Hook()  = default;
    ~Hook() = default;
    Hook(const Hook&)            = delete;
    Hook& operator=(const Hook&) = delete;

    // ── Internal setup ───────────────────────────────────────────────────────
    bool hookSwapBuffers();
    bool hookWndProc(HWND hwnd);
    bool initImGui(HWND hwnd, HDC hdc);

    // ── State ────────────────────────────────────────────────────────────────
    bool     m_ready      = false;
    bool     m_imguiInit  = false;
    HWND     m_hwnd       = nullptr;
    WNDPROC  m_origProc   = nullptr;

    // Original function pointers (saved by MINHook)
    using wglSwapBuffers_t = BOOL(WINAPI*)(HDC);
    wglSwapBuffers_t m_origSwap = nullptr;

    // ── Hook implementations (static, called by trampolines) ─────────────────
    static BOOL  WINAPI hkSwapBuffers(HDC hdc);
    static LRESULT WINAPI hkWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};

} // namespace Spectral
