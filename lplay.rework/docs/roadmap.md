# LangPlay / C% Future Plan

## Direction

Grow C% into a small, approachable language layer over C++ that keeps familiar C++ escape hatches while adding readable conveniences. New syntax should have one clear meaning, preserve existing programs, and produce useful compiler errors when a feature is misused.

## Command-line tool roles

- **`lplay`** is the C% interpreter/runner. It translates C% to C++, invokes a C++ compiler, and runs the result. It does not manage other language compilers.
- **`lplayprofile`** registers compiler commands already installed on the system and invokes them by alias, for example `lplayprofile compile -java Main.java`.
- **`toolplay`** remains an experimental toolbox for setup and developer utilities while its long-term purpose is decided.
- **`lplaynote`** and **`lplayxpr`** are retired from the active toolset. Their source is preserved under `archive/legacy/src/tools/`.

Compiler profiles are command templates, not bundled compiler distributions. A profile may use `{input}` and `{output}` placeholders; only the source path is appended automatically when `{input}` is absent. Compiler commands must be available on the user's `PATH`.

## Current baseline

- The language reference describes the current syntax aliases and identifies features that still need validation.
- Treat those features as **implemented but awaiting a focused validation pass**: verify that the examples in the reference compile and behave as documented before calling them stable.
- **Indexing is settled as zero-based** across C% and C++ containers. Do not add a setting to change the index base.

The October 2026 smoke pass built the compiler and companion utilities, ran the OOP and file-I/O examples, checked unit conversion and basic Chronos timing, and confirmed that `-p` emits C++ while `-S` emits assembler. This is not yet a complete regression suite; edge cases and reproducible checks remain future work.

## Roadmap

### Source file names and imports

- Recommend `.cpr` and `.cmod` for C% files. The current `lplay` accepts any regular input filename and does not select behavior from the extension; `.lpx` and `.cx` remain accepted too.
- Keep `<iostream>` as the concise C% spelling for a recognized standard library. Also accept the familiar `#include <iostream>` spelling. These forms are currently supported for recognized headers.
- Explore `import "my_lib.lmd";` for importing a custom library file. This is a proposal and is not implemented.
- Explore `from <iostream> import output;` and `from <iostream> bring output;` as selective-import spellings. Decide whether both are useful before implementing either.
- Define `import_when` before adding it. The current idea is conditional or lazy loading when declarations are used inside functions or branches, but the exact scope, dependency rules, and error behavior are not settled.
- Keep the language reference explicit about which forms are working and which remain proposals.

### 1. Stabilize the current language features

**Goal:** Make the existing units and Chronos behavior predictable before expanding the language surface.

- Compile the documented units and Chronos examples and record any mismatches.
- Specify conversion, rounding, overflow, dimension mismatch, timer, and calendar behavior.
- Add regression cases for supported syntax and error cases.
- Mark experimental features clearly in the documentation until their behavior is settled.

**Done when:** the documented examples work as written, failures give actionable diagnostics, and later changes can be checked against regression cases.

### 2. Add `std::double_vector` (first new feature)

**Goal:** Provide a simple two-dimensional, resizable collection. The initializer in the proposal has rows of different lengths, so the first version should support ragged rows.

Proposed starting point:

```cmod
<vector>

int main() {
    std::double_vector v{
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10}
    };

    output << v.at(0, 1); // first row, second item: 2; checked access
    output << v[1][2];    // second row, third item: 7
}
```

#### Syntax and behavior decisions

