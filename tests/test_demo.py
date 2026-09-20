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


class MultiEcuIntegrationTests(unittest.TestCase):
    def test_normal_transmission_meets_req_com_001_002(self):
        output, summary = run_scenario("normal")
        self.assertEqual(summary["tx"], 11)
        self.assertEqual(summary["accepted"], 11)
        self.assertEqual(summary["timeouts"], 0)
        self.assertIn("CanIf_TxConfirmation", output)
        self.assertIn("PduR_CanIfRxIndication", output)
        self.assertIn("Com_RxIndication", output)

    def test_timeout_is_detected_at_500ms_age_req_err_001(self):
        output, summary = run_scenario("timeout")
        self.assertEqual(summary["accepted"], 3)
        self.assertEqual(summary["dropped"], 8)
        self.assertEqual(summary["timeouts"], 1)
        self.assertIn("[0700ms][ECU2][COM] TIMEOUT age=500ms", output)

    def test_unexpected_can_id_is_rejected_req_err_002(self):
        output, summary = run_scenario("invalid-id")
        self.assertEqual(summary["reject_id"], 1)
        self.assertEqual(summary["accepted"], 10)
        self.assertIn("REJECT reason=unexpected-can-id", output)

    def test_invalid_dlc_is_rejected_req_err_003(self):
        output, summary = run_scenario("invalid-dlc")
        self.assertEqual(summary["reject_dlc"], 1)
        self.assertEqual(summary["accepted"], 10)
        self.assertIn("REJECT reason=invalid-dlc", output)

    def test_range_boundary_rejects_above_250_and_accepts_250_req_rng_001(self):
        output, summary = run_scenario("invalid-range")
        self.assertEqual(summary["reject_range"], 2)
        self.assertEqual(summary["accepted"], 9)
        self.assertIn("REJECT reason=range speed=65535", output)
        self.assertIn("speed=250kph seq=5 result=ACCEPT", output)
        self.assertIn("REJECT reason=range speed=251", output)

    def test_sequence_continuity_rejects_duplicate_req_err_004(self):
        output, summary = run_scenario("invalid-seq")
        self.assertEqual(summary["reject_seq"], 1)
        self.assertEqual(summary["accepted"], 10)
        self.assertIn("REJECT reason=seq-not-new counter=3 last=4", output)

    def test_wrap_counter_255_to_0_accepted_req_err_005(self):
        output, summary = run_scenario("wrap")
        self.assertEqual(summary["tx"], 11)
        self.assertEqual(summary["accepted"], 10)
        self.assertEqual(summary["reject_seq"], 1)
        self.assertEqual(summary["dropped"], 0)
        self.assertEqual(summary["timeouts"], 0)
        # the rolling counter genuinely reaches 255 and is ACCEPTED
        self.assertIn(
            "[0600ms][ECU2][COM] Com_RxIndication speed=90kph seq=255 result=ACCEPT",
            output,
        )
        # the 255 -> 0 wrap must be ACCEPTED, not rejected
        self.assertIn(
            "[0700ms][ECU2][COM] Com_RxIndication speed=95kph seq=0 result=ACCEPT",
            output,
        )
        self.assertNotIn("[0700ms][ECU2][COM] REJECT", output)
        # a stale replay of 255 after the wrap is still rejected
        self.assertIn(
            "[0800ms][ECU2][COM] REJECT reason=seq-not-new counter=255 last=0",
            output,
        )
        # normal increment continues after the wrap (re-synchronized stream)
        self.assertIn(
            "[0900ms][ECU2][COM] Com_RxIndication speed=105kph seq=1 result=ACCEPT",
            output,
        )

    def test_unknown_scenario_exits_2(self):
        completed = subprocess.run(
            [str(BINARY), "bogus"], capture_output=True, text=True
        )
        self.assertEqual(completed.returncode, 2)
        self.assertIn("Unknown scenario", completed.stderr)

    def test_uds_diagnostic_coexists_with_com(self):
        """UDS scenario runs the full COM loop while injecting a diagnostic
        request/response at 600ms.  Both COM and UDS traces must appear."""
        completed = subprocess.run(
            [str(BINARY), "uds"], check=True, capture_output=True, text=True
        )
        self.assertIn("UDS_INJECT", completed.stdout)
        self.assertIn("UDS_RESP", completed.stdout)
        # COM path delivers all frames (UDS injection is read-only on the bus)
        self.assertIn("delivered=", completed.stdout)


if __name__ == "__main__":
    unittest.main()
