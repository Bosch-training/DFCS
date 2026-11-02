# Drone Flight Control System (DFCS)
## Software Requirements Specification (SRS)

**Document ID:** DFCS-SRS-001  
**Version:** 1.0  
**Status:** Baseline for POC development  
**Date:** 2026-11-02

---

## 1. Purpose

This document defines the requirements for a proof-of-concept (POC) custom quadcopter flight-control system. The project is intended to demonstrate an end-to-end software development lifecycle (SDLC), including requirements analysis, architecture and detailed design, implementation, verification, test automation, continuous integration/continuous delivery (CI/CD), simulation, and controlled hardware validation.

This is an engineering POC. It is **not an aviation-certified, production-ready, or inherently safe flight-control system**. No requirement in this document constitutes authorization to fly.

## 2. Project Overview

### 2.1 Objective

Develop custom flight-control firmware for a Quad-X multirotor using an STM32H743-based development board. Implement and verify the portable flight-control logic in simulation before integrating it with physical sensors and actuator interfaces.

### 2.2 Project goals

- Develop a modular embedded firmware architecture.
- Implement sensor acquisition, state estimation, flight-state management, attitude control, and motor mixing.
- Establish a simulation-first development and verification workflow.
- Automate unit, integration, static-analysis, build, and simulation tests.
- Establish traceability from requirements through design and tests.
- Demonstrate a reproducible CI pipeline and versioned firmware artifacts.
- Validate hardware interfaces on a bench with propulsion disabled.

### 2.3 Out of scope

The following are excluded from the initial POC:

- Autonomous waypoint missions.
- Obstacle detection or avoidance.
- Computer vision or AI-based navigation.
- Swarm or multi-drone coordination.
- Beyond-visual-line-of-sight operation.
- Payload delivery.
- Commercial or production deployment.
- Aviation certification or claims of compliance with a safety standard.
- Unsupervised or public flight testing.

## 3. Scope and Assumptions

### 3.1 In scope

1. Custom firmware for STM32H743.
2. FreeRTOS-based task scheduling.
3. Hardware abstraction and sensor-driver interfaces.
4. IMU acquisition and attitude estimation.
5. Basic attitude stabilization using configurable PID control.
6. Quad-X motor mixing and output limiting.
7. Flight-state and arming-state management.
8. Basic telemetry and command handling using MAVLink 2.
9. Software-in-the-loop (SITL) integration with Gazebo through a custom bridge.
10. Host-based unit and integration testing.
11. Hardware-in-the-loop (HIL) groundwork and bench validation.
12. Git-based source control and GitHub Actions CI.

### 3.2 Assumptions

- The initial airframe model is a four-motor Quad-X configuration.
- The selected development board is the STM32 NUCLEO-H743ZI2, subject to procurement and board-revision verification.
- The MCU is from the STM32H743 family and uses an ARM Cortex-M7 core.
- The portable control and estimation code will be buildable on a desktop host.
- Gazebo Harmonic will provide the simulated vehicle and environment.
- QGroundControl is the initial ground-station candidate; compatibility with the custom MAVLink message set must be verified.
- Sensor and ESC part numbers, electrical characteristics, pin mapping, and calibration parameters will be finalized during detailed design.
- Initial testing will use simulated actuators or electrically disconnected motor outputs. Physical propulsion is not part of initial acceptance.

## 4. System Context and Architecture

### 4.1 Logical architecture

The system consists of the following logical elements:

1. **Ground Control Station (GCS):** Displays telemetry and, where enabled, sends supported commands.
2. **Communication interface:** Receives and transmits MAVLink 2 messages.
3. **Flight application:** Manages flight states, arming conditions, and mode transitions.
4. **State estimation:** Processes sensor data to estimate vehicle attitude and other supported state values.
5. **Flight control:** Computes control demands from validated inputs and estimated state.
6. **Motor mixer:** Converts control demands into four bounded motor commands using the Quad-X geometry.
7. **Safety manager:** Enforces arming checks, output inhibition, fault handling, and failsafe transitions.
8. **Hardware abstraction layer (HAL):** Provides portable interfaces to MCU peripherals and device drivers.
9. **RTOS/platform layer:** Provides task scheduling, synchronization, timing, and hardware services.
10. **Simulation adapter:** Exchanges simulated sensor data and actuator commands with Gazebo.

