@echo off
if exist build (
    cd build
    mingw32-make
    cd ..
) else (
    mkdir build
    cd build
    cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic ..
    mingw32-make
    cd ..
)
if exist build\bin\owop-cpp-server.exe (
    start "" "build\bin\owop-cpp-server.exe"
) else (
    echo Error: owop-cpp-server.exe not found in build\bin directory.
    pause
)