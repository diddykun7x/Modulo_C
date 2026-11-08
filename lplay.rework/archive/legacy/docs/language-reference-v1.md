# C% Language Reference & Specification
C% (C-Percent) is a high-level, human-readable systems programming wrapper designed to streamline C++ syntax. It introduces cleaner keyword aliases, built-in macro helpers, modern variable abstractions, and terminal styling namespaces while maintaining native 1:1 C++ performance via transpilation.
Key Features
⚬ Readable Types & Logic: Replaces verbose keywords with clean, intuitive alternatives (num, dec, cstring, fact).
⚬ Expressive Logic Gates: Uses clear logical operator aliases (ANDL, ORL, NOTL, XORL).
⚬ Zero-Setup Output Streams: Pre-configured I/O stream aliases (output, input).
⚬ Built-in Standard Library Modules: Simple header includes for terminal color control (<clrnsp>), file handling (<filesys>), and random number generation (<rng>).
Data Types & Constants
Basic Types
C% Type	C++ Equivalent	Description
num	int	Standard integer
dec	double	Floating-point double-precision number
cstring	std::string	String literal/object
fact	bool	Boolean logic type
constvar	const	Constant variable modifier
Boolean Literals
⚬ True or T \rightarrow true
⚬ False or F \rightarrow false
Pre-defined Math Constants
⚬ #O_PI \rightarrow 3.14159265358979323846
⚬ #O_EULER \rightarrow 2.71828182845904523536
Operators & Identifiers
C% Syntax	C++ Transpilation	Description
output	std::cout	Console output stream
input	std::cin	Console input stream
ANDL	&&	Logical AND
ORL	`	
NOTL	!	Logical NOT
XORL	^	Bitwise/Logical XOR
@	&	Address-of operator
Standard Libraries
1. Terminal Colors (<clrnsp>)
Provides built-in ANSI terminal formatting under the clr:: namespace.
⚬ Text Colors: clr::red, clr::green, clr::yellow, clr::cyan, clr::reset
⚬ Text Styles: clr::bold, clr::dim, clr::italic, clr::underline
⚬ Background Colors: clr::bgRed, clr::bgBlack
⚬ Screen Utility: clr::clear() (clears terminal buffer)
2. Random Number Generation (<rng>)
Injects a seeded pseudo-random engine instance named rdnGen.
⚬ Usage: num roll = rdnGen.range(1, 6);
3. File System Helpers (<filesys>)
Provides shorthand syntax for file I/O operations.
⚬ +File("test.txt"); \rightarrow Creates a file
⚬ -File("test.txt"); \rightarrow Deletes a file
⚬ iFile("log.txt") << "entry"; \rightarrow Opens file stream in append mode (std::ofstream)
⚬ oFile("log.txt") \rightarrow Opens file stream for reading (std::ifstream)
Code Example
<iostream>
<clrnsp>
<rng>
<filesys>

num main() {
    clr::clear();
    output << clr::bold << clr::cyan << "=== C% Demo Program ===" << clr::reset << "\n";

    // Variables & Constants
    num age = 20;
    dec piValue = #O_PI;
    cstring name = "Alex";
    fact isActive = True;

    // Logic Gates & Conditions
    if (age >= 18 ANDL isActive) {
        output << clr::green << name << " is an active adult." << clr::reset << "\n";
    }

    // RNG Library Usage
    num diceRoll = rdnGen.range(1, 20);
    output << "D20 Roll: " << diceRoll << "\n";

    // File Operations
    +File("sample.txt");
    iFile("sample.txt") << "Logged roll: " << diceRoll << "\n";

    return 0;
}

# key-differences between C% and C++/any other language

**1. NO "#include", just the lib name directly**, this happened because of a coding mistake, my bad tho
**2. array[1] -> array[0]! Indexing starts at 1**, bcz 0-indexing is a pain in my fucking ass
**3. more binary** bools, bits, facts, all 3 are the binary variables, me personally i recommend the usage of bools, bits if you do more math
**4. simpler colors (colours)** do i gotta say anything
**5. #, @ tags** #for const numbers (#O_PI), @ as mem addresses
**6. Frequent updates that act change shi** 
