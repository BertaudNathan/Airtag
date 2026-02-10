<!--
SYNC IMPACT REPORT
==================
Version Change: N/A → 1.0.0 (Initial Constitution)
Date: 2026-02-10

Changes Summary:
- ✅ NEW: Established 5 core principles for Airtag ESP32 IoT Project
- ✅ NEW: Clean Code Architecture (modular .h/.cpp structure)
- ✅ NEW: Test-Driven Development (TDD) with Red-Green-Refactor
- ✅ NEW: Function Complexity Limits (30 lines max, 5 depth max)
- ✅ NEW: Embedded Best Practices (memory safety, resource management)
- ✅ NEW: Code Review & Quality Gates

Template Impacts:
- ✅ plan-template.md: Verified alignment with TDD and architecture principles
- ✅ spec-template.md: Verified alignment with functional decomposition requirements
- ✅ tasks-template.md: Verified alignment with TDD workflow and testing requirements
- ✅ All command files: No agent-specific references found

Follow-up TODOs:
- None - All placeholders resolved
-->

# Airtag ESP32 IoT Project Constitution

## Core Principles

### I. Clean Code Architecture
**MUST** maintain modular code structure:
- Headers (.h) declare interfaces; implementations (.cpp) define behavior
- Use classes for stateful components (sensors, controllers, services)
- Use structs for data containers (configurations, sensor readings, messages)
- Each module encapsulates a single responsibility
- Public APIs are minimal and well-documented

**Rationale**: Embedded systems require clear separation of concerns for testability, reusability, and maintenance. Header/implementation separation enables mock injection for unit testing without hardware dependencies.

### II. Test-Driven Development (NON-NEGOTIABLE)
**MUST** follow Red-Green-Refactor cycle:
1. **Red**: Write failing test that defines expected behavior
2. **Green**: Implement minimal code to pass the test
3. **Refactor**: Improve code quality while keeping tests green

- Tests written and reviewed **BEFORE** implementation begins
- No merge without passing tests
- Unit tests for logic, integration tests for hardware interaction
- Mock hardware interfaces in unit tests

**Rationale**: TDD ensures correctness, prevents regressions, and documents intended behavior. Critical for IoT devices where debugging deployed hardware is costly.

### III. Function Complexity Limits
**MUST** enforce strict complexity boundaries:
- Maximum **30 lines** per function (excluding blank lines and comments)
- Maximum **5 levels** of nested depth (loops, conditionals, blocks)
- Functions exceeding limits must be refactored into smaller units

**Rationale**: Embedded systems require predictable execution and stack usage. Short, shallow functions are easier to verify, test, and reason about memory/timing behavior.

### IV. Embedded Best Practices
**MUST** follow ESP32 Arduino framework standards:
- Explicit memory management - avoid dynamic allocation in loops
- Resource cleanup in destructors (RAII pattern where applicable)
- Const-correctness for read-only data
- Prefer stack allocation for small, short-lived objects
- Use hardware abstraction layers for testability
- Document assumptions about timing, memory, and hardware state

**Rationale**: Embedded constraints (limited RAM, no OS memory protection) require disciplined resource management. RAII and const-correctness prevent common embedded bugs.

### V. Code Review & Quality Gates
**MUST** verify before merge:
- All principles I-IV compliance checked
- Test coverage report included
- Static analysis passes (compiler warnings = errors)
- Code reviewed by at least one peer
- Hardware integration tested if applicable

**Rationale**: Quality gates catch principle violations early. Peer review shares knowledge and catches edge cases automated tools miss.

## Development Workflow

All development follows this sequence:
1. **Spec Phase**: Define feature requirements and acceptance criteria
2. **TDD Phase**: Write tests → Get approval → Verify tests fail
3. **Implementation Phase**: Make tests pass with minimal code
4. **Refactor Phase**: Apply principles I, III, IV to improve quality
5. **Review Phase**: Verify principle V gates before merge

Breaking changes to public APIs require:
- Version increment discussion
- Migration guide for dependent code
- Backward compatibility layer (if feasible)

## Governance

This constitution supersedes all other development practices and conventions. Any deviation requires explicit documentation of:
- Rationale for exception
- Impact assessment
- Mitigation plan
- Approval from project maintainer

**Amendment Process**:
- Proposed changes documented with rationale
- Impact on existing code assessed
- Migration plan created if needed
- Version incremented per semantic versioning:
  - **MAJOR**: Backward incompatible principle removal/redefinition
  - **MINOR**: New principle added or material expansion
  - **PATCH**: Clarifications, wording fixes, non-semantic refinements

All pull requests, code reviews, and architecture decisions must reference this constitution to verify compliance.

**Runtime Guidance**: For agent-assisted development, refer to `.specify/templates/agent-file-template.md` for operational patterns aligned with these principles.

**Version**: 1.0.0 | **Ratified**: 2026-02-10 | **Last Amended**: 2026-02-10
