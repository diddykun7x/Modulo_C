# C% language notes

C% is a small source-to-source layer over C++. The current compiler performs textual translations and then delegates compilation to a C++17 compiler. Ordinary C++ syntax remains available, including classes, access control, functions, and standard-library types.

These notes describe the current compiler where possible. Features marked **experimental** need more validation before they should be treated as stable language behavior.

## Keyword aliases

| C% spelling | C++ spelling |
| --- | --- |
| `num` | `int` |
| `dec` | `double` |
| `cstring` | `std::string` |
| `fact` | `bool` |
| `constvar` | `const` |
| `True`, `T` | `true` |
| `False`, `F` | `false` |
| `output` | `std::cout` |
| `input` | `std::cin` |
| `ANDL`, `ORL`, `NOTL`, `XORL` | `&&`, `||`, `!`, `^` |
| `@value` | `&value` |
| `#O_PI`, `#O_EULER` | `O_PI`, `O_EULER` |

For example:

```cmod
<iostream>

num main() {
    cstring name = "Ada";
    output << "Hello, " << name << "!\n";
    return 0;
}
```

## C++ classes

Classes use normal C++ rules. A `class` starts with private access by default; members become public after `public:`. A private helper can be called from a public method on the same class:

```cmod
class student {
    cstring build_fullname(cstring fn, cstring mn, cstring ln) {
        return fn + " " + mn + " " + ln;
    }

public:
    cstring fn;
    cstring mn;
    cstring ln;

    cstring full_name() {
        return build_fullname(fn, mn, ln);
    }
};
```

`build_fullname` is private. Code outside the class should call `full_name()` instead.

## Standard library headers and input

The current compiler recognizes these standard-header forms:

```cmod
<iostream>
<fstream>
#include <iostream>
```

The angle-bracket C% form and the C++ `#include` form are accepted for recognized headers. C% built-in imports currently include `<iostream>`, `<fstream>`, `<clrnsp>`, `<logicgate>`, `<filesys>`, `<memManagement>`, `<rng>`, `<units>`, and `<chronos>`. They enable C% translator support; they are not drop-in replacements for every feature in the corresponding C++ libraries. The translator also emits several common C++ headers for generated programs.

For `<fstream>`, `getline(name)` reads a full line from standard input, and `getline(stream, name)` maps to `std::getline(stream, name)`. The standard C++ `std::getline` spelling remains available too.

## Custom library imports (planned)

These forms are proposals for a future release. The current compiler does not load `.lmd` files or implement selective imports.

```cmod
import "my_lib.lmd";
import_when "my_lib.lmd";

<bring>
from <iostream> import output;

from <iostream> bring output;
include iostream;
```

The design goal is to let `import` load a custom library, and let `from ... import/bring ...` expose selected names. `include iostream;` is a proposed shorthand for a standard library. The exact meaning of `import_when` still needs a clear rule: the proposal is to load a library only when one of its declarations is used, including from a function or conditional block. These rules should be resolved before implementation. The current built-in header forms remain the supported way to enable standard C% libraries.

## Indexing

**C% uses zero-based indexing**, matching C++: the first element is at index `0`. This is a fixed language rule, not a project setting. Arrays, `std::vector`, and future C% containers must use the same base. Older notes proposing one-based indexing are obsolete.

## Experimental libraries

Measurement units and Chronos time helpers are implemented in the current compiler, but are still awaiting a focused validation pass. Their detailed proposals and open behavior questions are tracked in the [roadmap](roadmap.md). Avoid relying on edge cases until that work is complete.

The proposed `std::double_vector`, settings profiles, raylib wrapper, and replacement/interpreter syntax are not part of the stable language yet. Some early drafts are preserved under `archive/prototypes/`.
