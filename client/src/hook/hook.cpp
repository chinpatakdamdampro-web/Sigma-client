/**
 * hook.cpp
 *
 * Two hooks are installed:
 *
 *  1. wglSwapBuffers (opengl32.dll)
 *     Every rendered frame calls this. We intercept it to:
 *       - Init ImGui once (OpenGL3 + Win32 backends)
 *       - Run a full ImGui frame: NewFrame → Menu::render() → Render → RenderDrawData
 *       - Call the real wglSwapBuffers to present the frame
 *
 *  2. WndProc (GLFW30 window)
 *     Replaces the window procedure via SetWindowLongPtrA so we can:
 *       - Feed all messages to ImGui_ImplWin32_WndProcHandler
 *       - Toggle the menu on VK_INSERT keydown
 *       - Block mouse messages from reaching Minecraft while the menu is open
 */

#include "hook.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>

// MINHook — single-header-friendly API
#include <MinHook.h>

// ImGui
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_opengl3.h>

// Our systems
#include "../gui/menu.h"

// Forward-declare the ImGui WndProc handler (provided by imgui_impl_win32)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Spectral {

// ── Globals ───────────────────────────────────────────────────────────────────

// Singleton pointer used by static callbacks
static Hook* g_hook = nullptr;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Find the Minecraft GLFW window.  Minecraft 1.21 (LWJGL3/GLFW) registers
// its window with class "GLFW30".  We wait up to 30 s for it to appear.
static HWND waitForMinecraftWindow(int timeoutMs = 30000) {
    const int pollMs = 200;
    for (int elapsed = 0; elapsed < timeoutMs; elapsed += pollMs) {
        // Try both possible title prefixes
        HWND hwnd = FindWindowA("GLFW30", nullptr);
        if (hwnd) return hwnd;
        Sleep(pollMs);
    }
    return nullptr;
}

// ── Hook::init ────────────────────────────────────────────────────────────────

bool Hook::init() {
    g_hook = this;

    // 1. Wait for the game window
    m_hwnd = waitForMinecraftWindow();
    if (!m_hwnd) return false;

    // 2. Initialise MINHook
    if (MH_Initialize() != MH_OK) return false;

    // 3. Hook wglSwapBuffers
    if (!hookSwapBuffers()) {
        MH_Uninitialize();
        return false;
    }

    // 4. Hook WndProc for keyboard/mouse input
    hookWndProc(m_hwnd);   // non-fatal if this fails

    m_ready = true;
    return true;
}

// ── Hook::shutdown ────────────────────────────────────────────────────────────

void Hook::shutdown() {
    // Restore WndProc
    if (m_hwnd && m_origProc) {
        SetWindowLongPtrA(m_hwnd, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(m_origProc));
        m_origProc = nullptr;
    }

    // Remove all MINHook hooks + uninitialise
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    // Shut down ImGui
    if (m_imguiInit) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        m_imguiInit = false;
    }

    m_ready = false;
    g_hook  = nullptr;
}

// ── hookSwapBuffers ───────────────────────────────────────────────────────────

bool Hook::hookSwapBuffers() {
    // Resolve the real wglSwapBuffers from opengl32.dll
    HMODULE gl = GetModuleHandleA("opengl32.dll");
    if (!gl) gl = LoadLibraryA("opengl32.dll");
    if (!gl) return false;

    void* target = reinterpret_cast<void*>(
        GetProcAddress(gl, "wglSwapBuffers"));
    if (!target) return false;

    // Create + enable the hook
    if (MH_CreateHook(target,
            reinterpret_cast<void*>(&Hook::hkSwapBuffers),
            reinterpret_cast<void**>(&m_origSwap)) != MH_OK)
        return false;

    return MH_EnableHook(target) == MH_OK;
}

// ── hookWndProc ───────────────────────────────────────────────────────────────

bool Hook::hookWndProc(HWND hwnd) {
    // Replace the window procedure with our own
    LONG_PTR prev = SetWindowLongPtrA(hwnd, GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(&Hook::hkWndProc));
    if (!prev) return false;
    m_origProc = reinterpret_cast<WNDPROC>(prev);
    return true;
}

// ── initImGui ─────────────────────────────────────────────────────────────────

bool Hook::initImGui(HWND hwnd, HDC hdc) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Don't save an imgui.ini next to the MC jar
    io.IniFilename = nullptr;

    // Backend: Win32 (input) + OpenGL3 (rendering)
    if (!ImGui_ImplWin32_Init(hwnd))   return false;
    if (!ImGui_ImplOpenGL3_Init("#version 130")) return false;

    // Apply Spectral style + register modules
    Menu::get().init();

    m_imguiInit = true;
    return true;
}

// ── hkSwapBuffers ─────────────────────────────────────────────────────────────

BOOL WINAPI Hook::hkSwapBuffers(HDC hdc) {
    Hook& h = Hook::get();

    // Lazy ImGui init — first time we have a valid GL context + HDC
    if (!h.m_imguiInit) {
        HWND hwnd = WindowFromDC(hdc);
        if (hwnd) h.initImGui(hwnd, hdc);
    }

    if (h.m_imguiInit) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Menu::get().render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Call the real wglSwapBuffers to present
    return h.m_origSwap(hdc);
}

// ── hkWndProc ─────────────────────────────────────────────────────────────────

LRESULT WINAPI Hook::hkWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Hook& h = Hook::get();

    // Feed all messages to ImGui first
    if (h.m_imguiInit) {
        ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp);
    }

    // INSERT → toggle menu visibility
    if (msg == WM_KEYDOWN && wp == VK_INSERT) {
        Menu::get().toggle();
        return 0;
    }

    // If menu is open, swallow mouse messages so Minecraft doesn't react
    if (Menu::get().isVisible()) {
        switch (msg) {
            case WM_LBUTTONDOWN: case WM_LBUTTONUP:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP:
            case WM_MOUSEWHEEL:
            case WM_MOUSEMOVE:
                return 0;
            default:
                break;
        }
    }

    // Pass everything else to the original WndProc
    if (h.m_origProc) {
        return CallWindowProcA(h.m_origProc, hwnd, msg, wp, lp);
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

} // namespace Spectral
