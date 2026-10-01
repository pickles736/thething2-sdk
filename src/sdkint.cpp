// SDK Internal
// For internal cheats

#include "sdk/sdk.hpp"

#include <cstring>
#include <cstdint>
#include <handleapi.h>
#include <processthreadsapi.h>
#include <string>
#include <ranges>
#include <string_view>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>

namespace sdk {
    namespace process {
        bool read_raw(HANDLE handle, uintptr_t address, void* output, size_t size) {
            
        }

        bool write_raw(HANDLE handle, uintptr_t address, const void* input, size_t size) {
            
        }

        uintptr_t find_pattern(HANDLE handle, std::string_view module, std::string_view pattern) {
            
        }
    }

    game::game(std::string_view _) {
        _handle = GetCurrentProcess();
        _pid = GetCurrentProcessId();
    }

    game::~game() {}
}
