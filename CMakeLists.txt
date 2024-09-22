cmake_minimum_required(VERSION 3.10)
project(owop-cpp-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directories for uWebSockets
include_directories(C:/vcpkg/installed/x64-mingw-dynamic/include)

# Find nlohmann_json package
find_package(nlohmann_json CONFIG REQUIRED)
if(NOT nlohmann_json_FOUND)
    message(FATAL_ERROR "nlohmann_json not found. Please install it or set nlohmann_json_DIR to the correct path.")
endif()

# Add library directories for uWebSockets
link_directories(C:/vcpkg/installed/x64-mingw-dynamic/lib)

# Add executable
add_executable(owop-cpp-server 
    src/main.cpp 
    src/components/Client.cpp
    src/components/Bucket.cpp
)

# Link uWebSockets, OpenSSL, Zlib, libuv, and Winsock
target_link_libraries(owop-cpp-server PRIVATE nlohmann_json::nlohmann_json uSockets ssl crypto zlib uv ws2_32)

# Ensure the output directory is set correctly
set_target_properties(owop-cpp-server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Create the output directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy config.json to the build output folder
add_custom_command(TARGET owop-cpp-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/src/config.json
        $<TARGET_FILE_DIR:owop-cpp-server>/config.json
)

# Copy protocol.json to the build output folder
add_custom_command(TARGET owop-cpp-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/src/protocol.json
        $<TARGET_FILE_DIR:owop-cpp-server>/protocol.json
)

# Copy routing directory to the build output folder if it exists
if(EXISTS ${CMAKE_SOURCE_DIR}/routing)
    add_custom_command(TARGET owop-cpp-server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/routing
            $<TARGET_FILE_DIR:owop-cpp-server>/routing
    )
else()
    message(WARNING "Routing directory not found. Skipping copy.")
endif()

# Add nlohmann_json installation directory to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-mingw-dynamic")

# Enable verbose output
set(CMAKE_VERBOSE_MAKEFILE ON)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)