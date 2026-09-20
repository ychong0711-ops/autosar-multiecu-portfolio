"""Coverage gate: require 100% line coverage and >=80% branch coverage over src/*.c.

Runs gcov (GCC) or llvm-cov (Clang) inside build/ so all .gcov
artifacts stay under the gitignored build directory, parses the
'Lines executed' and 'Taken at least once' summaries per file,
and fails unless every file reaches 100% line coverage and the
aggregate branch coverage meets the threshold.
"""
import re
import shutil
import subprocess
import sys
from pathlib import Path

SOURCES = ("main.c", "ecu.c", "virtual_can.c", "wdgm.c", "nvm.c")
BRANCH_THRESHOLD = 80.0  # percent


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    build = root / "build"
    gcno = sorted(build.glob("*.gcno"))
    if not gcno:
        print("coverage data (.gcno) not found in build/; run: make coverage",
              file=sys.stderr)
        return 2
    if shutil.which("llvm-cov"):
        cmd = ["llvm-cov", "gcov", "-b"] + [str(p) for p in gcno]
    elif shutil.which("gcov"):
        cmd = ["gcov", "-b", "-o", str(build)] + \
              [str(root / "src" / s) for s in SOURCES]
    else:
        print("neither llvm-cov nor gcov found", file=sys.stderr)
        return 2
    completed = subprocess.run(cmd, capture_output=True, text=True, cwd=build)
    if completed.returncode != 0:
        print(completed.stderr.strip() or "coverage tool failed",
              file=sys.stderr)
        return 2

    # ── Parse line coverage ────────────────────────────────────────
    line_blocks = re.findall(
        r"File '([^']+)'\nLines executed:([\d.]+)% of (\d+)", completed.stdout
    )
    if not line_blocks:
        print("could not parse coverage output", file=sys.stderr)
        print(completed.stdout[-2000:], file=sys.stderr)
        return 2

    # ── Parse branch coverage per file ─────────────────────────────
    # The gcov -b output groups lines+branches per file block:
    #   File 'src\main.c'
    #   Lines executed:100.00% of 31
    #   Branches executed:100.00% of 22
    #   Taken at least once:95.45% of 22
    # We split on "File '" boundaries and extract the branch info
    # from each block that matches a source file.
    blocks = re.split(r"(?=File ')", completed.stdout)
    branch_map: dict[str, tuple[float, int]] = {}
    for block in blocks:
        file_m = re.search(r"File '([^']+)'", block)
        if not file_m:
            continue
        fname = Path(file_m.group(1)).name
        if fname not in SOURCES:
            continue
        taken_m = re.search(
            r"Taken at least once:([\d.]+)% of (\d+)", block
        )
        if taken_m:
            branch_map[fname] = (float(taken_m.group(1)), int(taken_m.group(2)))

    # ── Evaluate ───────────────────────────────────────────────────
    ok = True
    line_seen: dict[str, tuple[str, str]] = {}
    for filename, percent, total in line_blocks:
        short = Path(filename).name
        if short not in SOURCES:
            continue
        line_seen[short] = (percent, total)

    total_branches = 0
    taken_branches = 0

    for short in SOURCES:
        # Line coverage
        if short not in line_seen:
            print(f"{short}: no line coverage data", file=sys.stderr)
            ok = False
            continue
        l_pct, l_total = line_seen[short]
        l_pass = float(l_pct) >= 100.0
        ok = ok and l_pass

        # Branch coverage
        if short in branch_map:
            b_pct, b_total = branch_map[short]
            taken_branches += int(b_pct / 100.0 * b_total + 0.5)
            total_branches += b_total
            b_pass = b_pct >= BRANCH_THRESHOLD
            ok = ok and b_pass
            print(f"{short}: lines {l_pct}% of {l_total} "
                  f"{'OK' if l_pass else 'BELOW 100%'} | "
                  f"branches {b_pct}% of {b_total} "
                  f"{'OK' if b_pass else f'BELOW {BRANCH_THRESHOLD}%'}")
        else:
            print(f"{short}: lines {l_pct}% of {l_total} "
                  f"{'OK' if l_pass else 'BELOW 100%'} | "
                  f"branches: no data")

    # Aggregate branch summary
    if total_branches > 0:
        agg_pct = taken_branches / total_branches * 100.0
        agg_pass = agg_pct >= BRANCH_THRESHOLD
        ok = ok and agg_pass
        print(f"AGGREGATE branches {agg_pct:.2f}% of {total_branches} "
              f"{'OK' if agg_pass else f'BELOW {BRANCH_THRESHOLD}%'}")
    else:
        print("no branch data found", file=sys.stderr)
        ok = False

    print("COVERAGE_GATE_PASS" if ok else "COVERAGE_GATE_FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
