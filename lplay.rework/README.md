# LangPlay / C%

LangPlay is a small C% source translator and runner. `lplay` translates C% into C++, builds it with an installed C++17 compiler, and runs it. `lplayprofile` invokes installed compilers for other languages. `toolplay` contains experimental setup and developer utilities.

C% is under active development. The [language reference](docs/language-reference.md) describes the current behavior; the [roadmap](docs/roadmap.md) separates planned ideas from supported syntax. Archived prototypes are kept under `archive/` and are not part of the active toolchain.

## Build

You need a C++17 compiler. On macOS with Xcode Command Line Tools:

```sh
mkdir -p build/bin
clang++ -std=c++17 -O2 src/lplay_driver.cpp -o build/bin/lplay
clang++ -std=c++17 -O2 src/tools/lplayprofile.cpp -o build/bin/lplayprofile
clang++ -std=c++17 -O2 src/tools/toolplay.cpp -o build/bin/toolplay
```

Run the hello-world example:

```sh
./build/bin/lplay examples/hello.cmod
```

## C% source files

Use `.cpr` or `.cmod` for C% source files. The compiler accepts any filename extension; the suffix does not select a language mode. Existing `.lpx` and `.cx` files also continue to work.

```sh
./build/bin/lplay -p examples/hello.cmod       # write generated C++
./build/bin/lplay -S examples/hello.cmod       # write assembly
./build/bin/lplay -o hello examples/hello.cmod # choose executable path
```

By default, `-p` writes `<source>.cpp`, `-S` writes `<source>.s`, and a normal build writes an executable named after the source file. Use `lplay --help` for the full options list.

## Other compiler profiles

Register a compiler that is already on your `PATH`, then select it by alias:

```sh
./build/bin/lplayprofile import java Java .java 'javac {input}'
./build/bin/lplayprofile import cpp C++ .cpp 'clang++ {input} -o {output}'
./build/bin/lplayprofile compile -java Main.java
./build/bin/lplayprofile compile -cpp main.cpp -o main
./build/bin/lplayprofile list
```

Profiles do not install or bundle compilers. Commands support `{input}` and `{output}` placeholders.

## Examples and project layout

- [Hello world, input, and file I/O](examples/hello.cmod)
- [Classes and access control](examples/oop_basics.cmod)
- [Language reference](docs/language-reference.md)
- [Future plan](docs/roadmap.md)

| Path | Contents |
| --- | --- |
| `src/` | Active compiler and tools |
| `examples/` | Small C% programs |
| `docs/` | Language notes and roadmap |
| `config/examples/` | Sample configuration files |
| `archive/prototypes/` | Early experiments and syntax drafts |
| `archive/legacy/` | Retired tools and older documentation |
| `build/` | Local generated output; ignored by Git |

`lplaynote` and `lplayxpr` have been retired. `toolplay` remains experimental while its long-term role is decided.

## Release note

No project license has been selected yet. Add a license before inviting others to reuse or redistribute the project. Experimental libraries and planned syntax should not be presented as stable features.
