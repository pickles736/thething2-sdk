cmake_minimum_required(VERSION 3.16)
project(thething2-sdk VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # clangd

add_library(internal STATIC
    src/sdk.cpp
    src/sdkint.cpp
)

target_include_directories(internal
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        src/
)

add_library(external STATIC
    src/sdk.cpp
    src/sdkext.cpp
)

target_include_directories(external
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        src/
)

add_library(SDK::Internal ALIAS internal)
add_library(SDK::External ALIAS external)