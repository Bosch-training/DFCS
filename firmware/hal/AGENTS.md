# firmware/hal

Hardware abstraction and STM32H743 platform code.

- Implement the same logical interfaces the simulation adapter implements (FR-071).
- Place DMA buffers and perform D-cache maintenance explicitly (NFR-007).
- Document ownership and synchronization for data shared between ISRs and tasks (NFR-006).
- Keep the hardware motor-disable path independent of control code (FR-060).
- Bench work: no motors connected, propellers removed, power isolated (SRS 9.2).