### 4.2 Architecture principle

Flight-control, estimation, state-machine, and motor-mixing logic shall be independent of STM32-specific HAL calls and FreeRTOS APIs wherever practical. Hardware and simulation shall implement common interfaces so that the same portable application logic can be tested in both environments.

### 4.3 Proposed data flow

`Sensors / Simulator -> Sensor Drivers / Adapter -> Validation -> State Estimation -> Flight-State Manager and Controller -> Safety Output Gate -> Quad-X Mixer -> Actuator Interface / Simulator`

The safety output gate shall be able to inhibit actuator commands independently of the normal control calculation.

## 5. Technology Baseline

| Area | Baseline selection |
|---|---|
| MCU family | STM32H743 |
| Development board | ST NUCLEO-H743ZI2 |
| Firmware languages | C and C++17 |
| RTOS | FreeRTOS |
| MCU software package | STM32CubeH7 HAL/LL |
| Configuration | STM32CubeMX, where useful |
| Compiler | GNU Arm Embedded Toolchain |
| Build system | CMake and Ninja |
| Host unit testing | GoogleTest |
| Embedded C unit testing | Unity |
| Test automation | Python and pytest |
| Static analysis | Cppcheck and Clang-Tidy |
| Coverage | gcovr / host coverage tooling |
| Simulation | Gazebo Harmonic |
| Ground station | QGroundControl, subject to protocol compatibility |
| Vehicle communication | MAVLink 2 |
| Debugging | ST-LINK, GDB, OpenOCD |
| CI/CD | GitHub Actions |
| Source control | Git and GitHub |

Versions shall be pinned in repository configuration or documented toolchain manifests before implementation begins. Tool versions must not silently float in CI.

## 6. Users and External Actors

| Actor | Responsibility |
|---|---|
| Developer | Implements firmware and maintains code |
| Test engineer | Designs and executes verification |
| Operator | Uses the GCS in simulation or controlled bench testing |
| CI system | Builds, analyzes, tests, and publishes eligible artifacts |
| Simulator | Supplies simulated sensor/environment data and consumes actuator commands |
| Hardware | Supplies sensor readings and receives actuator-interface signals |

## 7. Functional Requirements

Requirement priority: **P0** = mandatory for initial POC; **P1** = required for the extended POC; **P2** = optional/future.

### 7.1 Initialization and lifecycle

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-001 | The firmware shall initialize required clocks, memory, peripherals, RTOS services, and application modules in a defined startup sequence. | P0 | Inspection, test |
| FR-002 | Actuator outputs shall remain inhibited from reset until initialization and all required safety checks have completed successfully. | P0 | Bench test |
| FR-003 | The firmware shall enter a defined safe state if a mandatory initialization step fails. | P0 | Fault-injection test |
| FR-004 | The firmware shall expose startup and initialization status through a diagnostic interface. | P1 | Integration test |

### 7.2 Sensor acquisition and validation

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-010 | The system shall provide an IMU interface for three-axis angular-rate and three-axis acceleration measurements. | P0 | Unit, integration |
| FR-011 | The system shall timestamp sensor samples using a monotonic time source. | P0 | Unit test |
| FR-012 | The system shall validate sensor data for freshness, range, and basic plausibility before use by control logic. | P0 | Unit, fault-injection |
| FR-013 | The system shall detect sensor communication errors and report a diagnostic fault. | P0 | Integration test |
| FR-014 | The system shall provide an interface for barometric altitude measurements. | P1 | Integration test |
| FR-015 | The system shall provide an interface for GNSS position and velocity measurements. | P1 | Integration test |
| FR-016 | The system shall provide an interface for magnetometer measurements if the selected estimator configuration requires them. | P1 | Integration test |
| FR-017 | Sensor calibration parameters shall be configurable and stored separately from application logic. | P1 | Inspection, test |

### 7.3 State estimation

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-020 | The system shall estimate roll, pitch, and yaw or an equivalent attitude representation from validated sensor data. | P0 | Unit, SITL |
| FR-021 | The estimator shall expose validity and health status with its estimated state. | P0 | Unit test |
| FR-022 | The estimator shall detect missing, stale, or invalid mandatory input data and mark its output invalid. | P0 | Fault-injection |
| FR-023 | Estimator parameters shall be configurable without changing control application code. | P1 | Inspection, test |
| FR-024 | The system shall provide an altitude estimate when the barometer integration is enabled. | P1 | SITL, integration |

