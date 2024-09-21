cmake_minimum_required(VERSION 3.10)
project(owop-cpp-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directories for uWebSockets
include_directories(C:/vcpkg/installed/x64-mingw-dynamic/include)
# Add library directories for uWebSockets
link_directories(C:/vcpkg/installed/x64-mingw-dynamic/lib)

# Add executable
add_executable(owop-cpp-server src/main.cpp)

# Link uWebSockets, OpenSSL, Zlib, libuv, and Winsock
target_link_libraries(owop-cpp-server uSockets ssl crypto zlib uv ws2_32)

# Ensure Visual Studio environment is set up
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0")
endif()

# Set the compiler explicitly if using MSVC
if(MSVC)
    set(CMAKE_C_COMPILER "cl.exe")
    set(CMAKE_CXX_COMPILER "cl.exe")
endif()

# Enable verbose output
set(CMAKE_VERBOSE_MAKEFILE ON)