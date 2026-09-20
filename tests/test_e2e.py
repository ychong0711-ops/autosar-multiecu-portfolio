import re
import subprocess
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "build" / "multiecu_demo"


def run_scenario(name: str):
    completed = subprocess.run(
        [str(BINARY), name], check=True, capture_output=True, text=True
    )
    summary_line = next(
        line for line in completed.stdout.splitlines() if line.startswith("[SUMMARY]")
    )
    values = dict(re.findall(r"([a-z_]+)=([\w-]+)", summary_line))
    return completed.stdout, {k: int(v) if v.isdigit() else v for k, v in values.items()}


class E2EProtectionTests(unittest.TestCase):
    """E2E-protection properties of the 8-bit COM sequence counter.

    Host-only and deterministic: the continuity rules are exercised through
    the same fault-injected scenarios used by the integration tests.
    """

    def test_sequence_counter_wrap_accepted_e2e_profile(self):
        output, summary = run_scenario("wrap")
        # E2E Profile-1 continuity: a single forward mod-256 wrap (255 -> 0)
        # computes diff == 1, so the wrapped frame is NEW and accepted.
        self.assertIn("seq=255 result=ACCEPT", output)
        self.assertIn(
            "[0700ms][ECU2][COM] Com_RxIndication speed=95kph seq=0 result=ACCEPT",
            output,
        )
        self.assertNotIn("[0700ms][ECU2][COM] REJECT", output)
        # only the deliberate stale replay counts as a sequence rejection
        self.assertEqual(summary["reject_seq"], 1)

    def test_timeout_flags_old_counter_stale_e2e(self):
        output, summary = run_scenario("timeout")
        # after 500 ms without a refresh the previously accepted counter is
        # stale: the monitor flags exactly one timeout and nothing more is
        # accepted (the frames after 300 ms are dropped on the bus).
        self.assertIn("[0700ms][ECU2][COM] TIMEOUT age=500ms", output)
        self.assertEqual(summary["timeouts"], 1)
        self.assertEqual(summary["accepted"], 3)
        self.assertEqual(summary["reject_seq"], 0)

    def test_duplicate_counter_rejected_e2e(self):
        output, summary = run_scenario("invalid-seq")
        # replaying an old counter yields diff == 255, which is not within
        # the half-range window, so it is rejected as not-new.
        self.assertEqual(summary["reject_seq"], 1)
        self.assertIn("REJECT reason=seq-not-new counter=3 last=4", output)
        # the stream re-synchronizes on the next forward frame
        self.assertIn(
            "[0600ms][ECU2][COM] Com_RxIndication speed=90kph seq=6 result=ACCEPT",
            output,
        )

    def test_e2e_counter_independent_of_pdu_dlc(self):
        output, summary = run_scenario("invalid-dlc")
        # E2E continuity sits above the CAN interface layer: a frame with a
        # corrupted DLC is rejected at the interface before the counter is
        # inspected, so no sequence rejection is attributed to it.
        self.assertEqual(summary["reject_dlc"], 1)
        self.assertEqual(summary["reject_seq"], 0)
        self.assertEqual(summary["accepted"], 10)


    def test_uds_diagnostic_request_response(self):
        """UDS ReadDataByIdentifier (0x22) is injected at 600ms.

        The virtual CAN bus emits UDS_INJECT / UDS_RESP trace lines.
        The COM path still delivers the frame normally because UDS
        traffic coexists with the periodic vehicle-speed COM signal.
        """
        output, summary = run_scenario("uds")
        self.assertIn("UDS_INJECT", output)
        self.assertIn("UDS_RESP", output)
        self.assertIn("ReadDataByIdentifier", output)
        self.assertIn("VehicleSpeedDID", output)
        # COM path remains functional: all 11 transmitted frames are delivered
        self.assertEqual(summary["delivered"], summary["tx"])


if __name__ == "__main__":
    unittest.main()