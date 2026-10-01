// SDK External
// For external cheats

#include <sdk/sdk.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <string>
#include <ranges>
#include <string_view>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <winternl.h>

namespace sdk {
    typedef LONG (WINAPI *TNtReadVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToRead, PSIZE_T NumberOfBytesReaded);
    typedef LONG (WINAPI *TNtWriteVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToWrite, PSIZE_T  NumberOfBytesWritten);

    HMODULE ntdll = GetModuleHandleA("ntdll.dll");

    auto NtReadVirtualMemory = (TNtReadVirtualMemory)GetProcAddress(ntdll, "NtReadVirtualMemory");
    auto NtWriteVirtualMemory = (TNtWriteVirtualMemory)GetProcAddress(ntdll, "NtWriteVirtualMemory");

    namespace process {
        unsigned int find_pid(std::string_view process_name) {
            const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot == INVALID_HANDLE_VALUE)
                return 0;

            PROCESSENTRY32 entry{};
            entry.dwSize = sizeof(entry);

            unsigned int pid = 0;
            if (Process32First(snapshot, &entry)) {
                do {
                    if (process_name == entry.szExeFile) {
                        pid = entry.th32ProcessID;
                        break;
                    }
                } while (Process32Next(snapshot, &entry));
            }

            CloseHandle(snapshot);
            return pid;
        }

        HANDLE find_handle(unsigned int pid) {
            if (pid == 0)
                return nullptr;

            return OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);
        }

        bool read_raw(HANDLE handle, uintptr_t address, void* output, size_t size) {
            if (handle == nullptr || size == 0)
                return false;

            size_t bytes_read;
            return NtReadVirtualMemory(
                handle,
                reinterpret_cast<void*>(address),
                output,
                size,
                &bytes_read
            ) && bytes_read == size;
        }

        bool write_raw(HANDLE handle, uintptr_t address, void* input, size_t size) {
            if (handle == nullptr || input == nullptr || size == 0)
                return false;

            size_t bytes_written = 0;
            return NtWriteVirtualMemory(
                handle,
                reinterpret_cast<void*>(address),
                input,
                size,
                &bytes_written
            ) && bytes_written == size;
        }

        std::string read_string(HANDLE handle, uintptr_t address) {
            uint32_t string_length{};
            read_raw(handle, address + 0x10, &string_length, sizeof(uint32_t));
            uintptr_t string_address{};

            if (string_length >= 16)
                read_raw(handle, address, &string_address, sizeof(uintptr_t));
            else
                string_address = address;

            if (string_length <= 0 || string_length > 255)
                return "Unknown";

            std::vector<char> buffer(string_length + 1, 0);
            read_raw(handle, string_address, buffer.data(), string_length);

            return std::string(buffer.data(), string_length);
        }

        uintptr_t find_module_address(HANDLE handle, std::string_view module_name) {
            uintptr_t module_address = 0;

            if (!handle) {
                return module_address;
            }

            DWORD process_id = GetProcessId(handle);
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

            if (snapshot == INVALID_HANDLE_VALUE) {
                CloseHandle(snapshot);
                return module_address;
            }

            MODULEENTRY32 module_entry{};
            module_entry.dwSize = sizeof(MODULEENTRY32);

            if (Module32First(snapshot, &module_entry)) {
                do {
                    if (module_name == module_entry.szModule) {
                        module_address = reinterpret_cast<uintptr_t>(module_entry.modBaseAddr);
                        break;
                    }
                } while (Module32Next(snapshot, &module_entry));
            }

            CloseHandle(snapshot);
            return module_address;
        }

        uintptr_t find_pattern(HANDLE handle, std::string_view module, std::string_view pattern) {
            uintptr_t module_base = 0;
            unsigned long module_size = 0;
            std::vector<int16_t> parsed_pattern;
            uintptr_t pattern_address = 0;

            // first, parse the pattern
            auto split = std::views::split(pattern, ' ');
    
            for (auto chunk : split) {
                std::string word(chunk.begin(), chunk.end());
                int16_t num;
                if (word.empty()) {
                    continue;
                } else if (word == "?") {
                    num = -1;
                } else {
                    num = std::stoi(word, nullptr, 16);
                }
                parsed_pattern.push_back(num);
            }

            // now get the module
            int pid = GetProcessId(handle);
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

            if (hSnapshot == INVALID_HANDLE_VALUE) {
                return 0;
            }

            MODULEENTRY32 me32;
            me32.dwSize = sizeof(MODULEENTRY32);

            if (!Module32First(hSnapshot, &me32)) {
                CloseHandle(hSnapshot);
                return 0;
            }

            do {
                if (me32.szModule == module) {
                    module_base = (uintptr_t)me32.modBaseAddr;
                    module_size = me32.modBaseSize;
                    break;
                }
            } while (Module32Next(hSnapshot, &me32));

            CloseHandle(hSnapshot);

            if (module_base == 0 || module_size == 0)
                return 0;

            std::vector<uint8_t> buffer;
            SIZE_T bytes_read = 0;
            if (!read_raw(
                handle,
                module_base,
                buffer.data(),
                module_size
            )) {
                return 0;
            };

            // and now finally scan the memory
            size_t pattern_length = parsed_pattern.size();
            
            for (size_t i = 0; i <= bytes_read - pattern_length; ++i) {
                bool found = true;

                for (size_t j = 0; j < pattern_length; ++j) {
                    if (parsed_pattern[j] != -1 && parsed_pattern[j] != buffer[i + j]) {
                        found = false;
                        break;
                    }
                }

                if (found) {
                    pattern_address = module_base + i;
                    break;
                }
            }

            return pattern_address;
        }
    }

    game::game(std::string_view process_name) {
        _pid = process::find_pid(process_name);
        _handle = process::find_handle(_pid);
        if (_handle == nullptr)
            exit(1);
    }

    game::~game() {
        CloseHandle(_handle);
    }
}
