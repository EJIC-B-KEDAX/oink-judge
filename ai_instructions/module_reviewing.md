# Module Reviewing Guide for oink-judge

## Overview

This instruction describes the full procedure for reviewing a single module located at `modules/<module>/`. The review produces two output files:

- `modules/<module>/issues.md` — all code and architecture issues found
- `modules/<module>/tests_to_add.md` — all missing test cases

## Step 1: Read the Entire Module

Read every file in the module directory:

```
modules/<module>/
├── CMakeLists.txt
├── include/oink_judge/<module>/
│   ├── *.h
│   └── *.inl
├── src/
│   └── *.cpp
└── tests/
    ├── CMakeLists.txt
    ├── test_*.cpp
    └── resources/
```

Build a mental model of the module: its public API, internal helpers, data flow, and dependencies on other modules.

## Step 2: Check Previous Review Results

Before starting the review, check if `modules/<module>/issues.md` or `modules/<module>/tests_to_add.md` already exist from a previous review.

If **either file exists**:

1. Read the existing file(s)
2. For each previously reported issue or missing test case, verify against the current code whether it has been **fixed** or **still remains**
3. Keep track of which issues are resolved and which persist

When generating the new `issues.md` and `tests_to_add.md`:

- **Do not include fixed issues** — only write issues that still exist in the code
- Previously reported issues that remain unfixed should be included in the new file (they may be renumbered or regrouped as needed)
- New issues discovered during this review are added alongside the remaining old ones
- There is no need to mention that an issue was "previously reported" — treat remaining and new issues uniformly

If neither file exists, skip this step and proceed normally.

## Step 3: Find Potential Issues

Analyze the code for bugs and unhandled cases:

- **Null / empty state access:** dereferencing `nullptr`, accessing empty containers, using `std::optional` without checking `has_value()`
- **Resource leaks:** unclosed handles, missing RAII wrappers, raw `new` without matching `delete`
- **Exception safety:** operations that can throw inside destructors, missing `noexcept` on move operations, catch-all swallowing errors silently
- **Concurrency issues:** data races, missing locks, deadlock potential, shared mutable state without synchronization
- **Boundary conditions:** off-by-one errors, integer overflow/underflow, empty input handling
- **Error handling:** ignored return values, unchecked error codes, missing validation at public API boundaries
- **Lifetime issues:** dangling references, use-after-move, returning references to locals

## Step 4: Check Architecture

Evaluate the module's design:

- **Single responsibility:** does the module or any class do too many unrelated things?
- **Dependency direction:** does the module depend on higher-level modules it shouldn't? Are there circular dependencies?
- **Interface design:** are public APIs minimal and clear? Are implementation details leaking into headers?
- **Coupling:** is the module tightly coupled to concrete implementations where it should use interfaces or templates?
- **Extensibility:** would adding a new feature require modifying existing code in fragile ways?
- **Code duplication:** is there duplicated logic that should be extracted into a shared utility or base class?

## Step 5: Generate `issues.md`

Create `modules/<module>/issues.md` with all issues found in Steps 3–4. Group similar issues into named blocks.

### Format

```markdown
# Issues — <module>

## Code Style Violations

### <Block Name> (e.g., "Naming Issues", "Include Order", "Missing Trailing Return Types")

#### Issue N: <short title>

- **Location:** `<file>:<line>` (or `<file>` if file-wide)
- **Description:** <why this is a problem>
- **Suggested fix:** <what to change>

---

## Potential Bugs

### <Block Name> (e.g., "Unchecked Optional Access", "Resource Leaks")

#### Issue N: <short title>

- **Location:** `<file>:<line>`
- **Description:** <explain the bug or unhandled case>
- **Suggested fix:** <concrete fix>

---

## Architecture Issues

### <Block Name> (e.g., "Tight Coupling", "Responsibility Violations")

#### Issue N: <short title>

- **Location:** `<file>` or `<class>` (if design-level)
- **Description:** <explain the design problem>
- **Suggested fix:** <proposed restructuring>
```

### Rules

- Number issues sequentially across the entire file (Issue 1, Issue 2, …)
- Every issue **must** have Location, Description, and Suggested fix fields
- If no issues are found in a category, omit that section entirely
- Group similar issues under a descriptive block name (e.g., "Missing `const` Qualifiers", "Dangling References")

## Step 6: Check Test Coverage

Read all test files in `modules/<module>/tests/` and the testing guide [test_generation.md](test_generation.md).

For every public function and class in the module, verify:

- **Happy path:** normal expected input produces correct output
- **Edge cases:** empty input, single element, maximum values, boundary values
- **Error paths:** invalid input, missing resources, thrown exceptions
- **State transitions:** if the class is stateful, all valid state transitions are exercised
- **Resource files:** if the code reads configs or files, tests cover well-formed, malformed, and missing files
- **Integration points:** if the module interacts with other modules, test the interface behavior (mocked or real)

## Step 7: Generate `tests_to_add.md`

Create `modules/<module>/tests_to_add.md` listing every missing test case. Group similar cases into named blocks.

### Format

```markdown
# Tests to Add — <module>

## <Block Name> (e.g., "Edge Cases for `parseConfig`", "Error Handling in `Session`")

### Case N: <short case title>

- **Function / Class:** `<name>`
- **Scenario:** <describe the situation that triggers this case>
- **Why it matters:** <explain when this case arises in practice and what could go wrong without a test>
- **Expected behavior:** <what the code should do>

---

## <Block Name>

### Case N: <short case title>

- **Function / Class:** `<name>`
- **Scenario:** <description>
- **Why it matters:** <explanation>
- **Expected behavior:** <expected result>
```

### Rules

- Number cases sequentially across the entire file (Case 1, Case 2, …)
- Every case **must** have Function/Class, Scenario, Why it matters, and Expected behavior fields
- Group related missing cases under a descriptive block name
- If all cases are covered, state that explicitly and omit the sections

## Review Checklist

Before finalizing, confirm:

1. All `.h`, `.inl`, `.cpp`, and `test_*.cpp` files in the module were read
2. Existing `issues.md` and `tests_to_add.md` were checked and fixed issues were excluded
3. Potential bugs and unhandled cases were analyzed
4. Architecture was evaluated
5. `modules/<module>/issues.md` was generated with grouped, numbered issues (only unfixed and new)
6. Test coverage was compared against [test_generation.md](test_generation.md) conventions
7. `modules/<module>/tests_to_add.md` was generated with grouped, numbered cases (only remaining and new)