- Use zero-based row and column indices, matching C++.
- Support `v.at(row, column)` as the bounds-checked form. It should report which row or column is invalid and handle short ragged rows correctly.
- Support `v[row][column]` as the concise subscript form. The first subscript selects a row; the second selects an element in that row. Define and document its out-of-range behavior before release (prefer checked access if C% wants safe defaults; otherwise match C++ `operator[]` and reserve checks for `.at`).
- The proposed third form, `v[1[2]]`, is not a second way to index `v`: it tries to subscript the integer literal `1`. Leave it invalid and point users to `v[1][2]`.
- The proposed fourth form, `v.at(1(2))`, treats `1` as if it were a function. Leave it invalid; `at` takes two comma-separated arguments.
- Infer the element type from the initializer when possible; also consider a typed spelling such as `std::double_vector<int>`. Decide whether `double_vector` means “two-dimensional vector” or “vector of doubles” before freezing the name. A clearer future spelling could be `std::vector2d<T>`; keep the proposed name as an alias if backwards compatibility matters.
- Specify whether rows can be appended, resized, or replaced, and whether reads/writes return ordinary C++ references.

#### Implementation sequence

1. Write the syntax and semantics specification, including empty vectors, ragged rows, type inference, mutation, and bounds behavior.
2. Choose whether `<vector>` is a C% library import that lowers to C++ `std::vector` support or simply permits the familiar C++ header spelling.
3. Implement the runtime representation and transpiler lowering without changing ordinary C++ vector behavior.
4. Add examples and diagnostics to the language reference.
5. Add compiler-level regression cases for both valid forms, ragged rows, type inference, and invalid indices. Test them before marking the feature stable.

**Done when:** both supported access forms select the same element using the chosen indexing base, ragged rows work, errors are understandable, and the feature’s type/name behavior is documented.

### 3. Make units safer to extend

- Add explicit symbols to custom prefix declarations so two prefixes cannot silently collide.
- Add named physical dimensions (length, area, volume, time, and user-defined dimensions) rather than assuming every custom unit is length-like.
- Specify conversions, compound dimensions, and the result-unit setting with examples for addition, multiplication, and division.
- Keep automatic operation conversion opt-in or make its scope explicit, so one setting cannot unexpectedly affect unrelated expressions.

### 4. Grow Chronos from durations into calendar time

- Keep elapsed-time measurement monotonic and separate from wall-clock timestamps.
- Document fixed durations separately from calendar months and years.
- Add calendar date/time types and clear timezone and daylight-saving rules before adding more calendar arithmetic.
- Define whether clock callbacks drift, catch up, or skip missed ticks when work takes longer than the interval.

### 5. Build the settings library

**Goal:** Let a program or project choose a named settings profile, start from documented defaults, and override a small set of language options.

#### Cleaned-up draft syntax

```cmod
<settings>

std::settings active = std::tool_sets.part("default");

active.syntax.statement_separator = semicolon;
active.variables.declaration_style = classic_c;
active.cpp_code.enabled = false;
```

The profile defaults could be declared separately:

```cmod
std::settings.defaults {
    syntax.statement_separator = semicolon;
    variables.declaration_style = classic_c;
    cpp_code.enabled = false;
}
```

This expresses the proposal as ordinary nested settings with consistent names and values. `std::tool_sets.part("default")` selects a **named** profile instead of relying on an unexplained numeric `tn` index. If numeric profile selection is retained, its indices should also start at zero.

#### Proposed options

| Option | Clean spelling | Meaning |
| --- | --- | --- |
| Statement separator | `syntax.statement_separator = semicolon` | Select `;` or `,` between statements. This must change statement parsing only; commas inside calls, initializers, and declarations keep their normal meaning. |
| Declaration style | `variables.declaration_style = classic_c` | Select `classic_c`, `var`, `let`, or `implicit` declaration forms. |
| Raw C++ escape | `cpp_code.enabled = false` | Permit embedded C++ only when the project imports `<cpp_code>` and explicitly enables this option. |

The declaration styles map to the proposal like this:

```cmod
// classic_c: int x = 4;
// var:       var x = 4;
// let:       let x = 4;
// implicit:  x = 4;
```

`var`, `let`, and `implicit` need type-inference and scope rules. In particular, `implicit` must say whether a bare assignment can create a variable or only assign to an existing one. Prefer making it create a variable only in a clear declaration context, so misspelled names do not silently become new variables.

#### Design suggestions and decisions

