# Safety Assumptions — ISO 26262 ASIL Decomposition Sketch

> **Scope disclaimer:** This repository is an **educational simulation**, not an
> ISO 26262-compliant product. The ASIL decomposition below is a *design
> intent* sketch (architectural candidate), mapped to what an implementer
> would need to justify at a system-(SWE.5)/hardware–software-interface level
> in an ASPICE + ISO 26262 context. It makes **no** conformance or certification
> claim.

## 1. Vehicle-level safety goal (SG)

| SG-1 | The vehicle speed signal shall not be **lost, stale, duplicated** or
**out-of-range** for a cumulative window that could lead to a hazard. |
|---|---|
| Hazardous event | Unavailable/inconsistent speed display or control input during use. |
| Candidate ASIL | **ASIL B** (per ISO 26262-3:2018, Table 2 severity/controllability/exposure for "continuous, largely controllable" vehicle-speed signals in normal driving on dual-lane roads). |

## 2. Decomposition candidate (independent architectural elements)

Two **sufficiently independent** mechanisms supervise the *same* signal:

| Element | Mechanism | ASIL claim | Independence argument |
|---|---|---|---|
| **D-1 (functional channel)** | COM → PduR → CanIf → CANTx; ECU1 100 ms cyclic TX with E2E profile (alive counter + half-range sequencing). | **ASIL B(D)** | Structural: separate translation units (`ecu.c`) from D-2. |
| **D-2 (supervision channel)** | WdgM alive-supervision + NvM CRC16 + redundant block, in `wdgm.c`/`nvm.c` on a **separate** ECU (ECU2), with a **second, diverse** fault-detection method (deadline supervision vs. CRC integrity) and **redundant data path** (bus + NvM readback). | **ASIL B(D)** | Diversity: deadline-vs-CRC; redundancy: primary+redundant NvM block; independence: distinct BSW modules, distinct call sites, ECU-resident supervision vs. provider COM. |

> **AUTOSAR §8(ASIL decomposition).** Two elements each capable of
> ASIL B(D) may be developed to **ASIL B(D)** when their combination
> satisfies the original ASIL B target AND the elements are
> **sufficiently independent**. The combination here is *alive supervision*
> (detects loss) + *CRC/redundancy* (detects corruption/duplication) —
> complementary failure modes with no single shared-code/memory failure
> that they both depend on.

## 3. Independence assumptions (what must hold for the decomposition)

| Assumption ID | Statement | Where it lives in the code |
|---|---|---|
| A-1 | D-1 and D-2 use **no shared mutable state** except the CAN bus; no shared counters/CRCs between the COM path and the supervision path. | `ecu.c` TX counter vs `wdgm.c` deadline counter and `nvm.c` CRC are distinct scalars; `NvM_Descriptor` and `WdgM_SupervisedEntity` are separate structs with no aliasing. |
| A-2 | The supervision channel runs **on a different ECU** (ECU2), eliminating collocation single-point-of-failure between supervision and the supervised COM provider. | `Ecu2_100msTask` owns `WdgM_MainFunction`, `NvM_MainFunction`, while `Ecu1_100msTask` owns COM TX. |
| A-3 | Deadline supervision and CRC are **diverse** — a corruption that evades the alive-ladder does not automatically evade CRC16, and vice versa. | WdgM checks aliveness; NvM checks CRC16-CCITT. Different algorithms, different tables, different modules. |
| A-4 | A single **redundant stored copy** survives transient RAM corruption; the two copies are compared before use and the primary is recovered from the redundant copy. | `NvM_ReadBlock` validates CRC; on failure it falls back to the redundant block, increments `redundancy_mismatch_count`; the primary is repaired from the redundant copy. |
| A-5 | Fault injection is **deterministic** (timestamp-keyed), so regression evidence is reproducible run-to-run. | VirtualCan_Transmit injects based on `scenario + timestamp_ms`; evidence logs are immutable. |

## 4. What this portfolio does NOT claim

- **No ISO 26262 conformance**: this is a reference-model demo, not a
  certification artifact; ASIL B(D) is written as *design intent*, not as a
  proven decomposition.
- **No ASIL C/D/ASIL decomposition proof**: SG-1 targets ASIL B only; the
  decomposition is a sketch of *how* a candidate would structure the argument,
  not a fulfilled independence demonstration (no FMEDA, no tool qual, no
  certification review).
- **No production NvM**: `NvM_*` is a RAM-backed simulation with CRC16 +
  redundant block; real AUTOSAR NvM requires block ownership, immediate
  writes, CRC on the whole block, wear-levelling, and hardware NVM.
- **No hardware diversity claim**: both ECUs execute on the same host/Athrill
  binary with a shared virtual CAN bus — the "different ECU" argument is an
  architectural sketch for a future real A-COMSTACK/Athrill target, not
  hardware diversity evidence.

## 5. Recommendation for interview use

Frame as: *"I structured the portfolio so the supervision and the functional
COM path are architecturally disjoint (different modules, different ECUs,
different fault-detection methods), which is the *basis* for an ASIL
decomposition argument under ISO 26262-9. What remains for a production
claim is FMEDA, tool qualification, and a real multi-core/hardware setup —
I'm explicitly not claiming those here."*

## 6. Traceability

| Safety assumption | Linked requirement | Linked test |
|---|---|---|
| A-1 (no shared state) | REQ-WDGM-001, REQ-NVM-001 | TC-WDGM-001, TC-NVM-001 |
| A-2 (different ECU) | REQ-COMP-001..002 (COM cross-ECU) | TC-INT-001, TC-WDGM-001 |
| A-3 (diverse algorithms) | REQ-WDGM-001, REQ-NVM-001 | TC-WDGM-002, TC-NVM-002 |
| A-4 (redundant copy) | REQ-NVM-001 | TC-NVM-002 (redundancy fallback) |
| A-5 (deterministic FX) | REQ-ERR-001..005, REQ-E2E-001..003 | TC-ERR-001..005, TC-E2E-001..003 |

*This section is intentionally small: it documents the safety **intent** and
its traceable anchors, leaving the substantially larger ISO 26262 work
(FMEDA, safety case) honestly out of scope.*