### 7.4 Flight-state and mode management

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-030 | The firmware shall implement explicit states for BOOT, INITIALIZING, DISARMED, ARMED, FAILSAFE, and FAULT. | P0 | Unit test |
| FR-031 | State transitions shall be defined and validated against an explicit transition table. | P0 | Unit test |
| FR-032 | The system shall reject arming requests when mandatory pre-arm checks fail. | P0 | Unit, integration |
| FR-033 | The system shall reject unsupported or invalid mode commands. | P0 | Unit test |
| FR-034 | The system shall support a disarm request that inhibits normal motor commands. | P0 | Unit, bench |
| FR-035 | The system shall expose current state, active mode, and relevant rejection/fault reasons through telemetry or diagnostics. | P1 | Integration test |
| FR-036 | Initial POC flight modes shall be limited to explicitly implemented and tested modes; unsupported modes shall not be advertised as available. | P0 | Inspection, test |

### 7.5 Attitude control and motor mixing

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-040 | The system shall implement configurable PID control for the selected attitude-control axes. | P0 | Unit, SITL |
| FR-041 | The controller shall use validated state estimates and valid command inputs only. | P0 | Unit, fault-injection |
| FR-042 | The controller shall apply configured output limits and anti-windup handling. | P0 | Unit test |
| FR-043 | The system shall implement a Quad-X mixer mapping roll, pitch, yaw, and throttle demands to four motor commands. | P0 | Unit test |
| FR-044 | Mixer output shall be bounded to configured actuator limits. | P0 | Unit test |
| FR-045 | Motor ordering, rotation direction, and mixer signs shall be configuration-controlled and documented. | P0 | Inspection, bench |
| FR-046 | Invalid controller inputs or an active safety inhibit shall result in actuator outputs being inhibited or set to the defined safe output. | P0 | Unit, bench |
| FR-047 | Control gains and limits shall be configurable, validated, and traceable to a configuration version. | P1 | Inspection, test |

**Note:** Controller rates, gains, actuator ranges, and mixer coefficients are intentionally not specified in this SRS. They require engineering analysis, simulation, and hardware-interface verification before being baselined in the detailed design.

### 7.6 Communication and telemetry

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-050 | The system shall implement a MAVLink 2 transport interface over a selected serial link in the hardware build. | P1 | Integration |
| FR-051 | The system shall parse only supported messages and validate message length, source, and command parameters. | P0 | Unit, security test |
| FR-052 | The system shall expose heartbeat and basic system-status telemetry. | P1 | Integration |
| FR-053 | The system shall report supported sensor, estimator, arming, and fault status. | P1 | Integration |
| FR-054 | The system shall reject unsupported commands without changing actuator state. | P0 | Unit, integration |
| FR-055 | Communication parsing shall not block time-critical control execution. | P0 | Timing test |
| FR-056 | MAVLink message signing or an equivalent authenticated-command mechanism shall be evaluated before any operational radio-control deployment. | P1 | Design review |

### 7.7 Failsafe and safety behavior

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-060 | The system shall provide a hardware-accessible motor-disable mechanism independent of normal control calculations. | P0 | Hardware inspection, bench |
| FR-061 | The system shall inhibit actuator outputs during boot, initialization, disarmed, and fault states. | P0 | Bench test |
| FR-062 | The system shall detect loss or staleness of mandatory control inputs. | P0 | Fault-injection |
| FR-063 | The system shall transition to a defined failsafe state when a configured critical fault occurs. | P0 | Fault-injection |
| FR-064 | The system shall detect RTOS task-health or execution failures using watchdog supervision. | P0 | Fault-injection |
| FR-065 | The system shall detect configured battery-voltage threshold violations when battery monitoring is integrated. | P1 | Hardware-integration |
| FR-066 | Failsafe behavior shall be deterministic, documented, and testable for each defined fault class. | P0 | Review, test |
| FR-067 | Recovery from a failsafe or fault state shall require explicit, validated recovery conditions; automatic re-arming shall not occur. | P0 | Unit, integration |

