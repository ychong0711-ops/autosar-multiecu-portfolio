#!/usr/bin/env python3
"""Check coverage gate for ATHRILL backend evidence.

Usage: python32 scripts/check_athrill_ci.py
Exit 0 if evidence is reproducible, 1 otherwise.
"""

from pathlib import Path
import re
import sys

EVIDENCE_DIR = Path("evidence")
REQUIRED_MARKERS = [
    "TASK", "ATK2", "Athrill", "COM", "Cyclic",
]


def main() -> int:
    logs = sorted(EVIDENCE_DIR.glob("athrill-*.log"))
    if not logs:
        print("No Athrill evidence logs found", file=sys.stderr)
        return 1
    ok = True
    for log in logs:
        text = log.read_text(encoding="utf-8", errors="replace")
        present = [m for m in REQUIRED_MARKERS if m in text]
        missing = [m for m in REQUIRED_MARKERS if m not in text]
        status = "OK" if not missing else f"MISSING {missing}"
        print(f"{log.name}: markers={len(present)}/{len(REQUIRED_MARKERS)} {status}")
        if missing:
            ok = False
    if not ok:
        print("ATHRILL_CI_GATE_FAIL")
        return 1
    print("ATHRILL_CI_GATE_PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
