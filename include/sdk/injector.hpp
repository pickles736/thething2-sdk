// injector.hpp
// The executable that's compiled if the cheat
// is compiled in INTERNAL mode.
// Made to inject the DLL (coming from mainint.cpp)
//
// Not compiled if mode is EXTERNAL.

#pragma once

#include <string_view>

namespace injector {
    // The classic and noisy way to inject a DLL
    inline bool load_dll(std::string_view dll, unsigned int pid);
}