Failsafe actuator behavior must be selected through safety analysis for the actual test configuration. The software shall not assume that a generic motor stop is safe in every airborne condition. Initial POC acceptance is limited to simulation and propulsion-disabled bench testing.

### 7.8 Simulation interface

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| FR-070 | The portable flight-control core shall be buildable for a desktop host target. | P0 | Build test |
| FR-071 | The simulation adapter shall provide sensor data to the portable application through the same logical interfaces used by hardware drivers. | P0 | Integration |
| FR-072 | The simulation adapter shall receive actuator commands and apply them to the simulated vehicle through a documented bridge. | P0 | SITL |
| FR-073 | The simulation environment shall support repeatable scenarios with recorded configuration and seed where applicable. | P1 | SITL |
| FR-074 | The system shall support automated tests for initialization, state transitions, sensor faults, controller response, and output limiting in simulation. | P0 | Automated SITL |

## 8. Non-Functional Requirements

| ID | Requirement | Priority | Verification |
|---|---|---:|---|
| NFR-001 | Time-critical control processing shall use bounded execution paths and shall not perform blocking console, file, or network I/O. | P0 | Review, timing test |
| NFR-002 | The detailed design shall define task priorities, periods, deadlines, stack sizes, and synchronization mechanisms. | P0 | Design review |
| NFR-003 | The implementation shall measure and report execution time and scheduling behavior for critical tasks. | P0 | Profiling |
| NFR-004 | The design shall define a worst-case execution-time and deadline analysis approach before hardware control-loop acceptance. | P0 | Design review |
| NFR-005 | Dynamic memory allocation shall not be used in time-critical runtime paths after system initialization. | P0 | Static review |
| NFR-006 | Shared data between tasks and interrupt contexts shall use documented synchronization and ownership rules. | P0 | Review, stress test |
| NFR-007 | DMA buffers and Cortex-M7 data-cache coherency shall be handled explicitly, including memory placement and cache maintenance where required. | P0 | Design review, hardware test |
| NFR-008 | The portable application core shall not depend directly on STM32 HAL or FreeRTOS APIs. | P0 | Architecture review |
| NFR-009 | Modules shall have documented interfaces and clear ownership of mutable data. | P0 | Review |
| NFR-010 | All build and test steps shall be reproducible using documented tool versions and commands. | P0 | CI validation |
| NFR-011 | Critical requirements shall be traceable to design elements and verification cases. | P0 | Traceability review |
| NFR-012 | The codebase shall follow agreed C and C++ coding standards and static-analysis rules. | P0 | CI |
| NFR-013 | The system shall provide diagnosable fault codes and useful logs without compromising time-critical execution. | P1 | Integration |
| NFR-014 | CI shall fail when mandatory build, test, or quality gates fail. | P0 | Pipeline test |
| NFR-015 | Release artifacts shall be versioned and associated with a source revision and build metadata. | P0 | Release audit |
| NFR-016 | Security-sensitive commands shall be validated, and malformed or unexpected input shall fail safely. | P0 | Negative testing |
| NFR-017 | The project shall not claim certification or flightworthiness based solely on passing POC tests. | P0 | Documentation review |

Numeric timing, memory, latency, coverage, and reliability targets shall be set during architecture and detailed design after measurement and analysis. They shall not be invented without supporting hardware and workload evidence.

## 9. External Interface Requirements

### 9.1 Sensor interfaces

Detailed design shall specify the exact device, electrical levels, bus speed, sample rate, data format, calibration, timeout, and error behavior for each selected sensor.

| Interface | Intended use | Status |
|---|---|---|
| SPI | Primary high-rate IMU connection | Proposed |
| I2C | Optional sensor/peripheral connection | Proposed |
| UART | GNSS and telemetry | Proposed |
| ADC | Battery voltage/current sensing | Proposed |
| GPIO / timer | Status, capture, and actuator interface | Proposed |

### 9.2 Actuator interface

- Initial hardware interface: PWM-compatible ESC interface, subject to ESC and timer verification.
- Four independent logical motor channels shall be represented.
- Output frequency, pulse range, startup behavior, and signal-loss behavior shall be specified from the selected ESC documentation.
- No motor shall be connected or enabled during initial software development and bench validation.
- Propeller removal and independent power isolation are mandatory for motor-interface bench work.

