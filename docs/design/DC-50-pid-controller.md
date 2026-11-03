# DC-50: CTL-01 PID controller — Design

| Field | Value |
|---|---|
| Jira | DC-50: CTL-01: PID controller (single-axis PID with configurable gains and a fixed-step `update(dt)`) |
| Story | CTL-01, S, P0 (from backlog) |
| SRS requirements | FR-040; constraints: NFR-001, NFR-005, NFR-008, NFR-009, NFR-010, NFR-011, NFR-012, NFR-017, FR-070 |
| Depends on | FND-03 (DC-9) |
| Blocks | CTL-02 (direct); CTL-03, CTL-04, APP-01, SIM-12, SIM-13 (transitive) |
| Status | Draft |

## 1. Summary and scope

CTL-01 adds one portable, host-buildable single-axis PID class to `dfcs_core`. It takes a gain set and, on each fixed step, turns an error and a step length `dt` into a PID output.

In scope:
- Gain storage, accepted only if finite.
- `update(error, dt)` computing P, I, and D terms.
- `reset()` of the dynamic state.
- Fail-closed rejection of invalid input, with distinct statuses.
- Unit tests that match reference responses for P, I, and D separately (`TC-CTRL-002`).

Not in scope: output limits and anti-windup (CTL-02), input gating from estimator and setpoint validity (CTL-03), gain and limit validation and versioning (CTL-04), gain values (SIM-13, OD-007), and mixing, safety gating, and actuator output (MIX, SAF).

The PID produces a demand value only. It has no actuator, arm-state, or MAVLink access.

## 2. Requirement analysis

| ID | Short text | Source | How addressed | TC ID |
|---|---|---|---|---|
| FR-040 | Configurable PID control for the selected attitude-control axes | SRS 7.5; backlog CTL-01 | A `Pid` class with run-time gains and `update(error, dt)`. Axis selection and per-axis instances belong to CTL-03. This story delivers the single-axis building block (4.1, 4.2). | TC-CTRL-002 (unit); SITL part owned by SIM-12 (TC-CTRL-006) |
| NFR-008 | Core must not depend on STM32 HAL or FreeRTOS | SRS 8; `firmware/core/AGENTS.md` | Standard C++17 only. No platform headers. Banned-API check is FND-07 (TC-CI-002). | TC-CI-002 (owner FND-07) |
| NFR-005 | No dynamic allocation in runtime paths | SRS 8 | Only fixed-size members. No heap, no containers. | TC-CI-002 (owner FND-07); review |
| NFR-001 | Bounded execution, no blocking I/O | SRS 8 | Straight-line code with no loops, I/O, or blocking. Execution time is TBD (measured) (4.4). | Review; timing owned by FWT-19 |
| NFR-009 | Documented interfaces, clear mutable-data ownership | SRS 8 | Ownership defined in 4.1. | Review |
| NFR-010, NFR-012 | Reproducible build and tests; coding standard | SRS 8 | Tests registered with CTest using the FND-03 harness. Format and static analysis via FND-04. | TC-CI-004 (owner FND-01) |
| FR-070 | Core builds for the desktop host | SRS 7.8 | The new file is added to `dfcs_core`. | TC-CI-004 (owner FND-01) |
| NFR-011 | Requirement traceability | SRS 8, 17 | RTM rows in section 6. | Traceability review |
| NFR-017 | No flightworthiness claim | SRS 8 | No such claim is made in code or docs. | Documentation review |
| FR-041, FR-046 | Valid inputs only; invalid input inhibits or gives a safe output | SRS 7.5 | Not enforced here, except the local rejection of non-finite inputs (4.3). Gating and the "invalid" output belong to CTL-03. | TC-CTRL-004 (owner CTL-03) |
| FR-042 | Output limits and anti-windup | SRS 7.5 | Not in this story. Owner CTL-02. A hook is left (4.1). | TC-CTRL-001, TC-CTRL-003 (owner CTL-02) |
| FR-047 | Gains and limits validated and versioned | SRS 7.5 | Not in this story. Owner CTL-04. Only a finite-value check here. | TC-CTRL-005 (owner CTL-04) |
| OD-007 | Gain, limit, and anti-windup values | SRS 16 | No values defined. The class has no default gains. Owner SIM-13. | TC-CTRL-006 (owner SIM-13) |

## 3. Assumptions, constraints, out of scope

