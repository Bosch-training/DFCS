"""FND-02 negative and positive tests for tools/verify_toolchain.py (NFR-010)."""

import json
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
VERIFY = REPO / "tools" / "verify_toolchain.py"
PYTHON_VERSION = ".".join(map(str, sys.version_info[:3]))


def run_verify(tmp_path: Path, pinned: str) -> subprocess.CompletedProcess:
    manifest = {
        "tools": {
            "python": {
                "version": pinned,
                "command": [sys.executable, "--version"],
                "pattern": "Python (\\d+\\.\\d+\\.\\d+)",
            }
        }
    }
    path = tmp_path / "toolchain.json"
    path.write_text(json.dumps(manifest), encoding="utf-8")
    return subprocess.run(
        [sys.executable, str(VERIFY), "--manifest", str(path)],
        capture_output=True,
        text=True,
        check=False,
    )


def test_verify_passes_on_matching_version(tmp_path):
    result = run_verify(tmp_path, PYTHON_VERSION)
    assert result.returncode == 0, result.stdout


def test_verify_fails_on_version_mismatch(tmp_path):
    result = run_verify(tmp_path, "0.0.1")
    assert result.returncode == 1
    assert "MISMATCH" in result.stdout
