# DFCS

Drone Flight Control System: POC Quad-X flight-control firmware for STM32H743, developed simulation-first. See [docs/Drone_Flight_Control_System_SRS.md](docs/Drone_Flight_Control_System_SRS.md).

## Host build

Prerequisites: a C/C++17 compiler, CMake >= 3.25, and Ninja.

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

This builds the portable core as `build/firmware/core/libdfcs_core.a` (FR-070).

Clean rebuild from scratch (TC-CI-004):

```sh
rm -rf build
cmake -S . -B build -G Ninja && cmake --build build
```