### 9.3 Ground station and telemetry

- MAVLink 2 is the proposed message protocol.
- The supported message and command subset shall be documented.
- QGroundControl interoperability shall be proven with a protocol compatibility test before it is treated as an accepted interface.
- Unsupported commands shall be rejected safely.

### 9.4 Simulation interface

The Gazebo bridge shall document message types, units, coordinate frames, timestamps, update rates, and failure behavior. Coordinate-frame conventions and unit conversions shall be explicit and covered by tests.

## 10. Flight-State Model

The following is the initial logical state model. Detailed design shall define every permitted transition and its guards.

| Current state | Event / condition | Next state | Required behavior |
|---|---|---|---|
| BOOT | Reset completed | INITIALIZING | Keep actuator outputs inhibited |
| INITIALIZING | Initialization and checks pass | DISARMED | Keep outputs inhibited |
| INITIALIZING | Mandatory initialization fails | FAULT | Keep outputs inhibited; report fault |
| DISARMED | Valid arm request and all checks pass | ARMED | Enable control path under defined limits |
| DISARMED | Invalid arm request | DISARMED | Reject request and report reason |
| ARMED | Valid disarm request | DISARMED | Inhibit normal actuator commands |
| ARMED | Critical fault | FAILSAFE or FAULT | Apply defined safety response |
| FAILSAFE | Fault cleared and explicit recovery accepted | DISARMED | Never automatically re-arm |
| FAULT | Reset/service recovery procedure | BOOT | Outputs remain inhibited |

The state model is a logical software model, not a complete operational flight-mode specification.

## 11. Initial Verification Strategy

### 11.1 Verification levels

| Level | Environment | Primary purpose |
|---|---|---|
| Static verification | Host / CI | Coding rules, static analysis, dependency checks |
| Unit testing | Host | Isolated logic and boundary conditions |
| Embedded unit testing | Target or host | C modules and hardware-independent interfaces |
| Integration testing | Host and target | Module, driver, RTOS, and communication integration |
| SITL | Gazebo + desktop build | Vehicle response and scenario validation |
| HIL groundwork | STM32 board + simulated inputs | Target firmware and interface validation |
| Bench validation | Propulsion disabled | Electrical and peripheral behavior |

### 11.2 Minimum test areas

1. Startup sequence and safe output state.
2. State-machine transition coverage.
3. Arming rejection for each failed precondition.
4. Sensor range, timeout, stale-data, and communication-error handling.
5. Attitude estimator validity and fault behavior.
6. PID saturation, anti-windup, and boundary behavior.
7. Quad-X mixer sign, motor ordering, and output bounds.
8. Failsafe activation and recovery rules.
9. MAVLink malformed, unsupported, and invalid command handling.
10. RTOS task health, watchdog, and timing behavior.
11. Simulation repeatability and sensor/actuator bridge correctness.
12. Firmware build, artifact generation, and release traceability.

### 11.3 Test case identification

Use IDs such as:

- `TC-BOOT-001` — actuator output inhibited during startup.
- `TC-SAFE-001` — motor-disable mechanism inhibits outputs.
- `TC-SENS-001` — stale IMU sample rejected.
- `TC-STATE-001` — arming rejected when pre-arm checks fail.
- `TC-CTRL-001` — controller output remains within configured limits.
- `TC-MIX-001` — Quad-X mixer output mapping verified.
- `TC-COMM-001` — unsupported command rejected.
- `TC-SIM-001` — SITL sensor and actuator bridge integration.
- `TC-CI-001` — pipeline blocks artifact release when mandatory tests fail.

Test procedures, expected results, test data, and pass/fail evidence shall be maintained separately in the test specification.

## 12. CI/CD Requirements

### 12.1 Pull-request pipeline

The GitHub Actions pull-request workflow shall execute:

1. Repository and formatting checks.
2. Static analysis.
3. Desktop/host build.
4. STM32 cross-compilation.
5. Unit tests.
6. Coverage report generation.
7. Integration tests that are suitable for the runner.
8. SITL tests when the simulator dependencies are available.
9. Artifact and test-report publication for review.

### 12.2 Release pipeline

A release workflow shall:

