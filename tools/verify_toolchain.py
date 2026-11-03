#!/usr/bin/env python3
"""Verify installed tool versions against tools/toolchain.json (NFR-010)."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

DEFAULT_MANIFEST = Path(__file__).with_name("toolchain.json")
OK, MISMATCH, MISSING, SKIPPED = "OK", "MISMATCH", "MISSING", "SKIPPED"


def installed_version(spec: dict) -> str | None:
    """Return the installed version, or None when the tool is absent."""
    command = spec["command"]
    if shutil.which(command[0]) is None:
        return None
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    match = re.search(spec["pattern"], result.stdout + result.stderr, re.MULTILINE)
    return match.group(1) if match else ""


def check_tool(spec: dict) -> tuple[str, str]:
    found = installed_version(spec)
    if found is None:
        return (SKIPPED if spec.get("optional") else MISSING), "not installed"
    if found != spec["version"]:
        return MISMATCH, f"installed {found or 'unparsable'}, pinned {spec['version']}"
    return OK, found


def verify(manifest_path: Path) -> int:
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    failed = False
    for name, spec in manifest["tools"].items():
        status, detail = check_tool(spec)
        print(f"{status:9} {name}: {detail}")
        failed |= status in (MISMATCH, MISSING)
    return 1 if failed else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    args = parser.parse_args()
    return verify(args.manifest)


if __name__ == "__main__":
    sys.exit(main())
