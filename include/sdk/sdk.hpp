// sdk.hpp
// The main sdk header

#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>
#include <string>

typedef void* HANDLE;

namespace sdk {
    namespace process {
        #ifdef EXTERNAL // functions for external mode only. variable defined in cmakelists

        unsigned int find_pid(std::string_view process_name);
        HANDLE find_handle(unsigned int pid);

        #elif INTERNAL // funtions for internal mode only

        

        #endif
        
        bool read_raw(HANDLE handle, uintptr_t address, void* output, size_t size);
        bool write_raw(HANDLE handle, uintptr_t address, void* input, size_t size);
        std::string read_string(HANDLE handle, uintptr_t address);
        uintptr_t find_module_address(HANDLE handle, std::string_view module_name);
        uintptr_t find_pattern(HANDLE handle, std::string_view module, std::string_view pattern);
    }

    class game {
    public:
        game(const game&) = delete;
        game& operator=(const game&) = delete;
        
        game(std::string_view process_name);
        ~game();

        // Return the game's HANDLE
        HANDLE handle() {
            return _handle;
        }

        // Simple wrapper for ReadProcessMemory
        // Returns the value
        template<typename T>
        T read(uintptr_t address) {
            if constexpr (std::is_convertible_v<T, std::string_view>)
                return process::read_string(_handle, address);
            else {
                T value{};
                process::read_raw(_handle, address, &value, sizeof(T));
                return value;
            }
        }

        // Reads a string from an address
        // Same as read<std::string>(...)
        std::string read_string(uintptr_t address) {
            return process::read_string(_handle, address);
        }

        // Simple wrapper for ReadProcessMemory
        // Doesn't return the value, instead uses a buffer for large outputs
        // WARN: Does not support strings (unlike the variant that returns the value)
        template<typename T>
        void read(uintptr_t address, T* output) {
            process::read_raw(_handle, address, output, sizeof(T));
        }
        
        // Simple wrapper for WriteProcessMemory
        // Returns true if succeeded, false if not
        template<typename T>
        bool write(uintptr_t address, const T& value) {
            return process::write_raw(_handle, address, &value, sizeof(T));
        }

        // Find the base address of a module
        // Returns 0 if not found
        uintptr_t find_module_address(std::string_view module_name) {
            return process::find_module_address(_handle, module_name);
        }

        // Pattern scanner with wildcard support
        // Returns the beginning address of the pattern (excluding the first byte)
        uintptr_t find_pattern(std::string_view module, std::string_view pattern) {
            return process::find_pattern(_handle, module, pattern);
        }
    private:
        HANDLE _handle = nullptr;
        unsigned int _pid = 0;
    };
}

#define OBFUSCATE(s) (sdk::security::obfuscated_string<sizeof(s), __COUNTER__>(s).decrypt())