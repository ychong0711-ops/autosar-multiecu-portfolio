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
| TC-UP-001 | Build and run pinned TOPPERS backend in Docker | ATK2 banner on both ECUs and 150 cyclic COM transmissions per ECU recorded; **PASS** — see `evidence/atk2-athrill.log` |
| TC-UP-002 | Apply `target/vehicle-speed-signal.patch`, rebuild both ECUs, rerun demo | ATK2 banner on both ECUs; 150 cyclic transmissions per side; ECU1 61..210 kph, ECU2 0,61..209 kph (one-cycle COM lag); 0 plausibility rejections; **PASS** — see `evidence/atk2-athrill-speed.log` |

## Execution

```bash
make test       # executable assertions
make evidence   # six immutable text logs + report
```

## Pass criteria

All executable assertions pass and generated summaries match the expected counters. TC-UP-001 passed with real Athrill evidence in `evidence/atk2-athrill.log`.
