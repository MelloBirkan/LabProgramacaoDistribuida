# 3  ticket-breakdown

> Converted from a former Claude Code command. Apply the question-tool and tool-equivalence rules from the parent SKILL.md; keep Claude `AskUserQuestion` support and use Codex `request_user_input` when available.

## Tools

- Use Claude Code `AskUserQuestion` or Codex `request_user_input` when available to present refinement options and gather feedback on the ticket breakdown. In Claude, present 2-4 distinct options per question ("Other" is added automatically) and ask up to 4 questions per round. In Codex, follow the parent SKILL.md limits.
- Use `Read` to review the Plan.

## Artifacts

All artifacts live in `.mf/<epic-folder>/`. Scan `.mf/` to find the epic folder. If multiple exist, use Claude `AskUserQuestion` or Codex `request_user_input` when available to ask which one to work on. Read the plan from `.mf/<epic-folder>/plan.md` and write tickets to `.mf/<epic-folder>/tickets/TICKET-NNN.md`.

## Processing User Request

1. Infer the area to prioritize for tickets from the arguments.
2. Review the Plan and identify natural work units.
3. Apply best judgment to create ticket breakdown:

Consider:

    - How to group work (by component, by flow, by layer)
    - What dependencies exist between pieces of work
    - What order makes sense for implementation

    Prefer coarse groupings:

    - Group by component or layer, not by individual function
    - Group by flow, not by step
    - Each ticket should be story-sized-meaningful work, not a single function

    Anti-pattern: Do NOT over-breakdown. The minimal least set of tickets is better than multiple small ones.

1. Draft tickets using best judgment:

For each ticket:

    - **Title**: Action-oriented
    - **Scope**: What's included, what's explicitly out
    - **Spec references**: Link to relevant Plan sections
    - **Dependencies**: What must be completed first (if any)
1. Present the proposed ticket breakdown to the user.

Use a mermaid diagram to visualize ticket dependencies for quick reference.

1. After presenting, offer refinement options (whatever are applicable and make sense):
    - Change ticket granularity (combine related work or split for parallel work/ clarity)
    - Reorganize dependencies or implementation order
    - Different grouping approach (by component, by flow, etc.)
1. Iterate based on feedback until the breakdown is right.
2. Once finalized, write each ticket as an individual file to `.mf/<epic-folder>/tickets/TICKET-NNN.md`.

## Next Step

Present the following options to the user:
1. `$mf-plan 4-execute` — begin implementation of the tickets (required next step)
2. `$mf-plan 7-cross-artifact-validation` — validate consistency across the plan and tickets before executing (optional intermediate step)