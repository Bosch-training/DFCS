# firmware/rtos

FreeRTOS tasks, scheduling, and watchdog supervision.

- Define task priorities, periods, deadlines, stack sizes, and synchronization in the detailed design before use; do not invent values (NFR-002).
- Supervise task health with the watchdog (FR-064).
- Measure and report execution time and scheduling behavior for critical tasks (NFR-003).
- Keep communication parsing from blocking time-critical control execution (FR-055).
- No blocking console, file, or network I/O in control tasks (NFR-001).
- No heap allocation after init in runtime paths (NFR-005).
