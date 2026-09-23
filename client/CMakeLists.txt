cmake_minimum_required(VERSION 3.20)
project(spectral_client CXX C)

# ── Standard ──────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── MinHook (compiled from source) ───────────────────────────────────────────
# Cloned by the GitHub Actions workflow into extern/minhook-src/
add_library(minhook STATIC
    extern/minhook-src/src/buffer.c
    extern/minhook-src/src/hook.c
    extern/minhook-src/src/trampoline.c
    extern/minhook-src/src/hde/hde32.c
    extern/minhook-src/src/hde/hde64.c
)
target_include_directories(minhook PUBLIC extern/minhook-src/include)

# ── spectral_client.dll sources ───────────────────────────────────────────────
add_library(spectral_client SHARED
    # Entry point
    src/dllmain.cpp

    # Hook layer
    src/hook/hook.cpp

    # GUI
    src/gui/menu.cpp

    # Module system
    src/modules/hitboxes.cpp

    # JNI bridge
    src/jni/jni_bridge.cpp

    # ImGui core  (cloned into extern/imgui/ by the workflow)
    extern/imgui/imgui.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/backends/imgui_impl_win32.cpp
    extern/imgui/backends/imgui_impl_opengl3.cpp
)

# ── Include paths ─────────────────────────────────────────────────────────────
target_include_directories(spectral_client PRIVATE
    src
    extern/imgui
    extern/imgui/backends
)

# ── JNI headers ───────────────────────────────────────────────────────────────
if(DEFINED JDK_DIR)
    target_include_directories(spectral_client PRIVATE
        "${JDK_DIR}/include"
        "${JDK_DIR}/include/win32"
    )
else()
    find_package(JNI REQUIRED)
    target_include_directories(spectral_client PRIVATE ${JNI_INCLUDE_DIRS})
endif()

# ── Link ──────────────────────────────────────────────────────────────────────
target_link_libraries(spectral_client PRIVATE
    minhook
    opengl32
    gdi32
    dwmapi
    imm32
)

# ── Compiler flags ────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(spectral_client PRIVATE /W3 /O2 /MP /wd4996)
    set_property(TARGET spectral_client PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(spectral_client PRIVATE -O2 -Wall -Wno-deprecated-declarations)
    target_link_options(spectral_client PRIVATE -static-libgcc -static-libstdc++)
endif()

# ── Output ────────────────────────────────────────────────────────────────────
set_target_properties(spectral_client PROPERTIES
    OUTPUT_NAME    "spectral_client"
    PREFIX         ""
    SUFFIX         ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out"
)

message(STATUS "Output → ${CMAKE_BINARY_DIR}/out/spectral_client.dll")
