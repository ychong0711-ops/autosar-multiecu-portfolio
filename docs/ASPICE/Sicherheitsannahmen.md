# Safety Assumptions — ISO 26262 ASIL Decomposition Sketch

> **Scope disclaimer:** This repository is an **educational simulation**, not an
> ISO 26262-compliant product. The ASIL decomposition below is a *design
> intent* sketch (architectural candidate), mapped to what an implementer
> would need to justify at a system-(SWE.5)/hardware–software-interface level
> in an ASPICE + ISO 26262 context. It makes **no** conformance or certification
> claim.

## 1. Intended-use boundary

| Item | Assumption |
|---|---|
| Operating context | Two-ECU vehicle-speed COM path on a single CAN segment, ATK2 100 ms cyclic tasks, host + Athrill(RH850) reference models. |
| Hazards considered | Loss/absence of speed signal; stale/duplicated speed; out-of-range speed; communication timeout. |
| Hazards **not** considered | Crash, bus-off/arbitration, memory corruption by external actors, malicious CAN traffic, thermal/EMC, multiple-fault accumulation beyond supervision latency, side-channel. |
| Lifecycle | Pre-silicon engineering reference; no production launch safety case. |

## 2. ASIL decomposition (candidate)

Based on a **`Vehicle Speed Availability`** vehicle-level safety goal of
**ASIL B** and a need to justify how a reference portfolio *could* carry an
ASIL without claiming it.

| Safety goal (SG-1) | Decomposed requirement | ASIL allocation (AUTOSAR Art. 26262-9 §7) |
|---|---|---|
| SG-1: The speed signal shall not be lost, stale, duplicated, or out-of-range for a cumulative window that could lead to unavailability (target: ASIL B). | D-1: E2E alive counter + CRC supervision detects staleness/duplication/loss (E2E profile hosted in COM/Ecu2). | **ASIL B(D)** |
| | D-2: WdgM alive supervision + NvM CRC redundant block independently verifies the same channel, and faults are **independently** detected (different algorithms: CRC16 vs alive-ladder vs range-gate). | **ASIL B(D)** |

> **Decomposition rationale (AUTOSAR §7.4.4):** D-1 (E2E Profile-style
> counter+CRC) and D-2 (WdgM alive + NvM redundant+CRC) use **distinct
> detection algorithms and distinct code paths**, satisfying the "sufficiently
> independent" criterion for ASIL B(D)+ASIL B(D) ⇒ decomposition to
> ASIL B(D) with independent elements, and the residual is below the
> ASIL B threshold.

## 3. Independence argument (what would need to be proven)

| # | Independence claim | Required evidence (not yet produced — this portfolio's gap) |
|---|---|---|
| I-1 | D-1 and D-2 use separate state machines in separate translation units (`ecu.c`, `wdgm.c`, `nvm.c`) with no shared mutable state besides the bus. | Shared-bus access is already serialised through `VirtualCan`; **no ICC locks / no shared CRC table between E2E and NvM**. Trace logs each layer. |
| I-2 | CRC16 in NvM and CRC-usage in E2E are distinct CRC instances (NvM CRC-CCITT table, E2E mod-256 counter) — no common-cause failure in a single CRC. | NvM uses CRC-CCITT 0x1021 table-driven; E2E uses rollover counter + half-range ladder. Distinct mechanisms. |
| I-3 | Supervision latencies are bounded and deterministic: WdgM FAILED after 500 ms, EXPIRED after 3 missed deadlines; NvM readback CRC gate catches corruption. | Deterministic PIT (Simulated_Time) evidence, coverage 100 %, branch 92.45 %. |

## 4. Safety mechanisms — mapping to code

| Mechanism | Module | Where verified |
|---|---|---|
| E2E counter continuity (half-range rollover wrap-safe) | `Com_RxIndication` in `ecu.c` | TC-E2E-004/005, wrap scenario |
| WdgM alive supervision (FAILED→EXPIRED→recovery) | `wdgm.c` `WdgM_MainFunction` | TC-WDGM-001, wdgm-recovery scenario, TC-WDGM recovery tests |
| NvM redundant block + CRC16 + CRF-error detection | `nvm.c` `NvM_WriteBlock`/`NvM_ReadBlock` | NvM tests, coverage 100 % lines |
| Timeout/identifier/DLC/range/sequence guards | `virtual_can.c` + `ecu.c` | 9 fault-injection scenarios |

## 5. What will be added next (Roadmap)

| Increment | Contents | Evidence gate |
|---|---|---|
| A | DCM ReadDTCInformation (0x19) DTC readout service coexisting with UDS 0x22 | new `uds-dtc` scenario |
| B | NvM CRC16 redundant block CRC verify error-injection test | `nvm` negative CRC test |
| C | Schedule Athrill regression in CI (scheduled run) | `athrill-scheduled` workflow |
| D | ISO 26262 §7 decomposition brief (this document, expanded) | ASPICE SWE.5 safety work product |

## 6. Honest limitations (interview-ready)

1. **This is not ASIL decomposition for a production ECU** — it's a *design-intent
   sketch* demonstrating the *reasoning*, not a certified independence argument.
2. **No FMEDA/quantitative**: no failure-rate data, no coverage of the
   independence argument against common-cause analysis; that requires a tool
   ecosystem (e.g., Medini Analyze).
3. **No ISO 26262 tool qualification claim** — clang/Athrill are used as
   engineering tools, not as qualified dev tools; CI evidence is best-effort.
4. **Simulator-level WdgM/NvM** — not full AUTOSAR BSW integration with
   Startup/EbOs/CRC hardware acceleration.
5. **Single DID/one DTC semantics** — production DCM/DEM would have many
   DTCs + grouped readout (ReadDTCInformationByStatus).

---

*This document supports interview narrative around process, safety awareness
and traceability; it does not imply certification. It is written to show that
the author knows *what* an ASIL decomposition is and *why* it is argued.*
