"""Coverage gate: require 100% line coverage over src/*.c.

Runs gcov (GCC) or llvm-cov (Clang) inside build/ so all .gcov
artifacts stay under the gitignored build directory, parses the
'Lines executed' summary per file, and fails unless every file
reaches 100%. Branch coverage is reported but not gated.
"""
import re
import shutil
import subprocess
import sys
from pathlib import Path

SOURCES = ("main.c", "ecu.c", "virtual_can.c")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    build = root / "build"
    gcno = sorted(build.glob("*_cov.gcno"))
    if not gcno:
        print("coverage data (.gcno) not found in build/; run: make coverage", file=sys.stderr)
        return 2
    if shutil.which("llvm-cov"):
        cmd = ["llvm-cov", "gcov", "-b"] + [str(p) for p in gcno]
    elif shutil.which("gcov"):
        cmd = ["gcov", "-b", "-o", str(build)] + [str(root / "src" / s) for s in SOURCES]
    else:
        print("neither llvm-cov nor gcov found", file=sys.stderr)
        return 2
    completed = subprocess.run(cmd, capture_output=True, text=True, cwd=build)
    if completed.returncode != 0:
        print(completed.stderr.strip() or "coverage tool failed", file=sys.stderr)
        return 2
    blocks = re.findall(
        r"File '([^']+)'\nLines executed:([\d.]+)% of (\d+)", completed.stdout
    )
    if not blocks:
        print("could not parse coverage output", file=sys.stderr)
        print(completed.stdout[-2000:], file=sys.stderr)
        return 2
    ok = True
    for filename, percent, total in blocks:
        short = Path(filename).name
        if short not in SOURCES:
            continue
        passed = float(percent) >= 100.0
        ok = ok and passed
        print(f"{short}: lines {percent}% of {total} "
              f"{'OK' if passed else 'BELOW 100%'}")
    print("COVERAGE_GATE_PASS" if ok else "COVERAGE_GATE_FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
