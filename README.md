# Photon Realtime GDExtension
<img src="https://github.com/user-attachments/assets/663991f2-b49d-43ac-85dc-40eea8a1ffe7" alt="godot-photon-logo" width="550">

## Description
Native C++ plugin for Godot (4.3+ branch) that integrates Photon Realtime (LoadBalancing) through GDExtension technology.

## Installation Instructions
Download the archive from the [latest GitHub release](https://github.com/adskoe96/godot-photon-plugin/releases/latest) or the [Godot Asset Store](https://store.godotengine.org/asset/adsk-dev/photon-realtime-gdextension/), and extract the `photon` folder into your Godot project's `res://addons/` directory.

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

## Build instructions

**Requirements for Windows:** Visual Studio 2022 (MSVC vc17) and CMake.

1. Clone repo with submodule:
```bash
   git clone https://github.com/adskoe96/godot-photon-plugin
   cd godot-photon-plugin
   git clone -b 4.3 https://github.com/godotengine/godot-cpp.git
```
2. Download Photon C++ SDK Realtime: [Windows](https://www.photonengine.com/sdks#realtime-windows) | [Linux](https://www.photonengine.com/sdks#realtime-linux)
3. Create `photon-sdk` folder and extract `Common-cpp`, `LoadBalancing-cpp`, `Photon-cpp` folders from downloaded Photon Realtime SDK archive to: `photon-sdk/windows` | `photon-sdk/linux`
4. Make directory for build and generate project:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```
4. Compile:
```bash
cmake --build . --config Release
```

## Documentation
The documentation is hosted on [Read the Docs](https://godot-photon-plugin.readthedocs.io/).

![Windows](https://custom-icon-badges.demolab.com/badge/Windows-111111?logo=windows&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)
![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?logo=cmake&logoColor=fff)  
