#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="$ROOT/third_party/hakoniwa-ecu-multiplay"

if ! command -v docker >/dev/null 2>&1; then
  echo "ERROR: Docker is required for the RH850/Athrill integration path." >&2
  echo "Run the host reference model instead: make test && make evidence" >&2
  exit 127
fi
if [[ ! -d "$UPSTREAM" ]]; then
  "$ROOT/scripts/setup_upstream.sh"
fi
cat <<'MSG'
The reproducible TOPPERS environment is ready.

The upstream demo requires four coordinated processes (Hakoniwa master,
ECU1, ECU2, and start command). Follow docs/UPSTREAM_INTEGRATION.md exactly.
This wrapper intentionally does not fabricate evidence: copy the real
terminal output to evidence/atk2-athrill.log after the run.
MSG
