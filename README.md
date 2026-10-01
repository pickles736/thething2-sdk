## Requirements

* **CMake**
* **gcc/g++**
* **mingw-w64** (if on linux)

### Windows

Install CMake: `winget install KitWare.CMake`
Then download and extract the latest release of [**w64devkit**](https://github.com/skeeto/w64devkit/releases)
Then you can open the **w64devkit** terminal and use **gcc/g++**

### Linux

* **Debian/Ubuntu**: `apt install x86_64-w64-mingw32-g++ cmake`
* **Arch**: `pacman -S mingw-w64-gcc cmake`
* **Fedora**: `dnf install mingw64-gcc-c++ cmake`
* **OpenSUSE**: `zypper install mingw64-cross-gcc-c++ cmake`
* **Alpine**: `sudo apk add mingw-w64-gcc cmake`

## Integration

1. Clone the repository to your project or add it as a submodule (preferred)
2. Adjust and add the following to your **CMakeLists.txt**:
```bash
# Add the library directory
add_subdirectory(PATH_TO_THIS_REPO_IN_YOUR_PROJECT)

# Link with the library's EXTERNAL mode features
target_link_libraries(YOUR_CMAKE_PROJECT_NAME PRIVATE external)
# Or make INTERNAL features available instead (for a DLL)
target_link_libraries(YOUR_CMAKE_PROJECT_NAME PRIVATE internal)
```