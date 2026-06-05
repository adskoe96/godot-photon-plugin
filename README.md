[![Read the Docs](https://img.shields.io/readthedocs/godot-photon-plugin?style=for-the-badge)](https://godot-photon-plugin.readthedocs.io/)

# Photon Cloud GDExtension
Native C++ plugin for Godot (4.3+ branch) that integrates Photon Realtime (LoadBalancing) through GDExtension technology.

## Features
- Connection to Master Server and Lobby system.
- Matchmaking API (Create, Join, JoinRandom, JoinOrCreate).
- Network Instantiation and Object Destruction.
- RPCs (Remote Procedure Calls) with target filtering (All, Others, MasterClient, Buffered).
- Smooth Transform and Rigidbody synchronization (Dead Reckoning).
- Automatic Network Scene Loading.
- Room and Player Custom Properties.
- Ownership Transfer for network objects.

## Project structure
- `godot-cpp/` — Submodule Godot 4.3.
- `photon-sdk/` — Photon C++ SDK.
- `src/` — Source code of GDExtension plugin.
- `game-project/` — Test project Godot.
- `CMakeLists.txt` — Main compile script.

## Installation Instructions
Download the [Release](https://github.com/adskoe96/godot-photon-plugin/releases/latest) archive and extract the `photon` folder into your Godot project's `res://addons/` directory.

## Build instructions

**Requirements for Windows:** Visual Studio 2022 (MSVC vc17) and CMake.

1. Clone repo with submodule:
```bash
   git clone https://github.com/adskoe96/godot-photon-plugin
   cd godot-photon-plugin
   git clone -b 4.3 https://github.com/godotengine/godot-cpp.git
```
2. Download Photon C++ SDK Realtime: https://www.photonengine.com/sdks#realtime-windows
3. Make directory for build and generate project:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```
4. Compile:
```bash
cmake --build . --config Release
```
