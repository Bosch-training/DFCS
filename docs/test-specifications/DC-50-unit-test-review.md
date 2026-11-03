Verdict: PASS
# DC-50: Unit Test Spec Review (round 2)
Spec: docs/test-specifications/DC-50-unit-test-spec.csv  Design: docs/design/DC-50-pid-controller.md  Cases: 35

Format: header and 14 columns on every row, RFC 4180 quoting, enums, unique `tc_id` (TC-CTRL-002 `Existing`, TC-CTRL-007..040 `NEW`, contiguous), requirement IDs exist in the SRS. `NEW` IDs do not collide with the backlog (TC-CTRL-001..006) or SRS 11.3 (TC-CTRL-001). All expected values were re-derived by hand from design 4.2 (including the new cases 031..040 and the reworked 018, 020, 021) and match. `mocks = none` is valid: design 4.1 says `Pid` has no ports and no dependencies. Every case is GoogleTest on a C++17 core class with no HAL, RTOS, file, thread, or timer use (NFR-008).

## Round 1 findings

| Round 1 # | tc_id | Status | Evidence in the revised spec |
|---|---|---|---|
| 1 (major) | TC-CTRL-018 | Resolved | Reconfigures to `kd=2`, expects output 5; a kept previous error of 3 would give 13, so stale derivative history is now observable. |
| 2 (minor) | TC-CTRL-020 | Resolved | Cites FR-040 only. |
| 3 (minor) | TC-CTRL-017, 019, 020 | Resolved | FR-047 no longer cited; notes defer to CTL-04. |
| 4 (minor) | TC-CTRL-021, 026 | Resolved | FR-046 no longer cited; notes defer to CTL-03. |
| 5 (minor) | TC-CTRL-027, 028 | Resolved | FR-042 no longer cited; notes defer to CTL-02. |
| 6 (minor) | TC-CTRL-002 | Resolved | TC-CTRL-002 notes name 007..040 as its sub-cases, and each of those rows says "Sub-case of TC-CTRL-002 (P/I/D/combined/reset/configure/reject)". The backlog "Done when" is traceable. |
| 7 (minor) | 016, 019, 027, 029, 030 | Resolved | Split into 016/031, 019/032/033, 027/034/035/028, 029/036/037, 030/040. Remaining multi-value cases (022, 024, 026) are data variants of one behaviour. |
| 8 (minor) | TC-CTRL-021 | Resolved | No longer reads `integral()` on an unconfigured instance. |
| 9 (minor) | TC-CTRL-022..029 | Resolved | TC-CTRL-038 and 039 check D = 0 after a rejected first update. |

## New findings

| # | tc_id | severity | check | finding | source |
|---|---|---|---|---|---|
| 1 | TC-CTRL-031, 032, 033 | minor | 8 Quality | Ordering: these IDs sit in the middle of the file (after 016 and 019), so the file is not in ID order. No gap is hidden. | Mode check 8 |

## Validator errors (6): scope exclusion, not a gap

`validate_spec.py` exits 1 with 35 cases, 6 errors, 16 warnings.

| Validator error | Design treatment | Verdict impact |
|---|---|---|
| FR-042 no test case | Design 1 "Not in scope" (CTL-02); design 2 row "Not in this story. Owner CTL-02" (TC-CTRL-001, 003); design 3 out of scope (limits, anti-windup); design 5 "Not covered here". | None |
| FR-046 no test case | Design 2 row "FR-041, FR-046": gating and the "invalid" output belong to CTL-03 (TC-CTRL-004); design 1 (CTL-03). | None |
| FR-047 no test case | Design 2 row "Not in this story. Owner CTL-04. Only a finite-value check here" (TC-CTRL-005); design 1 (CTL-04). | None |
| FR-051, FR-054, NFR-016 no negative case | Design 4.3: "MAVLink ... not touched. This story adds no command path and no way to change gains from external input." There is no design 2 row for them. | None |

Reasoning:
1. The authoritative scope is the design plus the backlog. Backlog CTL-01 lists FR-040 only. FR-042 is CTL-02, FR-041/FR-046 are CTL-03, and FR-047 is CTL-04 (backlog "Epic CTL"). The design repeats this in four places (sections 1, 2, 3, 5), each with an owner story and a TC ID. That is a documented exclusion, not an omission.
2. Mode check 3 requires coverage of "every design requirement" the design assigns to this story. The design's own coverage claim for this story is FR-040 (plus local FR-041 rejection). Both are covered (35 cases).
3. Mode check 4 applies hard rules "where touched". Design 4.3 states FR-051, FR-054, and NFR-016 are not touched. Adding MAVLink-labelled cases to a `Pid` unit would claim false RTM coverage of P0 requirements (the round 1 finding 2 concern). The hard rules that are touched, FR-002/FR-061 (TC-CTRL-016, 021, 031) and FR-067 (TC-CTRL-016, 031), have negative cases.
4. Cause of the errors: the validator treats every ID in design section 2 col 1, and every ID mentioned in 4.3, as "assigned to this story". Its out-of-scope detection only matches `review`, `TC-CI-`, or `documentation` in the last column, and does not recognise "Not in this story / owner <story>" or "not touched". The errors are a validator heuristic limit.

Impact: no blocker or major finding follows from the 6 errors, so the verdict is not changed by them. If automation gates on the validator exit code, it will still see exit 1 for this spec; that needs a validator or design-format decision, not a spec change (open question 1).

## Uncovered requirements
- None at unit level. FR-040 is covered by TC-CTRL-002 and 007..040.
- Routed by the design to review or CI: NFR-001, NFR-005, NFR-008, NFR-009, NFR-010, NFR-011, NFR-012, NFR-017, FR-070 (TC-CI-002, TC-CI-004, FND owners).
- Owned by other stories, deliberately not cited: FR-042 (CTL-02, TC-CTRL-001/003), FR-041 gating and FR-046 (CTL-03, TC-CTRL-004), FR-047 (CTL-04, TC-CTRL-005), OD-007 (SIM-13, TC-CTRL-006).
- Not touched by this design: FR-051, FR-054, NFR-016 (design 4.3).

## Open questions
1. Validator exit 1: should `validate_spec.py` learn to skip design rows marked "Not in this story / Owner <story>" and 4.3 "not touched" mentions, or should the design reword them? Owner: Unit Test Design skill maintainer. Until then, Unit Test Automation should not gate on the validator exit code for DC-50.
2. FR-041 is cited as "local only" in cases 022 and 026 notes but not in 023, 024, 029, 036, 038, 039, 040. Design 5 lists FR-041 "(local only)" for the whole reject group, so this is allowed; confirm the RTM records FR-041 as partial. Owner: FND-08.