- Run only after required checks pass and release approval conditions are met.
- Build firmware from a tagged source revision.
- Generate the configured firmware formats (for example ELF, HEX, and BIN where supported).
- Attach version, source revision, toolchain, and build metadata.
- Publish test and static-analysis summaries.
- Preserve artifacts for traceability.

### 12.3 CI quality gates

- Build failure blocks the pipeline.
- Mandatory unit-test failure blocks the pipeline.
- Critical static-analysis findings block the pipeline according to the agreed rule set.
- Coverage thresholds shall be established after a baseline is measured; critical modules shall have explicit coverage goals.
- Failed safety-related tests block release.
- Hardware flashing and physical-flight authorization are not automated CI release actions.

## 13. Safety, Security, and Engineering Practices

### 13.1 Safety controls

- Independent motor-disable mechanism.
- Actuator output inhibition during startup and disarmed states.
- Explicit arming checks and state transitions.
- Watchdog and task-health supervision.
- Sensor freshness and validity checks.
- Communication timeout handling.
- Battery monitoring when integrated.
- Defined safe-state behavior for each fault.
- Propeller removal during bench testing.
- Physical power isolation and controlled test procedures.

### 13.2 Security controls

- Validate all externally supplied message fields and command parameters.
- Reject unsupported message types and commands.
- Avoid unsafe parsing, buffer overflows, and unbounded input handling.
- Do not store credentials, private keys, or secrets in source control.
- Use dependency and toolchain version pinning.
- Evaluate MAVLink 2 signing or an authenticated command channel before operational radio use.
- Restrict debug and firmware-update interfaces in any later operational configuration.

### 13.3 Standards and guidance

The project may use the following as engineering references, where applicable:

- MISRA C and MISRA C++ coding guidance.
- IEC 61508 functional-safety lifecycle concepts.
- ISO 26262 safety-engineering concepts as a learning reference only.
- OWASP secure-design and input-validation principles.
- Official STM32, FreeRTOS, MAVLink, Gazebo, and toolchain documentation.

This POC does not claim conformance, certification, or compliance with these standards.

## 14. Acceptance Criteria

The POC shall be considered complete for its defined software and bench-validation scope when all applicable criteria below are met:

| ID | Acceptance criterion | Evidence |
|---|---|---|
| AC-001 | Requirements are uniquely identified and traceable to design and tests. | Traceability matrix |
| AC-002 | Portable flight-control core builds successfully for the host target. | CI build log |
| AC-003 | STM32H743 target firmware builds successfully using the pinned toolchain. | CI build log and artifact |
| AC-004 | Mandatory unit tests pass. | Test report |
| AC-005 | Static-analysis gates pass or have formally reviewed and accepted deviations. | Analysis report |
| AC-006 | State-machine, sensor-validation, controller, mixer, and safety tests pass. | Test report |
| AC-007 | SITL can exchange simulated sensor inputs and actuator outputs through the documented bridge. | SITL execution report |
| AC-008 | Defined simulation scenarios execute repeatably and produce recorded results. | Scenario reports |
| AC-009 | Hardware startup and actuator-inhibit behavior are verified with propulsion disabled. | Bench report |
| AC-010 | CI produces versioned artifacts with source and toolchain metadata. | Release record |
| AC-011 | Known limitations, unresolved risks, and deferred requirements are documented. | Final POC report |

Passing these criteria does not establish that the aircraft is safe or ready for flight.

## 15. Project Milestones

| Milestone | Scope | Exit evidence |
|---|---|---|
| M1 — Firmware foundation | Repository, toolchain, startup, FreeRTOS, HAL boundaries, basic drivers, CI | Target build, host build, initial tests |
| M2 — Control and simulation | Estimation, state machine, controller, mixer, simulator bridge | Unit/integration/SITL reports |
| M3 — Hardware integration | Sensor interfaces, telemetry, HIL groundwork, bench validation | Hardware integration and bench reports |

## 16. Risks and Open Decisions

The following items must be resolved during architecture and detailed design:

