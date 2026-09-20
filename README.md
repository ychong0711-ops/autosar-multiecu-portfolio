# Virtual Multi-ECU AUTOSAR Communication Portfolio

[![Host Reference CI](https://img.shields.io/badge/host%20reference-tested-brightgreen)](#verified-results)
[![CI](https://github.com/ychong0711-ops/autosar-multiecu-portfolio/actions/workflows/ci.yml/badge.svg)](https://github.com/ychong0711-ops/autosar-multiecu-portfolio/actions/workflows/ci.yml)
[![ATK2 Backend](https://img.shields.io/badge/ATK2%20backend-verified-brightgreen)](evidence/atk2-athrill.log)
[![Process](https://img.shields.io/badge/process-ASPICE--inspired-blue)](docs/ASPICE/SWE.1_Software_Requirements.md)

A recruiter-runnable two-ECU vehicle-speed communication demo, designed around the AUTOSAR Classic communication path:

```text
ECU1: ATK2 Task → COM → PduR → CanIf → CAN Driver
                                             ↓
                                Virtual CAN / Hakoniwa
                                             ↓
ECU2: Consumer ← COM ← PduR ← CanIf ← CAN Driver
```

The repository combines:

1. a **portable C11 reference model** that builds and tests in seconds;
2. a pinned path to **TOPPERS ATK2-SC1 + A-COMSTACK + A-RTEGEN + Athrill**;
3. an **Automotive SPICE-inspired** engineering evidence package.

> **Honest scope:** the host model is tested, and the Docker-based RH850/Athrill backend was executed once with real evidence in [`evidence/atk2-athrill.log`](evidence/atk2-athrill.log) (TC-UP-001). This is an educational portfolio, not an AUTOSAR conformance or ISO 26262 safety claim.

## Quick start

Requirements: C compiler, Make, Python 3.

```bash
make
make test
make evidence
```

Run individual scenarios:

```bash
./build/multiecu_demo normal
./build/multiecu_demo timeout
./build/multiecu_demo invalid-id
./build/multiecu_demo invalid-dlc
./build/multiecu_demo invalid-range
./build/multiecu_demo invalid-seq
```

## Verified results

| Scenario | Requirement | Expected |
|---|---|---|
| `normal` | REQ-OS-001, REQ-COM-001..003 | 11/11 frames accepted; full layer trace; no timeout |
| `timeout` | REQ-ERR-001 | Frames dropped from 300 ms; timeout at exactly 700 ms |
| `invalid-id` | REQ-ERR-002 | One unexpected identifier rejected |
| `invalid-dlc` | REQ-ERR-003 | One malformed DLC rejected |
| `invalid-range` | REQ-RNG-001 | 65535 and 251 km/h rejected; exact boundary 250 km/h accepted |
| `invalid-seq` | REQ-ERR-004 | One duplicate counter flagged at 500 ms; stream re-synchronizes on next frame; remaining 10 frames accepted |

Generated evidence is in [`evidence/`](evidence/).

## Example evidence

```text
[0300ms][ECU1][COM] Com_SendSignal speed=75kph seq=3
[0300ms][ECU1][PDUR] PduR_ComTransmit
[0300ms][ECU1][CANIF] CanIf_Transmit pdu=VehicleSpeedPdu
[0300ms][ECU1][CAN] Can_Write id=0x101 dlc=3
[0300ms][BUS] ROUTE id=0x101 dlc=3 ECU1->ECU2
[0300ms][ECU2][CANIF] CanIf_RxIndication id=0x101 dlc=3
[0300ms][ECU2][PDUR] PduR_CanIfRxIndication
[0300ms][ECU2][COM] Com_RxIndication speed=75kph seq=3 result=ACCEPT
[0300ms][ECU1][CANIF] CanIf_TxConfirmation pdu=VehicleSpeedPdu
```

## Engineering evidence

| ASPICE-inspired work product | File |
|---|---|
| SWE.1 Software requirements | [`SWE.1_Software_Requirements.md`](docs/ASPICE/SWE.1_Software_Requirements.md) |
| SWE.2 Software architecture | [`SWE.2_Software_Architecture.md`](docs/ASPICE/SWE.2_Software_Architecture.md) |
| SWE.3 Detailed design | [`SWE.3_Detailed_Design.md`](docs/ASPICE/SWE.3_Detailed_Design.md) |
| SWE.4 Unit verification | [`SWE.4_Unit_Verification.md`](docs/ASPICE/SWE.4_Unit_Verification.md) |
| SWE.5 Integration test | [`SWE.5_Integration_Test.md`](docs/ASPICE/SWE.5_Integration_Test.md) |
| Bidirectional traceability | [`Traceability_Matrix.csv`](docs/ASPICE/Traceability_Matrix.csv) |
| Safety assumptions/non-claims | [`Safety_Assumptions.md`](docs/ASPICE/Safety_Assumptions.md) |

## Real ATK2/A-COMSTACK path

```bash
make upstream-setup
make upstream-run
```

See [`docs/UPSTREAM_INTEGRATION.md`](docs/UPSTREAM_INTEGRATION.md). The upstream dependency is cloned on demand and is not redistributed by this repository.

## Repository map

```text
src/                 portable layered communication reference model
config/              CAN contract and timing configuration
tests/               executable integration assertions
docs/ASPICE/         lifecycle work products and traceability
evidence/             generated raw logs and report
scripts/              evidence generation and pinned upstream setup
.github/workflows/    reproducible CI
PORTFOLIO.md          one-page technical case study
portfolio.html        offline visual recruiter presentation
```

## Skills demonstrated

Embedded C · AUTOSAR Classic architecture · ATK2/OSEK reasoning · CAN · COM/PduR/CanIf layering · deterministic testing · fault injection · requirements traceability · Automotive SPICE awareness · GitHub Actions · technical English

## License

Original host-model code and documentation: MIT, see [`LICENSE`](LICENSE). TOPPERS components are fetched separately and remain under their respective upstream licenses.
