#include "host_environment.h"

#include "windows.h"

namespace sunrise::core::runtime {
bool is_wine() {
    static const bool is_linux = []() -> bool {
        HMODULE hntdll = GetModuleHandleW(L"ntdll.dll");
        if (!hntdll) {
            return false;
        }
        return GetProcAddress(hntdll, "wine_get_version")
               != nullptr; // Exported by wine to identify itself but not by real windows
    }();

    return is_linux;
}
} // namespace sunrise::core::runtime