| ID | Open item / risk | Required resolution |
|---|---|---|
| OD-001 | Exact IMU, magnetometer, barometer, and GNSS part numbers | Select devices and verify datasheets and availability |
| OD-002 | NUCLEO-H743ZI2 board revision and peripheral pin mapping | Verify board documentation and schematic |
| OD-003 | SPI/I2C/UART/ADC/timer allocation | Produce pin and peripheral mapping |
| OD-004 | DMA and Cortex-M7 cache policy | Define memory regions, alignment, and cache maintenance |
| OD-005 | Sensor sample rates and control-loop periods | Analyze sensor capabilities and measured timing |
| OD-006 | Attitude-estimation algorithm | Select and validate algorithm in simulation |
| OD-007 | PID gains, limits, and anti-windup parameters | Tune in simulation; document configuration |
| OD-008 | ESC protocol and electrical interface | Select ESC and verify its documented input requirements |
| OD-009 | Quad-X motor numbering and rotation directions | Freeze mixer convention and test mapping |
| OD-010 | MAVLink message subset and GCS compatibility | Define dialect/message support and run interoperability tests |
| OD-011 | Gazebo bridge implementation | Define transport, frames, units, timestamps, and adapter |
| OD-012 | Numeric timing, memory, and coverage targets | Establish baselines through analysis and measurement |
| OD-013 | Hardware-disable implementation | Design and independently verify the motor-disable path |
| OD-014 | Battery monitoring thresholds | Derive from selected battery and power-system specifications |

## 17. Requirements Traceability

A requirements traceability matrix (RTM) shall be maintained with at least these columns:

| Requirement ID | Requirement summary | Design component | Test case ID | Status | Evidence |
|---|---|---|---|---|---|
| FR-002 | Outputs inhibited during startup | Safety manager / actuator gate | TC-BOOT-001 | Planned | Pending |
| FR-012 | Validate sensor freshness and range | Sensor validation | TC-SENS-001 | Planned | Pending |
| FR-030 | Explicit flight states | Flight-state manager | TC-STATE-001 | Planned | Pending |
| FR-043 | Quad-X motor mixing | Motor mixer | TC-MIX-001 | Planned | Pending |
| FR-060 | Independent motor disable | Hardware safety path | TC-SAFE-001 | Planned | Pending |
| FR-070 | Host-buildable portable core | Portable application core | TC-CI-001 | Planned | Pending |

The RTM shall be expanded to cover every requirement before verification sign-off. A requirement is not verified merely because its implementation exists.

## 18. Definition of Done

A requirement or feature is complete only when:

- Its implementation and interface are reviewed.
- Relevant unit and integration tests are present and passing.
- Static-analysis findings are resolved or formally dispositioned.
- Requirements-to-test traceability is updated.
- Relevant design and user documentation is updated.
- CI checks pass.
- Test evidence is retained.
- Safety impact and failure behavior have been reviewed where applicable.

## 19. Glossary

| Term | Definition |
|---|---|
| DFCS | Drone Flight Control System |
| MCU | Microcontroller Unit |
| RTOS | Real-Time Operating System |
| HAL | Hardware Abstraction Layer |
| IMU | Inertial Measurement Unit |
| GNSS | Global Navigation Satellite System |
| ESC | Electronic Speed Controller |
| GCS | Ground Control Station |
| MAVLink | Lightweight messaging protocol for drones and related systems |
| SITL | Software in the Loop |
| HIL | Hardware in the Loop |
| PID | Proportional-Integral-Derivative controller |
| RTM | Requirements Traceability Matrix |

---

## Appendix A — Recommended Repository Layout

```text
drone-flight-control/
├── docs/
│   ├── requirements/
│   │   └── Drone_Flight_Control_System_SRS.md
│   ├── architecture/
│   ├── design/
│   ├── test-strategy/
│   ├── test-specifications/
│   └── test-reports/
├── firmware/
│   ├── application/
│   ├── control/
│   ├── estimation/
│   ├── navigation/
│   ├── safety/
│   ├── communication/
│   ├── drivers/
│   ├── hal/
│   └── rtos/
├── simulation/
│   ├── gazebo/
│   ├── bridge/
│   └── scenarios/
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── sitl/
│   ├── hil/
│   └── automation/
├── tools/
├── cmake/
├── .github/
│   └── workflows/
├── CMakeLists.txt
├── AGENTS.md
└── README.md
```

## Appendix B — Document Change History

| Version | Date | Description |
|---|---|---|
| 1.0 | 2026-11-02 | Initial POC SRS based on finalized architecture baseline |
