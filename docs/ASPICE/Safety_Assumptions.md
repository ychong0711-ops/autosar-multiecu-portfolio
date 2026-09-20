# Safety and Compliance Assumptions

## Explicit non-claims

This repository:

- is **not** ISO 26262 certified;
- is **not** an AUTOSAR conformance claim;
- is **not** suitable for controlling a real vehicle;
- does **not** model CAN arbitration, bit errors, bus-off recovery, clock drift, or multicore interference;
- uses an ASPICE-inspired document structure but has not undergone an Automotive SPICE assessment.

## Intended use

The project is an educational portfolio demonstrating requirements engineering, layered communication-stack understanding, deterministic fault injection, traceability, and a reproducible path to the TOPPERS open-source automotive stack.

## Residual risks

| Risk | Mitigation in this project | Remaining gap |
|---|---|---|
| Corrupt payload | DLC, range and sequence validation | No CRC/E2E profile |
| Lost communication | 500 ms timeout | No degraded vehicle function |
| Duplicate/out-of-order frame | Sequence-continuity rejection in COM (non-fresh counter rejected); forward jumps tolerated as frame loss | No AUTOSAR E2E profile, no CRC; a corrupted counter that jumps forward is indistinguishable from frame loss |
| Toolchain mismatch | Pinned upstream commit | Docker/Athrill path must be rerun on target host |
