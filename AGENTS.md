# AGENTS.md

## Project title

DFCS: Drone Flight Control System

## Project description

POC Quad-X flight-control firmware for STM32H743 (NUCLEO-H743ZI2), developed simulation-first. Not certified or flight-ready; no claim of flightworthiness (NFR-017). Source of truth: [docs/Drone_Flight_Control_System_SRS.md](docs/Drone_Flight_Control_System_SRS.md). Cite SRS IDs in commits and tests.

## Jira

Use only the Jira project `DC` (Drone-control) on karthikeyanaitrainings.atlassian.net. Do not read or write issues in any other project.

## Hard rules

- Actuator outputs are inhibited by default until init and safety checks pass (FR-002, FR-061).
- Never auto re-arm after failsafe or fault (FR-067).
- Never invent gains, mixer coefficients, or timing/memory targets; the SRS defers them to design (SRS 7.5, 8).
- MAVLink input: parse only supported messages, validate length, source, and parameters, and reject unsupported commands without changing actuator state (FR-051, FR-054, NFR-016).

## Folder structure

- `firmware/core/`: portable logic. See [firmware/core/AGENTS.md](firmware/core/AGENTS.md).
- `firmware/hal/`: STM32 HAL and drivers. See [firmware/hal/AGENTS.md](firmware/hal/AGENTS.md).
- `firmware/rtos/`: FreeRTOS tasks and scheduling. See [firmware/rtos/AGENTS.md](firmware/rtos/AGENTS.md).
- `firmware/app/`: application wiring.
- `sim/`: Gazebo bridge and SITL. See [sim/AGENTS.md](sim/AGENTS.md).
- `tests/`: unit, integration, SITL. See [tests/AGENTS.md](tests/AGENTS.md).
- `tools/`: scripts and toolchain manifest.
- `docs/`: SRS, design, test specification.

## Tech stack

C/C++17, FreeRTOS, STM32CubeH7 HAL/LL, GNU Arm toolchain, CMake + Ninja, Gazebo Harmonic, MAVLink 2, GitHub Actions. Pin all tool versions; never let them float in CI (NFR-010).

## Testing framework along with examples

GoogleTest (host C++), Unity (embedded C), pytest (automation/SITL), gcovr (coverage). Every change ships with tests. See [tests/AGENTS.md](tests/AGENTS.md).

- `TC-STATE-001`: arming rejected when pre-arm checks fail.
- `TC-MIX-001`: Quad-X mixer output mapping verified.

## Linting and formatting

clang-format, clang-tidy, cppcheck, cmake-format, ruff. CI fails on any violation (NFR-012, NFR-014).

## Simulation standards

See [sim/AGENTS.md](sim/AGENTS.md).

## External library references

STM32CubeH7, FreeRTOS, GoogleTest, Unity, MAVLink 2, Gazebo Harmonic, QGroundControl.

## Handling sensitive data

Never read or commit `.env*` files (except `.env.example`), ST-LINK or MAVLink signing keys, or calibration dumps containing device serial numbers.
