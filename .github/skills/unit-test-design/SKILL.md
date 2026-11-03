---
name: unit-test-design
description: 'Produce a DFCS unit test specification CSV from a design doc. Use when given a Jira ID (e.g. DC-12) or a docs/design file and asked for a unit test spec, test cases, test design, TC IDs, or requirement-to-test coverage. Reads the doc made by Requirement Analysis; writes one CSV to docs/test-specifications; validates it with a script. Hands off to Unit Test Reviewer.'
argument-hint: 'Jira ID or design doc path, e.g. DC-12'
---

# Unit Test Design

Turn one design doc into a unit test specification for DFCS (simulation-first POC quad-X, STM32H743). Every requirement gets cases that prove it, including boundary and fault cases. Never invent values the design or SRS defers.

## Resources
- [CSV format](./references/csv-format.md) and [schema](./assets/unit-test-spec.schema.json): columns, enums, patterns.
- [Test derivation](./references/test-derivation.md): SRS 11.2 areas, hard-rule negative cases, scope, mocks, values.
- [Template](./assets/unit-test-spec.template.csv): header plus one example row.
- `scripts/next_tc_id.py <AREA> [COUNT]`: next free TC ID.
- `scripts/validate_spec.py <spec> [--design <doc>]`: format, ID, coverage, hard-rule, and unsourced-number checks.

## Sources
- `docs/design/<JIRA-ID>-*.md`: requirements table, modules, fault handling, test plan, open questions. Ask for the ID if missing. No design doc: stop and tell the user to run Requirement Analysis first.
- [SRS](../../../docs/Drone_Flight_Control_System_SRS.md) sections 11 and 17, [backlog](../../../docs/DFCS_Stories.md), [tests/AGENTS.md](../../../tests/AGENTS.md), `AGENTS.md` of the touched module folders, existing `docs/test-specifications/*`.

## Rules
- Write only `docs/test-specifications/<JIRA-ID>-unit-test-spec.csv` (create or update). No test code, source code, design edits, or Jira access.
- Unit level only: GoogleTest (host C++) or Unity (embedded C), host-runnable. Integration, SITL, and bench cases are listed as out of scope with their TC IDs.
- Each case verifies one observable behaviour of one unit (return value, output state, or call made through an interface), never private members or call order the design does not require.
- Isolate the unit: every collaborator, library, driver, HAL, RTOS, clock, and I/O is mocked or faked at its interface and named in `mocks`.
- Reuse TC IDs from the design and backlog. A new case takes the next free ID from `scripts/next_tc_id.py`, marked `NEW`. Never reuse an ID.
- Procedures and expected results live in the CSV, not in test code.
- No gain, mixer, timing, tolerance, or threshold values unless quoted from the SRS or design. Otherwise `TBD (design decision)` or `TBD (measured)` plus the resolving story or OD-xxx.
- Cover the hard rules the design touches: outputs inhibited by default (FR-002, FR-061); no auto re-arm (FR-067); malformed, unsupported, invalid MAVLink input rejected with no actuator change (FR-051, FR-054, NFR-016).
- Fault-injection and boundary cases are required.
- No flightworthiness claim (NFR-017).
- Cite only SRS, story, OD, and TC IDs that exist in their source documents.

## Procedure
1. Read the design doc. Collect every requirement ID, module, interface, and fault path.
2. Pick the SRS 11.2 test areas the design touches ([derivation](./references/test-derivation.md)).
3. Per requirement derive: nominal case, each boundary, each fault path, each hard-rule violation. Merge only cases that share setup and oracle.
4. Get IDs for new cases: `python3 .github/skills/unit-test-design/scripts/next_tc_id.py STATE 3`.
5. Draft from the [template](./assets/unit-test-spec.template.csv) following [CSV format](./references/csv-format.md). Check:
   - every design requirement has at least one case;
   - every case has requirement, inputs, steps, and an observable expected result;
   - every case states a behaviour, not an internal mechanism;
   - every dependency is in `mocks` with the mocked behaviour the case relies on;
   - every design fault path and hard rule has a negative case;
   - no unsourced numbers;
   - design gaps go in the reply and `notes`, not silently resolved.
6. Write the CSV, then validate:
   `python3 .github/skills/unit-test-design/scripts/validate_spec.py docs/test-specifications/<JIRA-ID>-unit-test-spec.csv`
   Fix every `ERROR`. Review each `WARN`; keep a warning only if justified.

## Reply
File path, case count, requirement-to-TC coverage list, uncovered requirements, out-of-scope TCs, RTM rows to add (every `NEW` case), open questions, validator result. These stay in chat; the CSV holds cases only. Suggest the Unit Test Reviewer agent next.

## Stop and ask
No design doc found, design Status is empty or contradicts the SRS, or a requirement has no observable pass/fail oracle. Smaller gaps go in the reply.