- FND-03 is satisfied: GoogleTest, Unity, and pytest smoke tests exist in `tests/unit`.
- Only FND-03 is a dependency. COR-01 (time types) and COR-04 (config container) do not exist yet. The PID therefore takes plain `float` values and a plain gain struct. COR-04 may later wrap or replace the gain struct. Names below are proposed and a later design may rename them.
- `dt` is in seconds and is supplied by the caller. The control period is not defined here (OD-005, DSN-02).
- Gains are supplied by the caller. There are no defaults. An unconfigured instance gives no usable output.
- The controller is single-axis and unit-agnostic. Angle wrap (yaw), units, and setpoint minus measurement are handled by the caller (CTL-03).
- Out of scope: limits, anti-windup, gain bounds, versioning, input gating, derivative filtering, feed-forward, and gain scheduling.
- The `firmware/core` layout is used, as in the backlog. SRS Appendix A shows `firmware/control`, which is not used.

## 4. Design

### 4.1 Modules and interfaces (ports, mutable-data ownership)

Proposed files (names may change):
- `firmware/core/src/pid.hpp`
- `firmware/core/src/pid.cpp`
- Add `pid.cpp` to `dfcs_core`. Expose the header to tests through the core target include path.

Proposed interface (namespace `dfcs::core`):

```cpp
struct PidGains { float kp; float ki; float kd; };

enum class PidStatus {
  Ok,
  NotConfigured,  // update() before a successful configure()
  InvalidConfig,  // configure() got a non-finite gain
  InvalidDt,      // dt not finite or dt <= 0
  InvalidInput,   // error not finite
  NumericFault    // a result was not finite
};

struct PidResult { PidStatus status; float output; };  // output is 0.0f unless status == Ok

class Pid {
 public:
  PidStatus configure(const PidGains& gains);
  PidResult update(float error, float dt_s);
  void reset();
  bool configured() const;
  float integral() const;
  const PidGains& gains() const;
};
```

Mutable-data ownership:
- Each `Pid` instance is the sole owner and sole writer of its gains, `integral_`, `prev_error_`, `has_prev_`, and `configured_`.
- Only `configure()`, `update()`, and `reset()` write them. There are no raw setters.
- Readers use the const accessors.
- A `Pid` instance has one owning caller (CTL-03). Sharing across tasks is not supported (4.4).

Ports: none. The class does not use `IClock`, `IActuator`, or `IFaultSink`. The caller supplies `dt`. Fault reporting is the caller's job: CTL-03 maps statuses to the COR-03 registry. This follows the DSN-01.3 convention once DSN-01 exists.

Hooks owned by other stories:
- CTL-02 limits and anti-windup: CTL-02 will extend `update()` with limit handling on the integral and output. CTL-01 does not clamp. Its tests assume values stay in range.
- CTL-03 input gating: CTL-03 checks estimate and setpoint validity before calling `update()` and calls `reset()` on invalid input. CTL-01's own checks are a last line of defence, not the gating.

### 4.2 Data flow and state interaction

Data flow (SRS 4.3): `State Estimation -> Controller (this PID, via CTL-03) -> Safety Output Gate -> Quad-X Mixer -> Actuator`. The PID output is a demand only. The safety gate can inhibit independently of the PID. This story touches no state-machine state.

SRS tables: the SRS 10 state model and SRS tables are not touched, so no SRS rows are mapped. Behavior rows below are numbered so the test plan can trace to them.

| # | Behavior | Requirement |
|---|---|---|
| B1 | Output is `P + I + D` with `P = kp*e`, `I = integral`, `D = kd*(e - e_prev)/dt` | FR-040 |
| B2 | Integral accumulates `ki*e*dt` per accepted step, in output units | FR-040 |
| B3 | D term is 0 on the first accepted step after `configure()` or `reset()` | FR-040 |
| B4 | Gains are set at run time with `configure()`, and `configure()` clears state | FR-040 (configurable) |
| B5 | `reset()` clears integral and derivative history, keeps gains | FR-040; supports CTL-03 |
| B6 | Invalid dt, invalid error, or unconfigured: rejected, no state change | Fail closed (4.3) |

Algorithm for `update(error, dt_s)`:
1. If not configured, return `{NotConfigured, 0}`.
2. If `dt_s` is not finite or `dt_s <= 0`, return `{InvalidDt, 0}`.
3. If `error` is not finite, return `{InvalidInput, 0}`.
4. Compute into temporaries: `p = kp*error`, `i_new = integral + ki*error*dt_s`, `d = has_prev ? kd*(error - prev_error)/dt_s : 0`.
5. If `p`, `i_new`, `d`, or `p + i_new + d` is not finite, return `{NumericFault, 0}` with state unchanged.
6. Commit `integral = i_new`, `prev_error = error`, `has_prev = true`.
7. Return `{Ok, p + i_new + d}`.

