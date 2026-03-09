# dosdoor Project Instructions

**Project Methodology:** The Unbroken Method for Human-AI Collaboration

## CRITICAL: READ FIRST BEFORE ANY WORK

### The Unbroken Method (Core Principles)

This project follows **The Unbroken Method** for human-AI collaboration. This isn't just project style - it's the core operational framework.

**The Seven Pillars:**

1. **Continuous Context** - Never break the conversation. Maintain momentum through collaboration checkpoints.
2. **Complete Ownership** - If you find a bug, fix it. No "out of scope."
3. **Investigation First** - Read code before changing it. Never assume.
4. **Root Cause Focus** - Fix problems, not symptoms.
5. **Complete Deliverables** - No partial solutions. Finish what you start.
6. **Structured Handoffs** - Document everything for the next session.
7. **Learning from Failure** - Document mistakes to prevent repeats.

**If you skip this, you will violate the project's core methodology.**

---

## Collaboration Checkpoint Discipline

**Use collaboration tool at EVERY key decision point:**

| Checkpoint | When | Purpose |
|-----------|------|---------|
| Session Start | Always | Evaluate request, develop plan, confirm with user |
| After Investigation | Before implementation | Share findings, get approval |
| After Implementation | Before commit | Show results, get OK |
| Session End | When work complete | Summary & handoff |

**Session Start Checkpoint Format:**
- CORRECT: "Based on your request to [X], here's my plan: 1) [step], 2) [step], 3) [step]. Proceed?"
- WRONG: "What would you like me to do?" or "Please confirm the context..."

The user has already provided their request. Your job is to break it into actionable steps and confirm the plan before starting work.

**Guidelines:**
- [OK] Investigate freely (reading files, searching code)
- [CHECKPOINT REQUIRED] Checkpoint BEFORE making changes
- [OK] Checkpoint AFTER implementation (show results)

### Session Start Protocol

**When:** User gives multi-step request OR recovering from handoff

**Steps:**
1. STOP - Do NOT start implementation
2. CALL user_collaboration with plan
3. WAIT for user response
4. ONLY THEN begin work

### After Investigation Protocol

**When:** After reading code/searching but BEFORE making changes

**Steps:**
1. STOP - Do NOT start making changes yet
2. CALL user_collaboration with findings
3. WAIT for user response
4. ONLY THEN make changes

### After Implementation Protocol

**When:** After completing work but BEFORE commit

**Steps:**
1. CALL user_collaboration with results
2. WAIT for confirmation
3. ONLY THEN commit

### Session End Protocol

**When:** Work complete or genuinely blocked

**Steps:**
1. CALL user_collaboration with summary
2. Create handoff documents

**NO CHECKPOINT NEEDED FOR:**

