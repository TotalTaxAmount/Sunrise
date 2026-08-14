#include <Windows.h>

#include <cstdio>
#include <intrin.h>
#include <fstream>
#include <string>
#include <mutex>
#include "client/hooks/egress/runtime.h"
#include "core/runtime/core_runtime.h"
#include "steam/runtime/internal.h"
#include "steam/runtime/runtime.h"

namespace {

HMODULE g_module{};

}

void LogSunrise(const std::string& msg) {
    static std::mutex s_mutex;
    std::lock_guard<std::mutex> lock(s_mutex);
    std::ofstream out("sunrise_debug.txt", std::ios::app);
    out << "[Sunrise] " << msg << "\n";
}

/** @return True when the egress guard and Core initialize from this DLL module. */
extern "C" __declspec(dllexport) bool SunriseInitialize() noexcept {
    if (!sunrise::client::hooks::egress::install() || !sunrise::core::initialize(g_module)) {
        return false;
    }
    // This export is one of two entry points; the guard reports once, whichever ran first.
    sunrise::client::hooks::egress::report_installation();
    return true;
}

/** @return True when the guard is in place and Client hooks activate. */
extern "C" __declspec(dllexport) bool SunriseActivateClient() noexcept {
    return sunrise::client::hooks::egress::is_installed()
           && sunrise::steam::runtime::activate_main_once();
}

/** @return True when the whole runtime shuts down cleanly. */
extern "C" __declspec(dllexport) bool SunriseShutdown() noexcept {
    return sunrise::steam::shutdown();
}

void InitializeWineDisplay() {
    HDC hdc = GetDC(NULL);
    if (hdc) {
        ReleaseDC(NULL, hdc);
    }
}

/** @return True when the Steam-compatible runtime initializes. */
extern "C" __declspec(dllexport) bool SteamAPI_Init() noexcept {
    InitializeWineDisplay();
    LogSunrise("-> SteamAPI_Init called");
    bool result = sunrise::steam::initialize(g_module);

    LogSunrise(std::string("<- SteamAPI_Init returned: ") + (result ? "TRUE" : "FALSE"));
    return result;
}

/** Stops the Steam-compatible runtime. */
extern "C" __declspec(dllexport) void SteamAPI_Shutdown() noexcept {
    (void)sunrise::steam::shutdown();
}

/** Delivers one batch of callbacks on the caller thread. */
extern "C" __declspec(dllexport) void SteamAPI_RunCallbacks() noexcept {
    sunrise::steam::run_callbacks();
}

/**
 * Stores the app id. The process is never restarted.
 * @return False because the in-process shim never asks for a restart.
 */
extern "C" __declspec(dllexport) bool SteamAPI_RestartAppIfNecessary(DWORD appId) noexcept {
    LogSunrise("-> SteamAPI_RestartAppIfNecessary called with AppID: " + std::to_string(appId));
    sunrise::steam::set_app_id(appId);
    return false;
}

/** @return True because this DLL provides the process-local Steam runtime. */
extern "C" __declspec(dllexport) bool SteamAPI_IsSteamRunning() noexcept {
    return sunrise::steam::is_running();
}

/** @return The shim's single Steam user handle. */
extern "C" __declspec(dllexport) sunrise::steam::UserHandle SteamAPI_GetHSteamUser() noexcept {
    return sunrise::steam::user_handle();
}

/** @return The shim's single Steam pipe handle. */
extern "C" __declspec(dllexport) sunrise::steam::PipeHandle SteamAPI_GetHSteamPipe() noexcept {
    return sunrise::steam::pipe_handle();
}

/**
 * Registers a Steam callback object.
 * @param callback Steam-owned callback object.
 * @param callbackId Steam callback type id.
 */
extern "C" __declspec(dllexport) void SteamAPI_RegisterCallback(void* callback,
                                                                int callbackId) noexcept {
    sunrise::steam::register_callback(callback, callbackId);
}

/** @param callback Steam-owned callback object removed from registrations. */
extern "C" __declspec(dllexport) void SteamAPI_UnregisterCallback(void* callback) noexcept {
    sunrise::steam::unregister_callback(callback);
}

/**
 * Registers a Steam API call-result object.
 * @param callback Steam-owned call-result object.
 * @param call Async API call id. Must not be zero.
 */
extern "C" __declspec(dllexport) void
SteamAPI_RegisterCallResult(void* callback, sunrise::steam::ApiCall call) noexcept {
    sunrise::steam::register_call_result(callback, call);
}

/**
 * Removes a Steam API call-result object.
 * @param callback Steam-owned call-result object.
 * @param call API call to remove, or zero for every call owned by the object.
 */
extern "C" __declspec(dllexport) void
SteamAPI_UnregisterCallResult(void* callback, sunrise::steam::ApiCall call) noexcept {
    sunrise::steam::unregister_call_result(callback, call);
}

/** @param data Steam-owned context table. @return Address of the interface field, after init. */
extern "C" __declspec(dllexport) void* SteamInternal_ContextInit(void* data) noexcept {
    LogSunrise("-> SteamInternal_ContextInit called");
    void* ptr = sunrise::steam::context_init(data);
    LogSunrise(std::string("<- SteamInternal_ContextInit returned: ") + (ptr ? "VALID POINTER" : "NULL"));
    return ptr;
}

/** @param version Interface version. @return Supported client interface or null. */
extern "C" __declspec(dllexport) void* SteamInternal_CreateInterface(const char* version) noexcept {
    std::string verStr = version ? version : "NULL";
    LogSunrise("-> SteamInternal_CreateInterface requested: " + verStr);

    void* ptr = sunrise::steam::create_interface(version, _ReturnAddress());

    if (ptr) {
        LogSunrise("<- SteamInternal_CreateInterface SUCCEEDED for: " + verStr);
    } else {
        LogSunrise("<- SteamInternal_CreateInterface FAILED (returned nullptr) for: " + verStr);
    }
    return ptr;
}

/**
 * Finds a supported user interface.
 * @param user Process-global zero handle, or the shim's local user handle.
 * @param version Interface version.
 * @return Supported user interface or null.
 */
extern "C" __declspec(dllexport) void*
SteamInternal_FindOrCreateUserInterface(sunrise::steam::UserHandle user,
    const char* version) noexcept {
    std::string verStr = version ? version : "NULL";
    LogSunrise("-> SteamInternal_FindOrCreateUserInterface requested: " + verStr);

    void* ptr = sunrise::steam::find_or_create_user_interface(user, version);

    if (ptr) {
        LogSunrise("<- SteamInternal_FindOrCreateUserInterface SUCCEEDED for: " + verStr);
    } else {
        LogSunrise("<- SteamInternal_FindOrCreateUserInterface FAILED (returned nullptr) for: " + verStr);
    }
    return ptr;
}

/**
 * Saves the DLL module and turns off unused thread notifications. Runs under the loader lock.
 * @param instance Loaded DLL module.
 * @return TRUE for every supported loader notification.
 */
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_module = instance;
        DisableThreadLibraryCalls(instance);
    } else if (reason == DLL_PROCESS_DETACH) {
        g_module = nullptr;
    }
    return TRUE;
}
