#!/usr/bin/env python3
"""Print the next free TC ID for an area.

Usage:
    next_tc_id.py STATE [COUNT] [--root DIR]

Scans the SRS, backlog, design docs, and existing specs/reviews under docs/ so a NEW case never reuses an ID.
"""
import argparse
import re
import sys
from pathlib import Path


def find_root(start: Path) -> Path:
    for p in [start, *start.parents]:
        if (p / "docs" / "Drone_Flight_Control_System_SRS.md").is_file():
            return p
    return Path.cwd()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("area", help="e.g. STATE, MIX, COMM")
    ap.add_argument("count", nargs="?", type=int, default=1)
    ap.add_argument("--root", type=Path)
    args = ap.parse_args()

    area = args.area.upper()
    root = (args.root or find_root(Path.cwd())).resolve()
    pat = re.compile(rf"\bTC-{re.escape(area)}-(\d{{3}})\b")
    used: set[int] = set()
    for p in (root / "docs").rglob("*"):
        if p.is_file() and p.suffix in {".md", ".csv"}:
            used.update(int(n) for n in pat.findall(p.read_text(encoding="utf-8", errors="ignore")))

    nxt = max(used, default=0) + 1
    for n in range(nxt, nxt + args.count):
        print(f"TC-{area}-{n:03d}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
