"""Dedicated tests for BSW modules: WdgM (Watchdog Manager) and NvM."""
import subprocess
import unittest

BIN = "build/multiecu_demo.exe"


def _run(scenario: str) -> str:
    result = subprocess.run(
        [BIN, scenario],
        capture_output=True, text=True, timeout=10,
    )
    return result.stdout


class WdgMTests(unittest.TestCase):
    """Verify WdgM alive-counter supervision and timeout escalation."""

    def test_wdgm_checkpoint_logged_normal(self):
        out = _run("normal")
        self.assertIn("WdgM", out)

    def test_wdgm_ok_on_regular_checkpoints(self):
        out = _run("normal")
        self.assertIn("Task_100ms activated", out)
        self.assertNotIn("deadline missed", out)

    def test_wdgm_timeout_escalates_to_failed(self):
        out = _run("timeout")
        self.assertIn("deadline missed", out)

    def test_wdgm_expired_on_sustained_miss(self):
        out = _run("timeout")
        self.assertIn("EXPIRED", out)

    def test_wdgm_recovery_from_failed_to_ok(self):
        out = _run("wdgm-recovery")
        self.assertIn("deadline missed", out)
        self.assertIn("WDGM", out)

    def test_wdgm_ok_status_preserved_in_normal(self):
        out = _run("normal")
        self.assertNotIn("deadline missed", out)
        self.assertNotIn("EXPIRED", out)


class NvMTests(unittest.TestCase):
    """Verify NvM block read/write persistence across ECU tasks."""

    def test_nvm_persist_normal(self):
        out = _run("normal")
        self.assertIn("[INIT][ECU1]", out)
        self.assertIn("[INIT][ECU2]", out)

    def test_nvm_write_on_tx(self):
        out = _run("normal")
        self.assertIn("Com_SendSignal", out)

    def test_nvm_write_on_rx_accept(self):
        out = _run("normal")
        self.assertIn("result=ACCEPT", out)

    def test_nvm_persist_timeout(self):
        out = _run("timeout")
        self.assertIn("[INIT][ECU1]", out)
        self.assertIn("[INIT][ECU2]", out)

    def test_nvm_persist_wrap(self):
        out = _run("wrap")
        self.assertIn("[INIT][ECU1]", out)
        self.assertIn("result=ACCEPT", out)

    def test_nvm_persist_uds(self):
        out = _run("uds")
        self.assertIn("UDS_INJECT", out)

    def test_nvm_persist_invalid_range(self):
        out = _run("invalid-range")
        self.assertIn("REJECT reason=range", out)


if __name__ == "__main__":
    unittest.main()
