Verdict: PASS

# DC-17: Unit Test Spec Review (round 2)
Spec: docs/test-specifications/DC-17-unit-test-spec.csv  Design: docs/design/DC-17-states-and-transition-table.md  Cases: 24

Findings: 0 blocker, 0 major, 4 minor.

Format check passed: header exact, 14 columns on every row, valid quoting, no empty cells, `status` (Existing, NEW), `type` (nominal, negative, fault, boundary) and `framework` (GoogleTest) valid, 24 unique `tc_id`s in ascending order. `Existing` IDs (TC-STATE-002, TC-STATE-003) are in the backlog (STA-01 "Done when") and design section 5. NEW IDs TC-STATE-008 to 029 do not collide with any ID in the SRS, backlog, or design. All requirement IDs (FR-002, FR-030, FR-031, FR-051, FR-054, FR-061, FR-067, NFR-016) exist in the SRS. Expected `behavior` values match SRS 10 "Required behavior" as mapped in design 4.2.

## Round 1 findings

| Round 1 # | Status | Evidence |
|---|---|---|
| 1 (TC-STATE-017 asserts table internals) | Fixed | Public API only: Accepted count, per-pair outcome, distinct `row` values. |
| 2 (TC-STATE-018 asserts table internals) | Fixed | Observes ARMED entry through `state()` over 54 pairs. |
| 3 (TC-STATE-020 second guard behaviour missing in `mocks`) | Fixed | `mocks` lists the denying fake per row and the allowing fake on `GuardId::Arm`. |
| 4 (TC-STATE-021 `mocks` none) | Fixed | `mocks` lists the allowing fake for the D8 machine; `setGuard` is never called with null. |
| 5 (TC-STATE-003 non-deterministic start state) | Fixed | Fake wired before forcing the start state; call record reset afterwards. |
| 6 (`DenyAllGuard` not exercised) | Fixed | TC-STATE-028 and TC-STATE-029 added. |
| 7 to 9, 12 (minor) | Fixed | TC-STATE-016 title, TC-STATE-027 preconditions, TC-STATE-023 notes, row order. |
| 10, 11 (minor) | Open | See findings 3 and 4 below. |

## Findings

| # | tc_id | severity | check | finding | source |
|---|---|---|---|---|---|
| 1 | TC-STATE-023 | minor | 3 | Only the upper bound of the event range is covered (last enumerator, one past it). A value below the first enumerator has no case. The notes say the underlying type is unspecified, but design 4.2 step 1 rejects any event "outside the `FlightEvent` enumerators", which includes values below the first. | Design 4.2 step 1, 4.3; spec TC-STATE-023 notes |
| 2 | TC-STATE-017, TC-STATE-018 | minor | 5 | Step 2 says "as in TC-STATE-003 step 3" instead of listing the forcing paths. Cases should be readable and runnable alone. | Spec TC-STATE-017 and TC-STATE-018 step 2; tests/AGENTS.md |
| 3 | TC-STATE-002 | minor | 2 | Backlog "Done when" and the design RTM name TC-STATE-002 as covering every permitted transition. The spec narrows it to D1 and puts D2 to D9 under NEW IDs. The parent link is carried only in the notes. Carried over from round 1 (finding 10). | Backlog STA-01 "Done when"; design sections 5 and 6; spec TC-STATE-002 notes |
| 4 | TC-STATE-016, 017, 018, 024, 025, 027 | minor | 8 | Partial overlap with TC-STATE-003 and each other (rejected pairs, never-ARMED). The overlap is declared in the notes as hard-rule redundancy and hides no gap. Carried over from round 1 (finding 11). | Spec notes; design 5 |

## Uncovered requirements

None uncovered at unit level. Design requirements without a case in this spec are routed by the design to other verification methods:
- NFR-005, NFR-008: TC-CI-002 (design section 2).
- FR-070: TC-CI-004.
- NFR-006, NFR-009: review.
- NFR-017: documentation review. No meaningful unit-level negative case exists; accepted as out of scope.

Hard-rule negative cases present: FR-002 and FR-061 (TC-STATE-016, 021, 027, 029), FR-067 (TC-STATE-014, 018, 020, 021, 024, 025, 029), FR-051, FR-054, NFR-016 (TC-STATE-022, 023).

Design faults and boundaries covered: all 9 permitted rows (D1 to D4, D6 to D9 with D7a and D7b), all 45 non-permitted pairs, guard denied (explicit reason and unwired), `DenyAllGuard`, out-of-range event, the three distinct rejection outcomes. Not covered, with a recorded reason: out-of-range stored state (unreachable through the public API, TC-STATE-023 notes).

## Validator coverage errors (FR-001, FR-003, FR-032 to FR-036, FR-046, FR-063, FR-066)

Accepted by the user as a validator false positive from design section 2.1. Not counted as findings. Design sections 1 and 2.1 place these requirements in other stories (STA-02 to STA-06, SAF-01, SAF-03 to SAF-05).

## Open questions

1. Is a value below the first `FlightEvent` enumerator reachable and meant to be `RejectedInvalidInput`? If yes, add a lower-bound case (finding 1).
2. Should the backlog and RTM name TC-STATE-008 to 015 as children of TC-STATE-002 (finding 3)?
3. Design 4.1 names both a null-pointer default and a `DenyAllGuard` stub. Which backs D4, D8 and D9 after construction? TC-STATE-021 and TC-STATE-029 cover both, so the spec holds either way.
