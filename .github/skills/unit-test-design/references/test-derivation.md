# Test derivation reference

## SRS 11.2 minimum test areas
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

Pick the areas the design touches. Areas 10 to 12 are mostly integration, SITL, or CI: list them as out of scope.

## Test case ID areas
`BOOT, SAFE, SENS, STATE, CTRL, MIX, COMM, SIM, CI` (SRS 11.3, tests/AGENTS.md). The backlog also uses `EST` and `RTOS`. Get a free ID with `scripts/next_tc_id.py <AREA>`.

## Hard rules and the negative case each needs
| Rule | IDs | Negative case |
|---|---|---|
| Outputs inhibited by default | FR-002, FR-061 | Initial state and every non-armed state report outputs not eligible/inhibited. |
| No auto re-arm | FR-067 | From FAILSAFE and FAULT, an arm request (and every event not in the table) never reaches ARMED. |
| MAVLink input rejected, no actuator change | FR-051, FR-054, NFR-016 | Bad length, unknown source, unsupported ID, invalid parameter: rejected, mocked actuator interface records zero calls. |
| No flightworthiness claim | NFR-017 | No spec text states or implies readiness. |

Apply only the rows the design touches.

## Derivation per requirement
1. **Nominal**: one case per permitted behaviour (for example each permitted table row).
2. **Boundary**: below, at, and above each limit; first and last enum value; empty and full capacity. Value comes from SRS or design, else `TBD (...)`.
3. **Fault**: each fault path in the design's fault-handling section (denied guard, invalid input, stale or missing data, dependency failure returned by a mock).
4. **Negative**: each hard-rule violation; each pair or input outside the permitted set, including out-of-range enum values.
5. Merge two cases only when setup and oracle are identical.

## Unit level scope
- In: host-runnable GoogleTest (C++) or Unity (C) tests of one unit.
- Out (list with TC ID in the reply): integration, SITL, HIL, bench, static analysis or CI checks such as `TC-CI-002`, review-only requirements (NFR-006, NFR-009, NFR-011).
- Core code has no HAL or FreeRTOS dependency (NFR-008); a case that needs one is mis-scoped.

## Isolation: the `mocks` column
Every collaborator, library, driver, HAL, RTOS, clock, and I/O the unit touches appears as `<dependency>: <mocked behaviour>`, for example `IGuardProvider: evaluate() returns false for ArmPreconditions`. Use `none` only for a dependency-free unit. No real hardware, files, network, threads, timers, or third-party calls.

## Values
Never invent gains, mixer coefficients, timing, tolerances, or thresholds. Quote from SRS or design, otherwise write `TBD (design decision)` or `TBD (measured)` with the resolving story or OD-xxx (see SRS section 16, OD-001 to OD-014; OD-005 rates, OD-007 gains, OD-009 motor numbering, OD-012 numeric targets).

## RTM rows
Every `NEW` case needs a row added to the SRS traceability matrix: requirement, TC ID, status Planned. List them in the reply; do not edit the SRS.
