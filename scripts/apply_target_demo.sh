#!/usr/bin/env bash
# Applies the author's target-side extension onto the pinned upstream clone.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="$ROOT/third_party/hakoniwa-ecu-multiplay"
PATCH="$ROOT/target/vehicle-speed-signal.patch"

if [[ ! -d "$UPSTREAM/.git" ]]; then
  echo "Upstream clone missing; run: make upstream-setup" >&2
  exit 1
fi
if git -C "$UPSTREAM" status --short -- a-rtegen/sample/general/HelloAutosar/ | grep -q .; then
  echo "SW-C sources already modified; skipping patch."
else
  git -C "$UPSTREAM" apply --check "$PATCH"
  git -C "$UPSTREAM" apply "$PATCH"
  echo "Applied target/vehicle-speed-signal.patch"
fi
git -C "$UPSTREAM" diff --stat -- a-rtegen/sample/general/HelloAutosar/
