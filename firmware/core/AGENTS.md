# firmware/core

Portable flight logic: estimation, state machine, PID, mixer, safety gate.

- Do not include STM32 HAL or FreeRTOS headers (NFR-008).
- No heap allocation after init (NFR-005).
- No blocking console, file, or network I/O (NFR-001).
- Validate sensor freshness, range, and plausibility before control use (FR-012, FR-041).
- Change flight state only through the explicit transition table (FR-031).
- Keep mixer signs and motor order in configuration, not inline (FR-045).
- All actuator output passes the safety gate, which can inhibit independently of control (SRS 4.3, FR-046).
- Must build for the desktop host (FR-070).
