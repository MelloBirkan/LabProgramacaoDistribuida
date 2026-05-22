# 0 trigger_workflow

> Converted from a former Claude Code command. Apply the question-tool and tool-equivalence rules from the parent SKILL.md; keep Claude `AskUserQuestion` support and use Codex `request_user_input` when available.

## Collaboration Philosophy

The philosophy and goal of this workflow is alignment, coming to a set of decisions made together, not deliverables to rush toward.

Value system:

- Questions are investments in correctness, not overhead
- Surfacing assumptions early is cheap; fixing wrong work is expensive
- Getting it right the first time is faster than iterating on wrong work
- Multiple rounds of clarification is normal and encouraged

Before proceeding to the next step:

1. Surface your key assumptions with genuine honesty
2. Continue asking questions until genuinely confident
3. Only proceed to the next step when you and the user have shared understanding

## Multi-Round Clarification

If uncertainty remains after initial interview questions, present more interview questions.

- Multiple rounds of clarification is normal and encouraged
- Don't feel pressured to draft after one round of answers
- The goal is shared understanding, not speed

## Tools

- Use Claude Code `AskUserQuestion` or Codex `request_user_input` when available for all interview and clarification questions. In Claude, present 2-4 distinct options per question ("Other" is added automatically) and ask up to 4 questions per round. In Codex, follow the parent SKILL.md limits.
- When the user's request involves specific technologies, frameworks, or domains you need more context on, use MCP tools to research: `ref_search_documentation` / `ref_read_url` for library and API docs, `web_search_exa` or `tavily_search` for general web research.

## User Request

The user's current request or stage arguments

## Processing User Request

1. Internalize the user's request above. Use interview questions to resolve ambiguous requirements, fill in missing details, etc. Multiple rounds of clarification are expected. Reach alignment and shared understanding with the user.
2. Assess the nature of the work — does this involve product-level decisions (user experience, new flows, behavior changes visible to users) or is it purely technical (refactoring, performance, infrastructure, bug fixes)?
3. Once clarified, present a very concise summary of the agreed requirements.
4. Create the epic folder at `.mf/<epic-name>/` where `<epic-name>` is a short, descriptive kebab-case name derived from the work (e.g., `.mf/auth-session-refactor/`). Create `.mf/` first if it doesn't exist. This folder is where all artifacts for this work will live (plan.md, tickets/).

Note: This step is for REQUIREMENT GATHERING and FOLDER CREATION only. No spec artifacts are created here.

## Next Step

Suggest the user proceed with `$mf-plan 1-plan` to create the plan.

## Acceptance Criteria

- The user's request is turned into precise requirements via structured interviewing - no assumptions.
- The user is satisfied with the requirements.

## Principles

- User intent first: Workflow guides but user directs.