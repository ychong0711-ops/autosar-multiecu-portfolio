# Real TOPPERS ATK2 + A-COMSTACK + Athrill Integration

## Verification status

- **Host reference model:** built and tested in this repository.
- **RH850/Athrill backend:** executed 2026-09-20 in Docker; evidence in [`../evidence/atk2-athrill.log`](../evidence/atk2-athrill.log) (ATK2 banner on both ECUs, 150 cyclic COM transmissions per ECU). TC-UP-001 **PASS**.

## Pinned dependency

- Repository: `https://github.com/toppers/hakoniwa-ecu-multiplay`
- Commit: `ba5c8744ccbeb7ac1437b92c31f8a7585db84d60`
- Components: ATK2-SC1, A-COMSTACK, A-RTEGEN, Athrill/Hakoniwa integration

```bash
make upstream-setup
```

## Container path

Install Docker, then use the upstream v1.3.0 environment or build its Dockerfile. The canonical upstream paths are:

```bash
export HAKO_WS_ECU1="a-rtegen/sample/sc1/HelloAutosarWithCom/hsbrh850f1k_gcc/ecu1"
export HAKO_WS_ECU2="a-rtegen/sample/sc1/HelloAutosarWithCom/hsbrh850f1k_gcc/ecu2"
export HAKO_WS_CAN="a-comstack/can/target/hsbrh850f1k_gcc/sample"
```

Build the two RTE/COM ECUs as documented in the pinned upstream `README.md`. Start:

1. `hako-master 100 200`
2. ECU1 via `hako-proxy` and `proxy_config_rte_ecu1.json`
3. ECU2 via `hako-proxy` and `proxy_config_rte_ecu2.json`
4. `hako-cmd start`

## Evidence acceptance criteria

Save unedited terminal output as `evidence/atk2-athrill.log` (done 2026-09-20). It must contain:

```text
TOPPERS/ATK2-SC1 Release 1.4.2 for HSBRH850F1K
```

and observable ECU-to-ECU COM/CAN activity. Record:

- Docker image digest
- upstream commit
- OS and Docker versions
- build commands
- terminal output

Do not mark `TC-UP-001` PASS until that evidence exists (satisfied 2026-09-20).

## Target-side extension (TC-UP-002)

Beyond running the upstream sample, the author modified two SW-C
runnable bodies (see [`../target/README.md`](../target/README.md) and
[`../target/vehicle-speed-signal.patch`](../target/vehicle-speed-signal.patch)):

- ECU1 sends a 60..250 kph ramp that wraps to 60 (never exceeds the
  host model's 250 kph plausibility limit, cf. REQ-RNG-001).
- ECU2 prints each received value and rejects anything above 250 kph.

Reproduce after `make upstream-setup`:

```bash
bash scripts/apply_target_demo.sh
```

then rebuild both ECUs and rerun the four terminals exactly as above.
Evidence: [`../evidence/atk2-athrill-speed.log`](../evidence/atk2-athrill-speed.log)
(TC-UP-002 PASS).
