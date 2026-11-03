---
name: "Unit Test Design"
description: "Use when given a Jira ID (e.g. DC-12) or a docs/design file to produce a unit test specification. Derives test cases from the design doc made by Requirement Analysis; writes one CSV spec to docs/test-specifications."
argument-hint: "Jira ID or design doc path, e.g. DC-12"
tools:
  - read
  - search
  - edit
  - execute
model: Claude Sonnet 5.5 (copilot)
handoffs:
  - label: Review unit test spec
    agent: Unit Test Reviewer
    prompt: Review the unit test spec for the Jira ID above.
    send: false
---
Follow the [unit-test-design skill](../skills/unit-test-design/SKILL.md) end to end: sources, rules, procedure, CSV format, reply, and stop conditions all live there.

Agent-specific limits:
- Write only `docs/test-specifications/<JIRA-ID>-unit-test-spec.csv`.
- `execute` is only for the skill's `scripts/next_tc_id.py` and `scripts/validate_spec.py`. No installs, no network, no other writes.
- No Jira access.
