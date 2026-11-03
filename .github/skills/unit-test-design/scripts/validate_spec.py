#!/usr/bin/env python3
"""Validate a DFCS unit test spec CSV (stdlib only).

Usage:
    validate_spec.py docs/test-specifications/DC-17-unit-test-spec.csv [--design PATH] [--root DIR]

Checks: header and column count, required cells, enums and patterns from
assets/unit-test-spec.schema.json, duplicate and colliding TC IDs, requirement IDs
that exist in the SRS, design requirement coverage, hard-rule negative cases,
unsourced numeric values. Exit code 1 if any error; warnings do not fail.
"""
import argparse
import csv
import json
import re
import sys
from pathlib import Path

SCHEMA = Path(__file__).resolve().parent.parent / "assets" / "unit-test-spec.schema.json"
REQ_RE = re.compile(r"\b(?:NFR|FR)-\d{3}\b")
TC_RE = re.compile(r"\bTC-[A-Z]{2,5}-\d{3}\b")
NUM_UNIT_RE = re.compile(
    r"(?<![\w.-])\d+(?:\.\d+)?\s?(?:ms|us|µs|ns|s|Hz|kHz|MHz|%|deg|rad|V|mV|A|mA|B|KB|kB|bytes)\b"
)
HARD_RULES = {
    "FR-002": "inhibit by default",
    "FR-061": "inhibit by default",
    "FR-067": "no auto re-arm",
    "FR-051": "MAVLink input rejected, no actuator change",
    "FR-054": "MAVLink input rejected, no actuator change",
    "NFR-016": "MAVLink input rejected, no actuator change",
}


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8") if path.is_file() else ""


def find_root(start: Path) -> Path:
    for p in [start, *start.parents]:
        if (p / "docs" / "Drone_Flight_Control_System_SRS.md").is_file():
            return p
    return Path.cwd()


