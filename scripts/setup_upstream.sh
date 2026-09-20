#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/third_party/hakoniwa-ecu-multiplay"
REPO="https://github.com/toppers/hakoniwa-ecu-multiplay.git"
PINNED_COMMIT="ba5c8744ccbeb7ac1437b92c31f8a7585db84d60"

if [[ -d "$DEST/.git" ]]; then
  echo "Upstream already present: $DEST"
else
  mkdir -p "$(dirname "$DEST")"
  git clone --recurse-submodules "$REPO" "$DEST"
fi
git -C "$DEST" fetch origin "$PINNED_COMMIT"
git -C "$DEST" checkout --detach "$PINNED_COMMIT"
git -C "$DEST" submodule update --init --recursive
printf 'Pinned TOPPERS revision: '
git -C "$DEST" rev-parse HEAD
