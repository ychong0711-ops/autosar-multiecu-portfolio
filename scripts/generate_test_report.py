from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
evidence = root / "evidence"
rows = []
for scenario in ("normal", "timeout", "invalid-id", "invalid-dlc",
                 "invalid-range", "invalid-seq", "wrap"):
    text = (evidence / f"{scenario}.log").read_text()
    line = next(x for x in text.splitlines() if x.startswith("[SUMMARY]"))
    vals = dict(re.findall(r"([a-z_]+)=([\w-]+)", line))
    expected = {
        "normal": vals["accepted"] == "11" and vals["timeouts"] == "0",
        "timeout": vals["timeouts"] == "1" and vals["dropped"] == "8",
        "invalid-id": vals["reject_id"] == "1",
        "invalid-dlc": vals["reject_dlc"] == "1",
        "invalid-range": vals["reject_range"] == "2" and vals["accepted"] == "9",
        "invalid-seq": vals["reject_seq"] == "1" and vals["accepted"] == "10",
        "wrap": vals["reject_seq"] == "1" and vals["accepted"] == "10"
                and vals["timeouts"] == "0",
    }[scenario]
    rows.append((scenario, "PASS" if expected else "FAIL", line))

report = [
    "# Automated Integration Test Report",
    "",
    "| Scenario | Result | Evidence |",
    "|---|---|---|",
]
for scenario, result, line in rows:
    report.append(f"| `{scenario}` | **{result}** | [`{scenario}.log`]({scenario}.log) |")
report += ["", "## Raw summaries", "", "```text"] + [r[2] for r in rows] + ["```", ""]
(evidence / "test-report.md").write_text("\n".join(report))
