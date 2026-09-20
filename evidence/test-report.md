# Automated Integration Test Report

| Scenario | Result | Evidence |
|---|---|---|
| `normal` | **PASS** | [`normal.log`](normal.log) |
| `timeout` | **PASS** | [`timeout.log`](timeout.log) |
| `invalid-id` | **PASS** | [`invalid-id.log`](invalid-id.log) |
| `invalid-dlc` | **PASS** | [`invalid-dlc.log`](invalid-dlc.log) |
| `invalid-range` | **PASS** | [`invalid-range.log`](invalid-range.log) |
| `invalid-seq` | **PASS** | [`invalid-seq.log`](invalid-seq.log) |

## Raw summaries

```text
[SUMMARY] scenario=normal tx=11 delivered=11 dropped=0 accepted=11 reject_id=0 reject_dlc=0 reject_range=0 reject_seq=0 timeouts=0 tx_confirm=11
[SUMMARY] scenario=timeout tx=11 delivered=3 dropped=8 accepted=3 reject_id=0 reject_dlc=0 reject_range=0 reject_seq=0 timeouts=1 tx_confirm=11
[SUMMARY] scenario=invalid-id tx=11 delivered=11 dropped=0 accepted=10 reject_id=1 reject_dlc=0 reject_range=0 reject_seq=0 timeouts=0 tx_confirm=11
[SUMMARY] scenario=invalid-dlc tx=11 delivered=11 dropped=0 accepted=10 reject_id=0 reject_dlc=1 reject_range=0 reject_seq=0 timeouts=0 tx_confirm=11
[SUMMARY] scenario=invalid-range tx=11 delivered=11 dropped=0 accepted=9 reject_id=0 reject_dlc=0 reject_range=2 reject_seq=0 timeouts=0 tx_confirm=11
[SUMMARY] scenario=invalid-seq tx=11 delivered=11 dropped=0 accepted=10 reject_id=0 reject_dlc=0 reject_range=0 reject_seq=1 timeouts=0 tx_confirm=11
```
