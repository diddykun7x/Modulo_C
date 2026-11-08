# LangPlay Ecosystem (C% Toolchain)
Welcome to the LangPlay Polyglot Ecosystem—a lightweight, custom terminal toolchain designed to transpile C% source code, manage compiler profiles, run sandboxed tests in RAM, and organize project notes directly from your command line.

Toolchain Architecture
The suite consists of five modular executables:
⚬ lplay: Core driver and C% to C++ transpiler engine.
⚬ lplayprofile: Standalone manager for external compiler profiles (config.json).
⚬ lplayxpr: Isolated RAM sandbox (/tmp/lplayxpr) for instant code experiments.
⚬ lplaynote: Integrated terminal cheat sheet and scratchpad (notes.json).
⚬ toolplay: Central interactive TUI dashboard and system controller.
## Install the `lplay` CLI (macOS)

Build the driver into your personal command directory:

```sh
mkdir -p "$HOME/bin"
clang++ -std=c++17 -O2 v1.0/lplay_driver.cpp -o "$HOME/bin/lplay"
```

Add `~/bin` to `PATH` once in `~/.zshrc`:

```sh
export PATH="$HOME/bin:$PATH"
```

Then start a new terminal (or run `source ~/.zshrc`) and check the command:

```sh
lplay --help
lplay --version
```

LangPlay source files can use `.lpx`, `.cmod`, or `.cx`; all three extensions are accepted by the same compiler. `.cmod` remains supported for existing projects. For example, `lplay <file.lpx>` writes an executable named after the source file. Use `-o <path>` to choose another output, `--transpile-only -o <path.cpp>` to inspect generated C++, or `--compiler <command>` to select a C++17 compiler. By default it uses `$CXX` when set, otherwise `clang++`.

The other suite programs can be built separately from their source files; this driver install only installs `lplay`.


Quick Reference & Commands
1. lplay — C% Transpiler Core
Transpiles `.lpx`, `.cmod`, and `.cx` files into native C++17 binaries using Clang.
⚬ Compile a program: lplay main.lpx -o main
⚬ Existing `.cmod` programs still work: lplay main.cmod -o main
⚬ `.cx` programs work too: lplay main.cx -o main
⚬ Show help: lplay --help
⚬ Check version: lplay --version

2. lplayprofile — Profile Manager
Save and execute custom build targets stored in ~/.lplay/config.json.
⚬ List profiles: lplayprofile list
⚬ Add a profile: lplayprofile add java "Java Profile" "javac.jar" "java -jar ~/.lplay/compilers/{compiler_file} {input} -o {output}"
⚬ Run via profile: lplayprofile run java App.java -o App.jar

3. lplayxpr — RAM Sandbox Engine
Test isolated code directly inside a RAM-backed temporary directory (/tmp/lplayxpr).
⚬ Create RAM file: lplayxpr +file test.cmod
⚬ Compile & run in RAM: lplayxpr comp test.cmod
⚬ Save RAM file to disk: lplayxpr promote test.cmod ~/my_project/main.cmod
⚬ Wipe RAM workspace: lplayxpr purge

4. lplaynote — Built-in Cheat Sheet
Store syntax references and snippets in ~/.lplay/notes.json.
⚬ Get a note: lplaynote get rng
⚬ Add/update a note: lplaynote add custom_tag "Your note text here"
⚬ List saved tags: lplaynote list

5. toolplay — Interactive Control Panel
Manage the entire suite through dedicated view modules or an interactive TUI.
toolplay mainboard        # Interactive Dashboard
toolplay extview          # Installed Profiles & Syntax Extensions
toolplay filesys          # Directory Inspector (~/.lplay & /tmp)
toolplay debug            # Dependency Diagnostics
toolplay tool_settings    # Edit toolsets.json settings
toolplay tool_managebloat # Clear residual cache files
toolplay dev_tool build_all # Recompile all suite binaries

C% Language Cheat Sheet
Standard Library
Feature
Transpiled Equivalent

output << input
std::cout << std::cin

clr::red, clr::clear()
Terminal ANSI styling

ANDL, ORL, NOTL, XORL (&&, ||, ! work in if statements tho)
&&, ||, !, ^
Primitive Types
num, dec, cstring, fact (also C++ variable types should work)
int, double, std::string, bool
Math Constants
#O_PI, #O_EULER
3.14159..., 2.71828...
Pointer Symbol
@var
&var


