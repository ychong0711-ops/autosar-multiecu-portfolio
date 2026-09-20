# SWE.5 — Software Integration and Integration Test

## Test cases

| Test ID | Preconditions / stimulus | Expected result |
|---|---|---|
| TC-INT-001 | Normal scenario, 0–1000 ms | 11 transmitted, 11 accepted, no timeout; complete COM→PduR→CanIf→CAN→CanIf→PduR→COM trace |
| TC-ERR-001 | Drop all frames from 300 ms | Last valid frame at 200 ms; exactly one timeout at 700 ms |
| TC-ERR-002 | Replace the 300 ms frame ID with 0x777 | Exactly one ID rejection; other 10 frames accepted |
| TC-ERR-003 | Replace the 300 ms DLC with 2 | Exactly one DLC rejection; other 10 frames accepted |
| TC-ERR-004 | Range boundary: 0 ms speed set to 65535, 500 ms speed set to exactly 250, 800 ms speed set to 251 | Two range rejections (65535, 251); exactly 250 accepted; 9 frames accepted total; no sequence side effects |
| TC-ERR-005 | Replace the 500 ms sequence counter with 3 (duplicate of an earlier counter) | Exactly one duplicate flagged at 500 ms (`counter=3 last=4`); the stream re-synchronizes on the next forward frame; 10 frames accepted, 1 sequence rejection |
| TC-ERR-006 | Wrap scenario: the 8-bit counter is walked 100, 200, 240, 250, 254, 255, then wraps to 0, and a stale 255 is replayed at 800 ms | 255→0 wrap accepted as NEW (`seq=0 result=ACCEPT`); stale replay rejected (`counter=255 last=0`); stream re-synchronizes on 1, 2; 10 frames accepted, 1 sequence rejection |
| TC-UP-001 | Build and run pinned TOPPERS backend in Docker | ATK2 banner on both ECUs and 150 cyclic COM transmissions per ECU recorded; **PASS** — see `evidence/atk2-athrill.log` |
| TC-UP-002 | Apply `target/vehicle-speed-signal.patch`, rebuild both ECUs, rerun demo | ATK2 banner on both ECUs; 150 cyclic transmissions per side; ECU1 61..210 kph, ECU2 0,61..209 kph (one-cycle COM lag); 0 plausibility rejections; **PASS** — see `evidence/atk2-athrill-speed.log` |
| TC-E2E-001 | Wrap scenario: counter ladder 0→254→255→0→255→1→2 | 255→0 wrap accepted; stale 255 rejected; stream re-synchronizes; E2E Profile-1 continuity verified |
| TC-E2E-002 | Duplicate counter replay at 500 ms | Exactly one sequence rejection; stream re-synchronizes on next forward frame |
| TC-E2E-003 | Invalid DLC scenario | DLC rejection at interface; E2E counter not inspected (independent layer); 0 sequence rejections |
| TC-UDS-001 | UDS scenario: diagnostic request/response injected at 600 ms | UDS_INJECT and UDS_RESP trace lines present; COM path delivers all 11 frames; no interference between COM and diagnostic traffic |
| TC-WDGM-001 | WDGM-recovery scenario: frames dropped 300–700 ms (FAILED), resume at 800 ms | WdgM transitions OK→FAILED at 700 ms; recovers to OK at 800 ms without reaching EXPIRED; "deadline missed" and recovery trace present |
| TC-NVM-001 | Normal scenario + SUMMARY NvM readback | NvM writes on TX/RX, reads back valid block, rejects invalid block ID (0xFF), rejects oversized length (100), prints read/write counts |

## Execution

```bash
make test       # executable assertions (14 tests)
make evidence   # eight immutable text logs + report
```

## Pass criteria

All executable assertions pass and generated summaries match the expected counters. TC-UP-001 passed with real Athrill evidence in `evidence/atk2-athrill.log`.
