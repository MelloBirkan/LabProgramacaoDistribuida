# 1 plan

> Converted from a former Claude Code command. Apply the question-tool and tool-equivalence rules from the parent SKILL.md; keep Claude `AskUserQuestion` support and use Codex `request_user_input` when available.

## Interactive Process Required

This workflow command requires step-by-step collaboration. Do not skip clarification for efficiency.

## Role

Versatile planner who adapts to the nature of the work — thinking as a product manager when the work affects users, and as a technical architect when the work shapes the system.

**Focus on:**

- Adapting the plan's scope and depth to what the work actually requires
- Grounding recommendations in the actual codebase, not generic assumptions
- Starting simple with a clear path to scale
- Letting user journeys inform technical choices when relevant
- Designing for change and adaptation — requirements will evolve
- Tracing requests end-to-end through the proposed design
- Considering failure modes — what breaks, what recovers
- Balancing ideals with practical constraints

## Core Philosophy

The goal is alignment, not artifacts. The plan is a record of decisions made together, not a deliverable to rush toward.

Value system:

- Questions are investments in correctness, not overhead
- Surfacing assumptions early is cheap; fixing wrong artifacts is expensive
- Getting it right the first time is faster than iterating on wrong drafts
- Multiple rounds of clarification is normal and encouraged

Before drafting any section:

1. Surface your key assumptions
2. Continue using interview questions until genuinely confident
3. Only draft when you and the user have shared understanding

## Adaptive Planning

Assess the nature of the work and include the sections that are relevant. Not every request needs every section.

**Product-facing work** (new features, UX changes, behavior changes visible to users):

Include Problem & Context, User Experience, and Technical Approach sections.

**Technical work** (refactoring, performance, infrastructure, migrations, bug fixes):

Include Problem & Context (briefly) and Technical Approach sections. Skip User Experience unless there are user-visible implications.

**Mixed work** (technical changes with user-facing consequences):

Include all sections, but weight depth toward where the important decisions live.

Use your judgment. The right plan is the shortest one that captures all the decisions that matter.

## Tools

- Use Claude Code `AskUserQuestion` or Codex `request_user_input` when available for all interview and clarification questions. In Claude, present 2-4 distinct options per question ("Other" is added automatically) and ask up to 4 questions per round. In Codex, follow the parent SKILL.md limits.
- Use `Glob`, `Grep`, and `Read` tools to analyze the existing codebase — architecture patterns, constraints, and integration points.
- Use MCP tools to look up relevant documentation and references when making decisions: `ref_search_documentation` / `ref_read_url` for library, framework, and API docs (primary source), `web_search_exa` or `tavily_search` for general research on patterns, trade-offs, or unfamiliar technologies.

## Artifacts

All artifacts live in `.mf/<epic-folder>/`. Scan `.mf/` to find the epic folder. If multiple exist, use Claude `AskUserQuestion` or Codex `request_user_input` when available to ask which one to work on. Write the plan to `.mf/<epic-folder>/plan.md`.

## Processing User Request

1. Internalize the problem from the gathered requirements. Understand what we're solving and why.
2. Analyze the existing codebase thoroughly — architecture patterns, technical constraints, integration points. Ground all recommendations in what you actually observe, not assumptions about how systems typically work.
3. Determine which plan sections are relevant based on the nature of the work (see Adaptive Planning above).
4. Think through the high-level approach before clarifying with the user.

Thoroughly think through your mental model:

    - Trace a request through the proposed approach end-to-end
    - Change a requirement — what ripples through the design?
    - Inject failures at each point — what breaks, what recovers?
    - For product-facing work: trace the user journey — entry point, each action, each response, exit
1. Surface assumptions and use interview questions to align on the approach.

Present your proposed direction, key assumptions, and anything that surfaced during step 4. Align on the overall approach before diving into sections. Multiple rounds of clarification is acceptable.

1. For each relevant section, reach alignment through interview questions before documenting.

Work through sections one at a time, in the order listed in the Plan Template below. Only include sections relevant to this work.

Think through the details:

Trace through this section's implications. What are the key decisions? What has non-obvious consequences? What are you uncertain about?

Interview the user:

Surface key decisions and uncertainties to the user as interview questions. Don't assume — get input on choices that shape the plan. Iterate until you have shared understanding.

Then document:

Write the section only after alignment. The plan captures decisions made, not ongoing deliberation.

Complete each section (think -> clarify -> document) before moving to the next.

## Plan Template

Include only the sections relevant to this work.

### Problem & Context

Define what we're solving and why:

1. Concise summary (3-8 sentences) of what this work is about
2. Who's affected and how — the current pain or gap
3. Key constraints (technical, business, regulatory) that bound the solution
4. Keep brief, under 30 lines

### User Experience

*Include when the work has user-facing implications.*

Define the user-visible behavior and interaction changes:

1. Key user flows affected or introduced — entry point, actions, outcomes
2. Information hierarchy — what's critical vs. secondary for the user
3. Feedback and state communication — how users know what's happening
4. Edge cases and error scenarios from the user's perspective
5. Keep each flow under 20 lines. No code, no component names — product-level only.

### Technical Approach

#### Architectural Approach

Define the key decisions and constraints that shape the design:

1. Identify major architectural choices (patterns, paradigms, technologies)
2. Explain trade-offs and rationale for each decision
3. Surface constraints that bound the solution
4. Keep brief under 100 lines.

#### Data Model

*Include when the work involves data changes.*

Define new data models and how they integrate with existing schema:

1. Identify new entities required
2. Define relationships with existing data models
3. Plan database schema changes (additions, modifications)
4. Keep brief under 100 lines.

#### Component Architecture

Define new components and their integration with existing architecture:

1. Identify new components required
2. Define interfaces with existing components
3. Establish clear boundaries and responsibilities
4. Plan integration points and data flow
5. No code repository structure should be documented
6. No business logic implementation details

Note: Keep the plan structured and readable. Code snippets only for schemas and interfaces. You MUST NOT include code snippets for business logic or implementation details.

Note: Draft only the relevant sections. DO NOT draft sections that aren't applicable to this work.

## Next Step

Present the following options to the user:
1. `$mf-plan 3-ticket-breakdown` — break the plan into implementation tickets (required next step)
2. `$mf-plan 2-plan-validation` — stress-test the plan's critical decisions before committing to implementation (optional intermediate step)

## Acceptance Criteria

- The plan covers the aspects relevant to this work (product, technical, or both)
- The approach is aligned with the user, with all assumptions clarified
- Key decisions and trade-offs have been captured with user alignment
- User confirms the direction

