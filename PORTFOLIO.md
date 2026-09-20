# Engineering Case Study — Two-ECU Vehicle Speed Communication

## Problem

Automotive software candidates often show code without demonstrating requirements, architecture, verification, or limitations. This project answers a narrow engineering question end to end:

> Can two virtual ECUs exchange a cyclic vehicle-speed signal through an AUTOSAR Classic-style communication stack, reject malformed frames, detect communication loss deterministically, supervise alive signals via Watchdog Manager, and persist diagnostic data via Non-Volatile Memory?

## My contribution

- Defined a 100 ms CAN interface contract for vehicle speed and sequence counter.
- Implemented a portable C11 reference model with separated COM, PduR, CanIf and CAN responsibilities.
- Implemented two virtual ECUs and deterministic bus fault injection.
- Added timeout, identifier, DLC and plausibility protection.
- **Added Watchdog Manager (WdgM) alive-counter supervision and Non-Volatile Memory (NvM) block storage** — BSW simulation modules with full error-path coverage.
- Built **nine automated integration scenarios** (normal, timeout, invalid-id, invalid-dlc, range boundary, sequence continuity, sequence wrap, UDS coexistence, **WdgM recovery**) and CI with warnings treated as errors.
- Created SWE.1–SWE.5 work products and bidirectional requirements-to-test traceability.
- Pinned a reproducible TOPPERS ATK2/A-COMSTACK/A-RTEGEN/Athrill integration path with CI gate stub.
- Extended the upstream sample with an own 60..250 kph vehicle-speed COM signal (patch + rerun evidence).

## Architecture

```text
ECU1 (Provider)                                      ECU2 (Consumer)
ATK2 100 ms task                                     ATK2 100 ms task
      │                                                    │
Com_SendSignal                                      WdgM supervision
      │                                                    ▲
PduR_ComTransmit                                      Com_RxIndication
      │                                                    ▲
CanIf_Transmit                                     PduR_CanIfRxIndication
      │                                                    ▲
Can_Write ───── CAN ID 0x101 / DLC 3 ────────> CanIf_RxIndication
                                NvM (both ECUs)
```

Payload: `[speed LSB, speed MSB, sequence]`  
Cycle time: `100 ms` · timeout: `500 ms` · valid range: `0..250 km/h`

## Verification result

All **27 executable integration tests** pass (9 scenarios × layer assertions + 9 dedicated BSW tests):

| Scenario | Stimulus | Expected result | Status |
|---|---|---|---|
| `normal` | 11 cyclic frames | 11 accepted; complete layer trace | PASS |
| `timeout` | Drop frames from 300 ms | One timeout at 700 ms | PASS |
| `invalid-id` | Inject 0x777 at 300 ms | One rejection; supervision not refreshed | PASS |
| `invalid-dlc` | Inject DLC 2 at 300 ms | One rejection; no unsafe payload read | PASS |
| `invalid-range` | 65535, 250, 251 km/h at boundaries | 65535 & 251 rejected; 250 accepted | PASS |
| `invalid-seq` | Duplicate counter at 500 ms | One rejection; stream re-synchronizes | PASS |
| `wrap` | Counter 255→0 wrap + stale replay | 255→0 accepted; stale rejected (E2E) | PASS |
| `uds` | UDS ReadDataByIdentifier at 600 ms | UDS_INJECT/RESP present; COM unaffected | PASS |
| `wdgm-recovery` | Drop 300–700 ms, resume at 800 ms | WdgM OK→FAILED→OK; no EXPIRED | PASS |

**Coverage:** 100 % line coverage across all 5 source files (main, ecu, virtual_can, wdgm, nvm).  
**Branch coverage:** 92.45 % aggregate (106 branches).  
**Static analysis:** clang-tidy (clang-analyzer + bugprone), warnings as errors — clean.

## Traceability example

`REQ-WDGM-001` → ECU2 WdgM supervision → `WdgM_MainFunction()` → `TC-WDGM-001` → `evidence/wdgm-recovery.log`

The complete matrix is in [`docs/ASPICE/Traceability_Matrix.csv`](docs/ASPICE/Traceability_Matrix.csv).

## Engineering decisions

1. **Deterministic simulation time:** avoids flaky wall-clock tests.
2. **Host model plus target path:** reviewers obtain instant evidence while the RH850/Athrill path remains reproducible.
3. **Layer boundaries kept explicit:** enables fault localization and maps reasoning to AUTOSAR Classic.
4. **Non-claims documented:** no safety, conformance, or production-readiness exaggeration.
5. **BSW modules as simulation, not middleware:** WdgM/NvM demonstrate architectural reasoning and testability without claiming AUTOSAR conformance.

## Competitive differentiation

| Dimension | Typical candidate portfolio | **This project** |
|---|---|---|
| **Architecture** | Monolithic main loop | Explicit COM/PduR/CanIf/CAN layering + BSW (WdgM/NvM) |
| **Verification** | "Runs on my machine" | 27 executable tests, CI, 100% line / 92% branch coverage |
| **Fault injection** | Ad-hoc printfs | 9 deterministic scenarios with evidence logs |
| **Traceability** | None | SWE.1–SWE.5 + bidirectional matrix (req→arch→code→test→evidence) |
| **Target backend** | None / Simulator only | Pinned TOPPERS ATK2/A-COMSTACK on Athrill (RH850) + CI gate |
| **Documentation** | README only | ASPICE-inspired work products + German summary (Zusammenfassung) |
| **Process awareness** | "I know C" | Static analysis gate, coverage gate, reproducible builds |

## Limitations and next increment

- The pinned RH850/Athrill integration was executed once in Docker with real evidence (`evidence/atk2-athrill.log`); it is not a CI-gated regression yet.
- Host transport does not model CAN arbitration, bus-off, clock drift, or electrical faults.
- Sequence-counter wrap-over is verified by a dedicated fault-injection scenario that climbs the counter to 255, wraps to 0, and confirms acceptance plus stale-replay rejection (the continuity logic uses an unsigned mod-256 half-range rule).
- WdgM/NvM are **simulation-grade** BSW modules — not production AUTOSAR BSW.
- No AUTOSAR E2E profile or DCM/DEM implementation yet.

Next increment: gate the Athrill run in CI (or a scheduled job), add E2E alive-counter policy, expose a UDS diagnostic service for timeout DTC readout, and extend NvM with CRC and multi-block management.

## Reproduce

```bash
make test && make evidence
```