binary variable types explained:
Bool = same as in C++
bit = 0 || 1
fact = True (or T) || False (or F)



# Most important, what is lplay?
lplay is NOT just a compiler for the C% (modulo c) language, it is split in:
**Toolplay** -Tools
**lplaynote** -Notes
**lplayprofile** -Profiles, i tried to make it save compilers so you could compile using lplay idk if that works mb if it doesent tho
**lplayxpr** -experimental files
**lplaycomp** -C% Compiler (or transpiler whatever), more on C% in the ANOTHER_README.MD file

## Units and Chronos (C% v1 implementation)

These built-in facilities are enabled by including `<units>` or `<chronos>` in a `.cmod` file. The transpiler emits the runtime support directly into its generated C++, so programs do not need an extra runtime library.

### Measurement units

```cmod
<units>

std::measure_unit<std::default_scale> meter(m);
auto short_distance = 10dm;
auto long_distance = 10dam;
auto total = short_distance + long_distance; // canonical result: 101m
output << total << "\n";
output << total.convert(into:dm) << "\n"; // 1010dm
```

The initial built-in ladder supports `mm`, `cm`, `dm`, `m`, `dam`, `hm`, and `km`. Values are stored in the base unit for their dimension; addition, subtraction, and comparisons therefore work across prefixes and reject incompatible dimensions at C++ compile time. `std::conversion_onOperation.result(into:dam);` changes the displayed result unit for matching dimensions. Without that setting, results display in the base unit.

Multiplication and division combine dimension powers. `10m * 10m` is an area, and `2m * 3m * 4m` is a volume. `std::measure_unit<std::square_scale> square_meter(m);` and `std::measure_unit<std::cubic_scale> cubic_meter(m);` name the generated `am` and `Vm` forms. Prefix powers apply to the whole unit: `1adm` is `0.01am`; `1Vdm` is `0.001Vm`.

Custom scale declarations support a decimal `multiple_smultiple.range` and prefix entries. Prefix spellings are inferred from names (for example `centi` becomes `c`, `deca` becomes `da`).

```cmod
std::scale decimal_scale {
    decimal_scale.data =
    multiple_smultiple.range = 10
    scale.done;
    std::scale_character_set : {
        M. blank;
        -10. "deci";
        -100 "centi";
        +10. "deca";
        +100 "hecto";
        +1000 "kilo";
    }
}
std::measure_unit<decimal_scale> unit(u);
auto small = 10cu;
auto large = 3dau;
```

The transpiler derives prefix codes from those names (`centi` becomes `c`, `deca` becomes `da`). A custom unit can be declared as `std::measure_unit<custom_scale> unit(u);`; its suffix and prefixed forms use that scale's factor table. The current unit model treats declared units as length-like dimensions; separate custom physical dimensions are not yet declared by this syntax. Prefix codes are inferred, so names with colliding initials need an explicit-symbol syntax in a future revision.

### Chronos

```cmod
<chronos>
chronos::time_count.use;
chronos::timer.import;
chronos::timer workout_plan = 50s; // timer starts at construction

// ... work ...
output << workout_plan.elapsed() << " elapsed\n";
if (workout_plan.expired()) output << "done\n";

chronos::clock.import;
chronos::clock::for_ticks(5, 1s, [&](std::size_t tick) {
    output << "tick " << tick << "\n";
});
output << chronos::time_count.sum() << " total\n";
```

Seconds, milliseconds, minutes, hours, and 24-hour days are fixed durations. `sec`, `second(s)`, `minute(s)`, `hour(s)`, and `day(s)` are accepted aliases. `time_count.use;` starts a monotonic elapsed-time counter; `sum()` returns its accumulated duration, `sum(duration)` adds a duration, `stop;` pauses it, and `reset;` clears it. A timer offers `elapsed()`, `remaining()`, `expired()`, `restart()`, and blocking `wait()`. `clock::for_ticks(count, interval, callback)` invokes the callback with each zero-based tick index.

Months, years, and decades are calendar periods, not fixed durations. Use them with a calendar timestamp: `chronos::add_to(std::time(nullptr), chronos::months(1))`. Calendar addition uses local time and clamps dates such as January 31 to the last day of the target month.