- Investigation/reading (always permitted - just do it)
- Tool execution and troubleshooting (iterate freely)
- Following through on approved plans (details don't need re-approval)
- Fixing obvious bugs in your scope (part of ownership)

**CRITICAL BALANCE:**

- Checkpoint MAJOR DECISIONS (what to build, how to approach)
- Execute DETAILS autonomously (specific implementations after approval)
- Complete requests CORRECTLY not just QUICKLY
- "Work autonomously" means AFTER approval, not INSTEAD OF approval

---

## Core Workflow

```
1. Read code first (investigation)
2. Use collaboration tool (get approval)
3. Make changes (implementation)
4. Test thoroughly (verify)
5. Commit with clear message (handoff)
```

## Tool-First Approach (MANDATORY)

**NEVER describe what you would do - DO IT:**
- WRONG: "I'll create a file with the following content..."
- RIGHT: [calls file_operations to create the file]

**IF A TOOL EXISTS TO DO SOMETHING, YOU MUST USE IT.**

**NO PERMISSION NEEDED (after checkpoint):**
- Don't ask "Should I proceed?" AFTER you've already checkpointed the plan
- Don't ask permission for investigation (reading files, searching, git status)

**PERMISSION REQUIRED (use user_collaboration):**
- Session start with multi-step work - present plan first
- Before making ANY code/config/file changes - show what you'll change
- Before destructive operations (delete, overwrite existing files)
- Before git commits - show what changed

## Investigation-First Principle

**Before making changes, understand the context:**
1. Read files before editing them
2. Check current state before making changes (git status, file structure)
3. Search for patterns to understand codebase organization
4. Use semantic_search when you don't know exact filenames/strings

**Don't assume - verify.**

## Complete the Entire Request

**What "complete" means:**
- Conversational: Question answered thoroughly with context and examples
- Task execution: ALL work done, ALL items processed, outputs validated, no errors

**Before declaring complete:**
- Did I finish every step the user requested?
- Did I process ALL items (if batch operation)?
- Did I verify results match requirements?
- Are there any errors or partial completions?

## Error Recovery - 3-Attempt Rule

**When a tool call fails:**
1. **Retry** with corrected parameters or approach
2. **Try alternative** tool or method
3. **Analyze root cause** - why are attempts failing?

**After 3 attempts:**
- Report specifics: what you tried, what failed, what you need
- Suggest alternatives or ask for clarification
- Don't just give up - offer options

## Ownership Model

**Primary Scope:**
- The problem user explicitly asked you to solve
- Anything directly blocking that problem
- Obvious bugs in the same system/module

**Secondary Scope (Fix if Quick, Ask if Complex):**
- Related issues discovered while solving primary
- Same system, would improve solution
- Quick wins (<30 min) that add value

**Out of Scope (Report & Ask):**
- Different systems/modules entirely
- Long-term refactoring tangents
- Architectural decisions affecting other systems

**Default: Own your primary scope completely. Ask before expanding to secondary scope.**

---

## Session Handoff Procedures

**When ending a session, ALWAYS create handoff directory:**

```
ai-assisted/YYYYMMDD/HHMM/
├── CONTINUATION_PROMPT.md  [MANDATORY]
├── AGENT_PLAN.md           [MANDATORY]
└── NOTES.md                [OPTIONAL]
```

### NEVER COMMIT Handoff Files

**Before every commit:**

```bash
git status
# If ai-assisted/ appears:
git reset HEAD ai-assisted/
```

### CONTINUATION_PROMPT.md (Mandatory)

Must be so complete the next developer can START WORK immediately without investigation.

**Required Sections:**
1. What Was Accomplished
2. Current State
3. What's Next
4. Key Discoveries & Lessons
5. Context for Next Developer
6. Quick Reference: How to Resume

### AGENT_PLAN.md (Mandatory)

**Required Sections:**
1. Work Prioritization Matrix
2. Task Breakdown
3. Testing Requirements
4. Known Blockers

---

## Anti-Patterns (What Not To Do)

| Anti-Pattern | Why It's Wrong | What To Do |
|--------------|----------------|------------|
| Describing instead of doing | Wastes time | Use tools immediately |
| Analysis paralysis | Perfect understanding impossible | Investigate to ~70%, then act |
| Permission-seeking after approval | Breaks momentum | Checkpoint once, then execute |
| Scope creep without asking | Loses focus | Stay in primary scope, ask to expand |
| Partial work without explanation | User doesn't know status | Report incomplete clearly |
| Committing handoff files | Pollutes repository | Always reset ai-assisted/ before commit |
| Giving up after few attempts | Problems are solvable | Exhaust approaches before reporting blocked |

---

## Project-Specific Conventions

**For technical details, see AGENTS.md:**
- Architecture overview
- Code style and patterns
- Build and test commands
- Platform-specific notes

**This document focuses on HOW TO WORK. AGENTS.md covers WHAT TO BUILD.**

---

*For universal agent behavior, see system prompt.*
*For technical reference, see AGENTS.md.*
