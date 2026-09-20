# Target-side extension: vehicle-speed COM signal

This directory records the author's own application change on top of the
pinned TOPPERS upstream — the counterpart to the host reference model.

## Delta

`vehicle-speed-signal.patch` (13 insertions, 5 deletions, vs pinned
`ba5c8744ccbeb7ac1437b92c31f8a7585db84d60`):

- `a-rtegen/sample/general/HelloAutosar/SWC1.c` — ECU1 sends a
  **60..250 kph ramp** (`60 + (cnt % 191)`) instead of the raw counter.
  The signal wraps to 60 and can never exceed the 250 kph plausibility
  limit (mirrors host requirement REQ-RNG-001).
- `a-rtegen/sample/general/HelloAutosar/SWC2.c` — ECU2 prints the
  received value and **rejects > 250 kph** as implausible (mirrors the
  host COM plausibility gate; never fires in the recorded run).

## Reproduce

```bash
make upstream-setup          # pinned clone into third_party/
bash scripts/apply_target_demo.sh   # applies this patch onto the clone
make upstream-run            # container preflight, then follow
                             # docs/UPSTREAM_INTEGRATION.md (4 terminals)
```

## Evidence

[`../evidence/atk2-athrill-speed.log`](../evidence/atk2-athrill-speed.log)
(TC-UP-002): ATK2 banner on both ECUs, 150 cyclic transmissions per
side, value streams match with the expected one-cycle COM lag
(ECU1 61..210, ECU2 0,61..209), zero plausibility rejections.

## Scope note

The upstream sample, toolchain, and configuration are untouched; only
the two SW-C runnable bodies above are modified. The patch applies
cleanly onto the pinned commit and nothing else.