`configure(gains)`: if any gain is not finite, return `InvalidConfig` with gains and state unchanged. Otherwise store the gains, run `reset()`, set `configured = true`, and return `Ok`.

`reset()`: `integral = 0`, `prev_error = 0`, `has_prev = false`. It does not change gains or `configured`.

Proposed design choices (see open questions 2 to 5):
- Derivative on error, backward difference, no filter.
- Forward-Euler (rectangular) integration with the current error.
- The integral is held in output units, so the `ki` factor is applied at accumulation. This lets CTL-02 clamp it directly.
- `float` as the numeric type.

### 4.3 Error, fault, and failsafe handling

- Outputs inhibited by default (FR-002, FR-061): the PID has no actuator access and cannot enable output. An unconfigured instance returns `NotConfigured` with output 0, which callers must not treat as a usable command. The "unconfigured, inhibit" state matches COR-04. Inhibit enforcement belongs to the safety gate (SAF).
- No automatic re-arm (FR-067): the PID holds no arm or failsafe state. `reset()` only clears control state. It is not a recovery action and enables nothing. Recovery conditions belong to STA and SAF.
- MAVLink (FR-051, FR-054, NFR-016): not touched. This story adds no command path and no way to change gains from external input. If gains are later set from MAVLink, that path must go through CTL-04 validation and COM command handling.
- Invalid input: non-finite `error`, non-finite or non-positive `dt`, or non-finite gains are rejected. The state does not change on any rejection (B6).
- Distinct outcomes: a test can assert each of `NotConfigured`, `InvalidConfig`, `InvalidDt`, `InvalidInput`, and `NumericFault` separately.
- Unwired dependencies: there are none. The PID needs no ports. An instance that was never configured is the only "unwired" case, and it is rejected.
- Out of scope: whether the estimate or setpoint is valid (CTL-03), whether the output is within limits (CTL-02), and whether the gains are sensible (CTL-04). A finite but wrong gain, or a `dt` that is finite but implausibly large, is not detected here (open question 6).

### 4.4 Timing, memory, concurrency (TBD only)

- Execution time of `update()`: TBD (measured). Owner FWT-19 and DSN-02 (OD-005, OD-012).
- Control period and maximum accepted `dt`: TBD (design decision). Owner DSN-02 (OD-005).
- Memory footprint of one `Pid`: TBD (measured). There is no heap use after init (NFR-005).
- Concurrency: one owner, no internal locking. Task ownership and synchronization are TBD (design decision), owner DSN-02 and APP-01 (NFR-006).

## 5. Test plan

Test code lives in `tests/unit/` and is registered in `tests/unit/CMakeLists.txt`, using GoogleTest, since the core is C++17. Expected values are derived analytically in the test from the formulas, not by calling the implementation. Test vectors and the comparison tolerance belong to the test specification. The tolerance is TBD (design decision). Exactly representable inputs should be preferred so that comparisons can be exact.

Sub-cases below are proposed under `TC-CTRL-002`. The test specification (FND-08) allocates the final IDs (open question 7).

| TC ID | Purpose | Framework | Requirement |
|---|---|---|---|
| TC-CTRL-002 (P) | Only `kp` non-zero: output equals `kp*e` for a sequence of errors, and the integral stays 0 (B1) | GoogleTest | FR-040 |
| TC-CTRL-002 (I) | Only `ki` non-zero: constant error gives a linear ramp `ki*e*n*dt` after `n` steps, and a sign flip reverses the ramp (B1, B2) | GoogleTest | FR-040 |
| TC-CTRL-002 (D) | Only `kd` non-zero: first step gives 0 (B3), a constant error then gives 0, and a ramp error gives `kd*slope` (B1, B3) | GoogleTest | FR-040 |
| TC-CTRL-002 (combined) | All three non-zero gives the sum of the separate terms | GoogleTest | FR-040 |
| TC-CTRL-002 (reset/configure) | `reset()` clears integral and derivative history and keeps gains. `configure()` clears state and replaces gains (B4, B5) | GoogleTest | FR-040 |
| TC-CTRL-002 (reject) | `NotConfigured`, `InvalidConfig`, `InvalidDt` (zero, negative, NaN, infinity), `InvalidInput` (NaN, infinity), and `NumericFault` (overflow) are each returned with output 0, and state is unchanged by the rejected call (B6) | GoogleTest | FR-040, FR-041 (local only) |
| TC-CI-002, TC-CI-004 | No banned APIs in `firmware/core`, and the clean host build includes the PID (owners FND-07, FND-01) | CI scripts | NFR-005, NFR-008, FR-070 |