- Zero-based indexing is a fixed language rule. Do not make it configurable; settings should not let different libraries disagree about what index `0` means.
- Prefer a named profile such as `"default"` over `part[tn]`; names survive profile reordering and are easier to read.
- Keep the C++ escape disabled by default. Importing `<cpp_code>` should make the feature available, while a separate explicit setting enables it. Do not use `c++` inside an identifier; `cpp_code` is valid and easier for the lexer.
- Decide whether settings are selected at compile time per project or changed at runtime. Syntax-changing options (separator and declaration style) should be fixed before parsing/transpilation, not mutable halfway through a program.
- Validate every option and reject unsupported values with a clear message. Declaration style should accept only the documented names.
- Begin with in-memory/default profiles. Add config-file loading and saving later, with a documented format, migration rules, and safe handling of malformed files.

#### Implementation sequence

1. Define profile selection, precedence (built-in defaults, project profile, local overrides), and when settings take effect.
2. Apply the settled zero-based indexing rule consistently to every container API.
3. Implement validation and the four declaration modes, starting with `classic_c` and `var` before `let` and `implicit`.
4. Add the parser-aware statement separator option only if it can be supported without changing commas in expressions or argument lists.
5. Add the opt-in `<cpp_code>` escape and diagnostics for disabled or missing imports.
6. Add tests and examples for defaults, overrides, profile lookup, each declaration style, zero-based container access, and rejected settings.

**Done when:** a program can select and override a named profile; invalid settings fail clearly; all syntax options apply consistently; and `<cpp_code>` requires both the import and explicit enablement.

### 6. Improve the compiler developer experience

- Report source filename, line, and column for transpiler errors.
- `-p` / `--transpile-only` now writes generated C++; `-S` now asks the selected C++ compiler to emit assembly. Verify both modes and document any compiler-specific behavior before calling them stable.
- Keep a small, documented compatibility policy for syntax changes.
- Add examples for each stable library and a single quick-start program that exercises C%, units, Chronos, and `double_vector`.

### 7. Add source metadata and faster builds

**Goal:** Make source/output naming predictable and reduce repeat build time without adding required boilerplate to every source file.

#### Cleaned-up proposal

```cmod
std::build.source = "file.cmod"; // optional when lplay already receives the path
std::build.output = "file";      // optional; defaults to the source filename stem
```

Treat these as build metadata rather than library imports. The `lplay` command already receives the input path, so source declaration should be optional; if both the command and source disagree, the command-line path should remain authoritative or the compiler should report a clear mismatch. For the output name, prefer an explicit `output` setting or the existing `-o` option. When neither is given, derive the name from the input path's stem (remove only the final extension, unlike a general string `strip`).

#### Compile-time improvement plan

- Measure transpilation and C++ compilation separately so optimization targets the part that is actually slow.
- Add a content-based build cache keyed by source contents, imported-file/dependency contents, transpiler version, compiler identity, and compiler flags. Reuse the output only when every input matches.
- Track dependencies for included C% libraries so a change to one invalidates affected builds while unrelated cached builds remain usable.
- Later, move reusable generated library support into separate compiled units or precompiled headers if measurements show that repeated library compilation is still costly.
- Keep a clean rebuild option and show when a cached artifact was reused; stale output must never be mistaken for a successful fresh build.

**Suggestion:** the next addition with the clearest user benefit is incremental build caching. `fname("file.cmod")` can name the input, but naming it alone does not reduce compiler work; caching unchanged inputs does.

## Suggested first milestone

Start with the validation pass for units/Chronos, then write the `double_vector` specification before touching the transpiler. Zero-based indexing is decided; the remaining design choices are the element type spelling and bounds behavior for subscript access. After that, specify the settings profile format and implement its compile-time options. For faster builds, measure current costs before adding a dependency-aware cache; source/output naming should stay optional because the compiler already knows the input path.

## Status tracking

Update this file as decisions are made. Move items to **Done** only after their documented examples and regression cases have been verified; until then, label them **In progress** or **Unverified**.
