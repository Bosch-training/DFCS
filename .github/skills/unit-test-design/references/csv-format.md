# CSV format reference

Machine-readable source of truth: [../assets/unit-test-spec.schema.json](../assets/unit-test-spec.schema.json). Starter file: [../assets/unit-test-spec.template.csv](../assets/unit-test-spec.template.csv).

Output: `docs/test-specifications/<JIRA-ID>-unit-test-spec.csv`, UTF-8, comma-separated, RFC 4180 quoting, one header row, one row per case. Quote any field with a comma, quote, or newline; double embedded quotes. Every field is single-line.

Header (exact order):

`tc_id,status,title,requirement_ids,module,type,framework,preconditions,mocks,inputs,steps,expected_result,evidence,notes`

| Column | Rule |
|---|---|
| `tc_id` | `TC-<AREA>-NNN`, unique. Reuse design/backlog IDs; new ones from `scripts/next_tc_id.py`. |
| `status` | `Existing` (ID in design or backlog) or `NEW` (needs RTM row). |
| `title` | Behaviour: given X, the unit does Y. |
| `requirement_ids` | `;`-separated SRS IDs that exist (`FR-xxx`, `NFR-xxx`). |
| `module` | Unit under test as named in the design. |
| `type` | `nominal`, `boundary`, `fault`, or `negative`. |
| `framework` | `GoogleTest` (C++) or `Unity` (C). |
| `preconditions` | State of the unit before the steps. |
| `mocks` | `none`, or `;`-separated `<dependency>: <mocked behaviour>`. |
| `inputs` | Concrete input values; unsourced numbers are `TBD (...)`. |
| `steps` | Numbered, separated by ` | `: `1. ... | 2. ...`. |
| `expected_result` | Observable, deterministic: return value, output state, or recorded mock call. |
| `evidence` | What proves pass or fail (assert, log, coverage report). |
| `notes` | `TBD (...)` with story or OD-xxx, hard-rule tags, design gaps. May be empty. |

Validate with `python3 .github/skills/unit-test-design/scripts/validate_spec.py <spec> [--design <design.md>]`.