Not covered here: `TC-CTRL-001` and `TC-CTRL-003` (CTL-02), `TC-CTRL-004` (CTL-03), `TC-CTRL-005` (CTL-04), and `TC-CTRL-006` (SIM-12 and SIM-13). SRS 11.2 item 6 (PID saturation, anti-windup, and boundary behavior) is only partly covered until CTL-02 is done.

## 6. Traceability (RTM rows to add or update)

The RTM (`docs/traceability/RTM.md`, FND-08) does not exist yet. When it exists, add or update:

| Requirement ID | Requirement summary | Design component | Test case ID | Status | Evidence |
|---|---|---|---|---|---|
| FR-040 | Configurable PID control | `Pid` (`firmware/core`, CTL-01) | TC-CTRL-002 (SITL: TC-CTRL-006) | Planned | Pending |
| NFR-008 | Core independent of HAL and RTOS | `Pid` uses no platform headers | TC-CI-002 | Planned | Pending |
| NFR-005 | No dynamic allocation | `Pid` has fixed-size members only | TC-CI-002 | Planned | Pending |
| NFR-009 | Documented interfaces and ownership | `Pid` ownership (4.1) | Design review | Planned | Pending |

## 7. Risks and open questions

Risks:
- Integrator windup before CTL-02. Mitigation: CTL-03 depends on CTL-02, so no actuator path can reach an unclamped PID. State this in the CTL-03 and CTL-02 designs.
- Derivative noise and derivative kick on a setpoint step, because the D term acts on error with no filter. Mitigation: documented as a known limitation. Revisit with simulation evidence (SIM-12, SIM-13).
- Floating-point rounding makes the reference comparison fragile. Mitigation: use exactly representable test values, and set the tolerance in the test specification.
- Convention drift: if CTL-02 needs the integral in error units, the accumulator choice must change. Mitigation: settle question 3 before CTL-02 starts.
- Missing prerequisites: no `docs/design/DSN-01` document, no test specification or RTM (FND-08), and no `COR-01` or `COR-04` types. Mitigation: the PID uses plain types and can be adapted once those exist.

Open questions:
1. Signature. Jira writes `update(dt)`. The design assumes `update(error, dt)`, because a PID needs an input. Confirm that the error is computed by the caller and not passed as setpoint and measurement. Owner: story author (CTL-01) with CTL-03.
2. Derivative form. Derivative on error versus derivative on measurement, and whether a filter is needed. Proposed: on error, no filter. Owner: CTL-03 design, with tuning evidence from SIM-13 (OD-007).
3. Integral accumulator. Proposed: accumulate `ki*e*dt`, held in output units. The alternative is to accumulate `e*dt` and multiply by `ki` on output. Owner: CTL-02 design.
4. Numeric type. Proposed: `float`. Confirm against estimator and config types. Owner: DSN-01 and COR-01.
5. Integration method. Proposed: forward Euler with the current error. Confirm that the reference responses should use this. Owner: CTL-01 reviewer.
6. Maximum accepted `dt` and whether a `dt` jitter check belongs in the PID. Left TBD. Owner: DSN-02 (OD-005).
7. Test IDs. `TC-CTRL-002` is only in the backlog and not in SRS 11.3, which lists `TC-CTRL-001` only. The backlog says allocations are suggestions. Confirm that rejection sub-cases stay under `TC-CTRL-002` or get their own ID. Owner: FND-08 test specification.
8. Input checks. Should the finite-value checks stay in the PID, or move to CTL-03 and CTL-04 only? They are kept here as fail-closed defence. Owner: CTL-03 and CTL-04 reviewers.
9. Header location. `firmware/core/src` is the only existing source folder. Confirm whether a public `include` folder is wanted. Owner: DSN-01.

## 8. Definition of Done checklist (SRS 18)

- [ ] Implementation and interface reviewed.
- [ ] `TC-CTRL-002` present and passing (P, I, D separately, plus combined, reset, and reject cases).
- [ ] Static-analysis findings resolved or dispositioned (FND-04 tooling).
- [ ] RTM rows in section 6 added, once the RTM exists (FND-08).
- [ ] Design documentation updated, and the test specification entry added for `TC-CTRL-002`.
- [ ] CI checks pass, including the banned-API check (FND-07).
- [ ] Test evidence retained.
- [ ] Safety impact reviewed: the PID is a demand source only and cannot enable output. Anti-windup and limits remain with CTL-02.
- [ ] No gain, timing, or memory values introduced. No flightworthiness claim (NFR-017).
