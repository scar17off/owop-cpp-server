# OWOP C++ Server

This is a simple C++ server for the OWOP protocol. It is built using the uWebSockets library and the OpenSSL library.  
This project is in early development and is not yet ready for production use.  
I'm using it to learn about the OWOP protocol and to build a working server to use for testing the OWOP C++ client.

## Building

You need to have [vcpkg](https://vcpkg.io/en/getting-started.html) installed and [mingw-w64](https://www.mingw-w64.org/) configured. Then you can build the project using [CMake](https://cmake.org/).

```sh
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### Building with vcpkg

```sh
cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=../vcpkg_root/scripts/buildsystems/vcpkg.cmake
mingw32-make
```

## Running
The output file is `build/owop-cpp-server.exe`. You can run it with:

```sh
./build/owop-cpp-server.exe
```

## Installing dependencies
The dependencies are:
- [uWebSockets](https://github.com/uNetworking/uWebSockets)
- [OpenSSL](https://github.com/openssl/openssl)

```sh
vcpkg install uwebsockets[core]:x64-mingw-dynamic
vcpkg install openssl:x64-mingw-dynamic
vcpkg install nlohmann-json3:x64-mingw-dynamic
```

# TODO
- [ ] Route the client to the http server
- [ ] The basic server classes and components
- [ ] Logging system
- [ ] Message handling
- [ ] Connection handling
- [ ] OWOP protocol implementation
- [ ] Customizeable ranking system

# License

This project is licensed under the MIT License. See the [LICENSE](LICENSE.md) file for more details.

# Contributing

Contributions are welcome! Please feel free to submit a PR.