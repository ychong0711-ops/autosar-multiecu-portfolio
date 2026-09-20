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
| REQ-ERR-005 | ECU2 shall accept a sequence counter that wraps from 255 to 0 (a genuine mod-256 forward wrap counts as NEW) and continue rejecting duplicate/backward counters after the wrap. | 8-bit counter rollover continuity | Fault-injection boundary test |
| REQ-E2E-001 | E2E Profile-1 continuity: the sequence counter wrap (255→0) shall be accepted as a genuine forward transition. | End-to-end protection for COM signals | Fault-injection wrap test |
| REQ-E2E-002 | E2E Profile-1 anti-replay: a duplicate sequence counter shall be rejected. | End-to-end protection for COM signals | Fault-injection duplicate test |
| REQ-E2E-003 | E2E counter shall be independent of the CAN DLC layer; DLC rejection must not produce false sequence rejections. | Layer separation | Fault-injection cross-layer test |
| REQ-UDS-001 | UDS diagnostic traffic (ReadDataByIdentifier 0x22) shall coexist with the periodic COM signal without interference. | Diagnostic services on shared CAN bus | UDS coexistence test |
| REQ-WDGM-001 | ECU2 shall supervise the alive signal from ECU1 using WdgM. If no valid frame arrives for **500 ms**, WdgM shall transition to FAILED. If frames resume within the deadline, WdgM shall recover to OK without reaching EXPIRED. | Watchdog supervision of communication | WDGM recovery test |
| REQ-NVM-001 | ECU1 and ECU2 shall persist the last transmitted/received vehicle speed to NvM block 0 and support readback with error handling for invalid block IDs and oversized lengths. | Persistent storage for diagnostics | NvM read/write test |
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
