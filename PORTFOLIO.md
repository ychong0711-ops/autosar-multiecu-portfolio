# Engineering Case Study — Two-ECU Vehicle Speed Communication

## Problem

Automotive software candidates often show code without demonstrating requirements, architecture, verification, or limitations. This project answers a narrow engineering question end to end:

> Can two virtual ECUs exchange a cyclic vehicle-speed signal through an AUTOSAR Classic-style communication stack, reject malformed frames, and detect communication loss deterministically?

## My contribution

- Defined a 100 ms CAN interface contract for vehicle speed and sequence counter.
- Implemented a portable C11 reference model with separated COM, PduR, CanIf and CAN responsibilities.
- Implemented two virtual ECUs and deterministic bus fault injection.
- Added timeout, identifier, DLC and plausibility protection.
- Built six automated integration scenarios (normal, timeout, invalid-id, invalid-dlc, range boundary, sequence continuity) and CI with warnings treated as errors.
- Created SWE.1–SWE.5 work products and bidirectional requirements-to-test traceability.
- Pinned a reproducible TOPPERS ATK2/A-COMSTACK/A-RTEGEN/Athrill integration path.
- Extended the upstream sample with an own 60..250 kph vehicle-speed COM signal (patch + rerun evidence).

## Architecture

```text
ECU1 (Provider)                                      ECU2 (Consumer)
ATK2 100 ms task                                     ATK2 100 ms task
      │                                                    │
Com_SendSignal                                      timeout monitor
      │                                                    ▲
PduR_ComTransmit                                      Com_RxIndication
      │                                                    ▲
CanIf_Transmit                                     PduR_CanIfRxIndication
      │                                                    ▲
Can_Write ───── CAN ID 0x101 / DLC 3 ────────> CanIf_RxIndication
```

Payload: `[speed LSB, speed MSB, sequence]`  
Cycle time: `100 ms` · timeout: `500 ms` · valid range: `0..250 km/h`

## Verification result

All four executable integration tests pass:

- 11 normal frames traverse COM → PduR → CanIf → CAN and back.
- Eight injected frame losses trigger exactly one timeout at 500 ms age.
- One incorrect CAN identifier is rejected without refreshing supervision.
- One incorrect DLC is rejected without unsafe payload access.
- The speed plausibility boundary is exact: 65535 and 251 km/h are rejected while 250 km/h is accepted.
- One injected duplicate sequence counter is rejected at 500 ms while the stream re-synchronizes on the next frame.

## Traceability example

`REQ-ERR-001` → ECU2 timeout monitor → `Ecu2_100msTask()` → `TC-ERR-001` → `evidence/timeout.log`

The complete matrix is in [`docs/ASPICE/Traceability_Matrix.csv`](docs/ASPICE/Traceability_Matrix.csv).

## Engineering decisions

1. **Deterministic simulation time:** avoids flaky wall-clock tests.
2. **Host model plus target path:** reviewers obtain instant evidence while the RH850/Athrill path remains reproducible.
3. **Layer boundaries kept explicit:** enables fault localization and maps reasoning to AUTOSAR Classic.
4. **Non-claims documented:** no safety, conformance, or production-readiness exaggeration.

## Limitations and next increment

- The pinned RH850/Athrill integration was executed once in Docker with real evidence (`evidence/atk2-athrill.log`); it is not a CI-gated regression yet.
- Host transport does not model CAN arbitration, bus-off, clock drift, or electrical faults.
- Sequence-counter wrap-over has no dedicated 256-frame rollover test (the continuity logic is `uint8_t`-wrap-safe by construction).
- No AUTOSAR E2E profile or DCM/DEM implementation yet.

Next increment: gate the Athrill run in CI (or a scheduled job), add E2E alive-counter policy, and expose a UDS diagnostic service for timeout DTC readout.

## Reproduce

```bash
make test && make evidence
```
