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

## Toolchain and tests

Tool versions are pinned in `tools/toolchain.json` (NFR-010); check them with `python3 tools/verify_toolchain.py`.

The configure step fetches GoogleTest and Unity at the pinned commits (network required). Pass `-DPython3_EXECUTABLE=<python with pytest>` if the default Python lacks pytest.

```sh
cmake -S . -B build -G Ninja && cmake --build build
ctest --test-dir build --output-on-failure
```
