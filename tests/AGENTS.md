# tests

Layout: `unit/`, `integration/`, `sitl/`.

- Name tests `TC-<AREA>-NNN`; areas: BOOT, SAFE, SENS, STATE, CTRL, MIX, COMM, SIM, CI (SRS 11.3).
- Cover the minimum areas in SRS 11.2.
- Fault-injection and malformed/unsupported MAVLink input cases are required (FR-054, NFR-016).
- Procedures and expected results live in the test specification, not in test code.
