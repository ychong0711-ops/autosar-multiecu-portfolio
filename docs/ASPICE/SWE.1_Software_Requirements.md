# SWE.1 — Software Requirements Analysis

**Project:** Virtual Multi-ECU Vehicle Speed Communication  
**Status:** Baselined v1.0  
**Scope:** Educational portfolio; not a safety-certified production ECU

## 1. Purpose

ECU1 shall cyclically transmit vehicle speed over a CAN-like transport. ECU2 shall validate, decode, and monitor the signal. The production-intent backend is TOPPERS ATK2-SC1 with A-COMSTACK on Athrill; the continuously tested host model preserves the same logical call chain.

## 2. Functional requirements

| ID | Requirement | Rationale | Verification |
|---|---|---|---|
| REQ-OS-001 | ECU1 shall activate its transmit runnable every **100 ms**. | Deterministic cyclic communication | Test + log review |
| REQ-COM-001 | ECU1 shall encode vehicle speed as an unsigned 16-bit little-endian value in CAN ID **0x101**, DLC **3**. Byte 2 is an 8-bit sequence counter. | Defined network contract | Integration test |
| REQ-COM-002 | ECU2 shall accept a valid 0x101/DLC3 frame and expose the decoded speed and sequence counter. | Receiver behavior | Integration test |
| REQ-COM-003 | ECU1 shall receive a transmit confirmation after a frame is accepted by the transport. | Observable communication lifecycle | Integration test |
| REQ-ERR-001 | ECU2 shall report one timeout when no valid frame has arrived for **500 ms**. | Loss-of-communication detection | Fault-injection test |
| REQ-ERR-002 | ECU2 shall reject frames whose CAN ID is not 0x101. | Interface protection | Fault-injection test |
| REQ-ERR-003 | ECU2 shall reject frames whose DLC is not 3. | Data integrity | Fault-injection test |
| REQ-RNG-001 | ECU2 shall reject vehicle speed values above **250 km/h** and accept the boundary value **250 km/h**. | Plausibility protection | Fault-injection boundary test |
| REQ-ERR-004 | ECU2 shall reject a frame whose sequence counter is not newer than the last accepted counter (duplicate or out-of-order arrival). Forward counter jumps are tolerated because a real bus silently loses frames; sustained loss is covered by the timeout monitor. | Message freshness / anti-replay ordering | Fault-injection test |
| REQ-PORT-001 | The repository shall provide a deterministic host build using a C11 compiler. | Recruiter-accessible evidence | CI |
| REQ-PORT-002 | The TOPPERS upstream revision shall be pinned. | Reproducibility | Inspection |

## 3. Non-functional requirements

| ID | Requirement |
|---|---|
| NFR-001 | Host build shall compile with `-Wall -Wextra -Werror -pedantic`. |
| NFR-002 | All automated scenarios shall execute without network access. |
| NFR-003 | Requirements, architecture, tests and evidence shall be bidirectionally traceable. |
| NFR-004 | The host model shall pass the clang-tidy static-analysis gate (`make tidy`). |

## 4. Assumptions and constraints

- The host bus is deterministic and has no arbitration delay.
- The host model demonstrates architecture and testability; it is **not** AUTOSAR-conformant middleware.
- Actual ATK2/A-COMSTACK/A-RTEGEN conformance belongs to the pinned TOPPERS backend.
- No ISO 26262 safety claim is made.
