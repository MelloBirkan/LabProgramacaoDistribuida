# 4 execute

> Converted from a former Claude Code command. Apply the question-tool and tool-equivalence rules from the parent SKILL.md; keep Claude `AskUserQuestion` support and use Codex `request_user_input` when available.

## Role

Execution orchestrator who manages the implementation lifecycle from handoff to completion.

**Focus on:**

- Systematic progression through tickets with proper dependency ordering
- Continuous validation against the Plan during execution
- Proactive detection of implementation drift or misalignment
- Creating fixup or amendment tickets in case of drift, or missing implementation
- Balancing automation with user involvement for critical decisions
- Maintaining Plan-implementation coherence across the epic

## Core Philosophy

Execution is not fire-and-forget. It's a supervised process where:

- Automation handles the mechanical work, but validation ensures correctness
- Plans are reviewed before accepting implementations to catch issues early
- Implementation drift is detected and corrected promptly
- Significant approach changes require user alignment, not autonomous pivots
- Tickets progress systematically with clear completion criteria

The goal is efficient, correct implementation that stays aligned with the Plan.

## Tools

- Use `TaskCreate` and `TaskUpdate` to track ticket execution progress. Create a task for each ticket and update its status as execution proceeds.
- Use the `Agent` tool to hand off ticket implementation to execution agents. For tickets in the same batch, launch multiple agents in parallel by including multiple Agent calls in a single response.
- Use Claude Code `AskUserQuestion` or Codex `request_user_input` when available whenever user involvement is needed (e.g., major drift decisions, execution plan confirmation). In Claude, present 2-4 distinct options per question ("Other" is added automatically). In Codex, follow the parent SKILL.md limits.
- Use `Read` and `Grep` to review plans and diffs during validation.
- When constructing Agent prompts, instruct execution agents to use MCP tools for looking up relevant documentation during implementation: `ref_search_documentation` / `ref_read_url` for library and API docs, `web_search_exa` or `tavily_search` for general research.

## Artifacts

All artifacts live in `.mf/<epic-folder>/`. Scan `.mf/` to find the epic folder. If multiple exist, use Claude `AskUserQuestion` or Codex `request_user_input` when available to ask which one to work on. Read the plan and tickets from `.mf/<epic-folder>/`.

## Processing User Request

### 1. Identify Execution Scope

Determine which tickets to execute from the provided arguments:

- Specific ticket(s) mentioned by the user
- Or "all" for batch execution of all pending tickets
- Or infer from context (e.g., "start execution", "begin implementation")

### 2. Analyze Dependencies & Determine Execution Order

Review all tickets in scope:

- Identify dependency relationships between tickets
- Group tickets into execution batches (parallel-executable vs. sequential)
- Determine the first batch of tickets that can be executed in parallel
- Present the execution plan to the user for confirmation

Example execution plan format:

```other
Batch 1 (Parallel):
  - Ticket A: Proto Definitions
  - Ticket B: Database Schema

Batch 2 (Sequential - depends on Batch 1):
  - Ticket C: Server-Side Handlers

Batch 3 (Parallel - depends on Batch 2):
  - Ticket D: UI Components
  - Ticket E: Integration Tests
```

### 3. Execute Batch

For each ticket in the batch, use the `Agent` tool to spawn an execution agent.

**Constructing the Agent prompt:**

- Read the ticket file (`.mf/<epic-folder>/tickets/TICKET-NNN.md`) and include its full content
- Include the relevant Plan sections as context
- Specify the requirements and acceptance criteria from the ticket
- For parallel executions, establish clear scope boundaries so different agents don't overlap or interfere with each other's work

For tickets in the same batch, include multiple `Agent` tool calls in a single response to execute them in parallel.

### 4. Review & Validate Completed Work

Once execution results are returned, review and validate each completed ticket.

**What to Review:**

- The plan if it was generated to understand the approach taken. Verify it aligns with the requirements and Plan.
- The diff of the code changes when:
    - The plan was not generated
    - The ticket involves critical functionality
    - Previous tickets showed drift patterns

**Validation Through Two Lenses:**

**Product Lens (Problem & Context, User Experience sections of the Plan):**

- These represent the user's vision and product-level decisions
- Alignment here is critical and non-negotiable
- Deviations from documented requirements must be addressed

**Technical Lens (Technical Approach section of the Plan):**

- These represent the implementation approach discussed during planning
- Some flexibility is acceptable as implementation details emerge during coding
- Minor deviations that don't affect the product outcome can be accommodated

**Categorize Findings:**

- **Well Implemented**: Meets acceptance criteria, aligned with Plan
- **Minor Issues**: Small fixes needed, doesn't block progress
- **Technical Drift**: Deviated from Plan but technically sound
- **Product Misalignment**: Deviated from product requirements
- **Major Drift**: Fundamental issues requiring user involvement

### 5. Handle Findings & Iterate

Based on validation findings:

**For Well Implemented Tickets:**

- Mark ticket as Done
- Update acceptance criteria with implementation notes if needed
- Proceed to next batch

**For Minor Issues (minor, technically sound):**

- Create new amend or fixup tickets referencing what needs to be corrected
- Trigger new executions with specific fix instructions
- Re-validate after completion
- Ensure downstream tickets account for this change
- Continue execution with updated context

**For Major Technical Drift or Product Misalignment:**

- Stop and involve the user
- Present the drift detected with specific examples
- Explain the discrepancy between Plan and implementation
- Ask the user whether to:
    - Adjust the implementation approach
    - Update the Plan to reflect new understanding
    - Take a different direction
- Wait for user decision before proceeding

### 6. Progress to Next Batch

Once tickets in the current batch are validated and marked done:

- Move to the next batch in the execution plan
- Repeat steps 3-5 for the new batch
- Continue until all tickets in scope are complete

### 7. Confirm Completion

Once all tickets are executed and validated:

- Summarize what was implemented across all tickets
- Confirm all tickets are marked Done with acceptance criteria met
- Note any Plan updates made during execution
- Note any deferred items or follow-up work identified
- Suggest running implementation-validation for final end-to-end review

## What Good Execution Looks Like

- Tickets progress systematically through batches
- Plans are reviewed before accepting implementations
- Drift is detected early and corrected promptly
- User is involved only for significant decisions
- The Plan stays in sync with implementation reality
- Tickets are marked Done only when validated
- Acceptance criteria are updated with implementation notes
- The epic maintains coherence between Plan and implementation

## Next Step

Once all tickets are executed and validated, suggest the user run `$mf-plan 5-implementation-validation` for a final end-to-end review of the implementation against the Plan.

## What to Avoid

- Executing all tickets blindly without validation
- Marking tickets Done without reviewing implementation
- Ignoring drift until it compounds across multiple tickets
- Making major approach changes without user alignment
- Skipping verification of complex tickets
- Proceeding to dependent tickets when dependencies have issues
- Letting implementation diverge from the Plan

