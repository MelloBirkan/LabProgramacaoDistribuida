---
name: mf-plan
description: Collaborative planning workflow converted from former Claude Code plan commands. Use when the user wants to gather requirements, create or revise an adaptive plan, break it into implementation tickets, execute tickets, validate implementation against the plan, or check plan and ticket consistency. Trigger on plan workflow, planning, ticket breakdown, implementation validation, revise requirements, cross-artifact validation, or former slash-command style requests such as plan stages.
---

# MF Plan

## Operating model

Use this skill as a staged, question-driven planning workflow. The goal is shared understanding before artifacts, not speed.

- Ask questions until the requirement, plan, or validation decision is genuinely clear.
- Surface assumptions explicitly before committing them to artifacts.
- Keep artifacts in `.mf/<epic-name>/`.
- Treat the Plan as the source of truth and tickets as derivatives.
- Load only the reference for the current stage, then follow that stage precisely.

Former slash-command names in the references are aliases. Interpret `/plan:1-plan` as "use `$mf-plan` with stage `1-plan`."

## Question tooling

For all interviews, clarification rounds, refinement choices, and user decisions, use the best available user-question mechanism:

- Claude Code: keep using `AskUserQuestion`.
- Codex Plan mode: use `request_user_input` when it is available. Ask 1-3 short questions, give 2-3 mutually exclusive choices per question, put the recommended choice first when there is one, and rely on the client-added "Other" option.
- Codex Default mode or any environment without a structured question tool: ask concise plain-text questions and wait for the user. Do not simulate a tool call.

Keep each round focused. Claude references allow up to 4 questions per round; Codex `request_user_input` supports up to 3. Multiple clarification rounds are expected.

## Tool equivalents

The reference files were converted from Claude Code commands and may mention Claude tool names. Apply these equivalents in Codex:

| Claude command instruction | Codex equivalent |
| --- | --- |
| `AskUserQuestion` | `request_user_input` when available, otherwise concise plain-text questions |
| `Glob`, `Grep`, `Read` | `rg --files`, `rg`, and shell file reads such as `sed` or `nl` |
| `Bash` | `exec_command` |
| `TaskCreate`, `TaskUpdate` | `update_plan` |
| `Agent` | `spawn_agent` only when current instructions allow delegation and the user has explicitly permitted agent work; otherwise do the work locally |
| `ref_search_documentation`, `ref_read_url` | use the available Ref MCP tools if present; otherwise use official docs or web search when current instructions require current or source-backed information |
| `web_search_exa`, `tavily_search` | use the available web-search mechanism, preferring primary sources for technical claims |

If current system or developer instructions conflict with a converted reference, follow the current higher-priority instruction.

## Stage selection

Infer the stage from the user request. If the user gives no stage and there is no active `.mf` epic, start with `0-trigger-workflow`. If there are multiple epic folders and the correct one is unclear, ask the user which one to use.

| Stage | Reference | Use when |
| --- | --- | --- |
| `0-trigger-workflow` | `references/0-trigger-workflow.md` | Turn an initial request into clarified requirements and create the `.mf/<epic-name>/` folder. |
| `1-plan` | `references/1-plan.md` | Create the adaptive plan in `.mf/<epic-name>/plan.md`. |
| `2-plan-validation` | `references/2-plan-validation.md` | Stress-test the plan before ticketing or implementation. |
| `3-ticket-breakdown` | `references/3-ticket-breakdown.md` | Convert the plan into story-sized tickets. |
| `4-execute` | `references/4-execute.md` | Execute tickets in dependency order and validate each batch. |
| `5-implementation-validation` | `references/5-implementation-validation.md` | Review implementation against the plan and tickets. |
| `6-revise-requirements` | `references/6-revise-requirements.md` | Propagate changed requirements through the plan and tickets. |
| `7-cross-artifact-validation` | `references/7-cross-artifact-validation.md` | Check consistency between plan sections and tickets. |

## Artifact rules

- Create `.mf/` if it does not exist.
- Derive `<epic-name>` as short, descriptive kebab-case.
- Store the plan at `.mf/<epic-name>/plan.md`.
- Store tickets at `.mf/<epic-name>/tickets/TICKET-NNN.md`.
- Update existing artifacts surgically. Preserve decisions that still hold.
- When a stage says to ask for confirmation or alignment, do that before writing or revising artifacts.

## Research rules

Ground technical recommendations in the actual codebase before proposing architecture. Use current, primary documentation when API behavior, library constraints, platform rules, or framework guidance might matter.

When documentation research is needed, prefer official docs through Ref MCP if available. If using web search, compare dates and source authority, and cite sources when reporting externally.
