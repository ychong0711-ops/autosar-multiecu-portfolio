# SWE.4 — Software Unit Verification

## Static verification

The build uses:

```text
-std=c11 -Wall -Wextra -Werror -pedantic -O2
```

Warnings are treated as errors. GitHub Actions rebuilds every push and pull request.

Static analysis runs `clang-tidy` with the `clang-analyzer` and
`bugprone` check groups and warnings-as-errors
(`make tidy`, also CI-gated). The
`insecureAPI.DeprecatedOrUnsafeBufferHandling` check is excluded: the
host model's `printf` logging is its evidence mechanism, and blanket
`fprintf_s` migration would obscure the trace output. MISRA-C rule
mapping (e.g. via the cppcheck MISRA addon) is the next
static-analysis increment — claimed as awareness, not compliance.

## Unit-level verification status

| Unit | Method | Status |
|---|---|---|
| Frame encoder | Covered through normal integration scenario | PASS |
| CAN ID validator | Fault injection | PASS |
| DLC validator | Fault injection | PASS |
| Timeout monitor | Deterministic simulation-time test | PASS |
| Range validator | Boundary fault injection: 65535 and 251 rejected, exactly 250 accepted | PASS |
| Sequence continuity validator | Duplicate-counter fault injection (reject non-fresh counter, re-synchronize on next forward frame) | PASS |
| Sequence counter wrap (mod-256) | Continuity logic is wrap-safe by construction (`uint8_t` successor); dedicated 256-frame rollover test | **OPEN** |
| Line coverage | 100% of lines over `src/*.c` across the six scenarios plus the bad-argument case (`make coverage`, CI-gated; branch coverage reported but not gated) | PASS |

Open tests are declared rather than hidden. They are candidates for the next increment.