def design_requirements(design_text: str) -> dict[str, bool]:
    """Requirement ID -> True if the design routes it to review/CI instead of a unit test."""
    ids: dict[str, bool] = {}
    in_sec = False
    for line in design_text.splitlines():
        if line.startswith("## "):
            in_sec = bool(re.match(r"## 2\b", line)) or "requirement" in line.lower()
            continue
        if in_sec and line.startswith("|"):
            cells = [c.strip() for c in line.strip().strip("|").split("|")]
            if cells:
                non_unit = bool(re.search(r"review|TC-CI-|documentation", cells[-1], re.I))
                for rid in REQ_RE.findall(cells[0]):
                    ids[rid] = non_unit
    return ids


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("spec", type=Path)
    ap.add_argument("--design", type=Path)
    ap.add_argument("--root", type=Path)
    args = ap.parse_args()

    spec = args.spec.resolve()
    root = (args.root or find_root(spec.parent)).resolve()
    schema = json.loads(SCHEMA.read_text(encoding="utf-8"))
    columns: list[str] = schema["x-columns"]
    props: dict = schema["properties"]
    optional_empty = set(schema.get("x-optional-empty", []))

    errors: list[str] = []
    warns: list[str] = []

    def err(row: str, msg: str) -> None:
        errors.append(f"[{row}] {msg}")

    def warn(row: str, msg: str) -> None:
        warns.append(f"[{row}] {msg}")

    srs = read_text(root / "docs" / "Drone_Flight_Control_System_SRS.md")
    stories = read_text(root / "docs" / "DFCS_Stories.md")
    jira_id = spec.name.split("-unit-test-spec")[0]

    design_path = args.design
    if not design_path:
        found = sorted((root / "docs" / "design").glob(f"{jira_id}-*.md"))
        design_path = found[0] if found else None
    design = read_text(design_path) if design_path else ""
    if not design:
        warn("file", f"no design doc found for {jira_id}; coverage checks skipped")

    try:
        with spec.open(encoding="utf-8", newline="") as fh:
            rows = list(csv.reader(fh))
    except (OSError, UnicodeDecodeError, csv.Error) as exc:
        print(f"ERROR: cannot read CSV: {exc}")
        return 1
    if not rows:
        print("ERROR: empty file")
        return 1

    if rows[0] != columns:
        err("header", f"header must be exactly: {','.join(columns)}")
    data = rows[1:]
    if not data:
        err("file", "no test case rows")

    known_tc = set(TC_RE.findall(srs + stories + design))
    other_specs = [p for p in (root / "docs" / "test-specifications").glob("*-unit-test-spec.csv") if p != spec]
    other_tc: set[str] = set()
    for p in other_specs:
        other_tc.update(TC_RE.findall(read_text(p).split("\n", 1)[-1]))
    srs_reqs = set(REQ_RE.findall(srs))

    seen: set[str] = set()
    covered: dict[str, list[dict]] = {}
    source_text = srs + design + stories

    for i, raw in enumerate(data, start=2):
        label = f"row {i}"
        if len(raw) != len(columns):
            err(label, f"expected {len(columns)} columns, found {len(raw)}")
            continue
        row = dict(zip(columns, (c.strip() for c in raw)))
        label = row["tc_id"] or label

        for col in columns:
            val = row[col]
            if "\n" in raw[columns.index(col)] or "\r" in raw[columns.index(col)]:
                err(label, f"{col}: must be single-line")
            if not val and col not in optional_empty:
                err(label, f"{col}: empty")
                continue
            if not val:
                continue
            p = props[col]
            if "enum" in p and val not in p["enum"]:
                err(label, f"{col}: '{val}' not in {p['enum']}")
            if "pattern" in p and not re.search(p["pattern"], val):
                err(label, f"{col}: '{val[:40]}' does not match {p['pattern']}")

        tc = row["tc_id"]
        if tc in seen:
            err(label, "duplicate tc_id")
        seen.add(tc)

        if row["status"] == "Existing" and tc not in known_tc:
            err(label, "status Existing but ID not in SRS, backlog, or design")
        if row["status"] == "NEW" and (tc in known_tc or tc in other_tc):
            err(label, "status NEW but ID already used elsewhere")

        for rid in filter(None, row["requirement_ids"].split(";")):
            if rid not in srs_reqs:
                err(label, f"requirement {rid} not in SRS")
            covered.setdefault(rid, []).append(row)

        mocks = row["mocks"]
        if mocks and mocks.lower() != "none":
            for part in mocks.split(";"):
                if ":" not in part:
                    err(label, f"mocks: '{part.strip()[:40]}' must be '<dependency>: <behaviour>'")
        steps = row["steps"]
        if steps:
            parts = [s.strip() for s in steps.split(" | ")]
            for n, s in enumerate(parts, start=1):
                if not s.startswith(f"{n}. "):
                    err(label, f"steps: item {n} must start with '{n}. '")
                    break

        if re.search(r"flight-?worth|certified|safe to fly", " ".join(raw), re.I) and "NFR-017" not in row["requirement_ids"]:
            warn(label, "mentions flightworthiness/certification; NFR-017 forbids such claims")

        for col in ("preconditions", "inputs", "steps", "expected_result", "notes"):
            for m in NUM_UNIT_RE.finditer(row[col]):
                if m.group(0) not in source_text:
                    warn(label, f"{col}: '{m.group(0)}' not found in SRS/design/backlog; use TBD (...) unless sourced")

    if design:
        for rid, non_unit in sorted(design_requirements(design).items()):
            if rid in covered:
                continue
            if non_unit:
                warn("coverage", f"{rid} routed to review/CI by the design; list as out of scope in the reply")
            else:
                err("coverage", f"design requirement {rid} has no test case")
        for rid, rule in HARD_RULES.items():
            if rid in design and not any(r["type"] in ("negative", "fault") for r in covered.get(rid, [])):
                err("hard-rule", f"{rid} ({rule}) is referenced by the design but has no negative/fault case")
        for tc in sorted(set(TC_RE.findall(design)) - seen):
            warn("coverage", f"design/backlog TC {tc} not in spec; list it as out of scope in the reply if not unit level")

    for line in errors:
        print("ERROR:", line)
    for line in warns:
        print("WARN: ", line)
    print(f"{len(data)} cases, {len(errors)} errors, {len(warns)} warnings")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
