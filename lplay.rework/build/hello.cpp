// Auto-generated C++ code from C% Source
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>

// --- C% Math Constants ---
constexpr double O_PI = 3.14159265358979323846;
constexpr double O_EULER = 2.71828182845904523536;

// --- C% <clrnsp> Library ---
namespace clr {
    const std::string reset = "\033[0m";
    const std::string bold = "\033[1m";
    const std::string dim = "\033[2m";
    const std::string italic = "\033[3m";
    const std::string underline = "\033[4m";
    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string yellow = "\033[33m";
    const std::string cyan = "\033[36m";
    const std::string bgRed = "\033[41m";
    const std::string bgBlack = "\033[40m";
    inline void clear() { std::cout << "\033[2J\033[1;1H"; }
}

#line 1 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
// Build the CLI first, then run: ./build/lplay examples/hello.cmod
#line 2 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
#line 3 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
#line 4 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
#line 5 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"

#line 6 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
int main() {
#line 7 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::string name;
#line 8 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::cout << "What is your name? ";
#line 9 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::getline(std::cin, name); // reads spaces too
#line 10 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"

#line 11 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::cout << clr::cyan << "Hello, " << name << "!" << clr::reset << "\n";
#line 12 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"

#line 13 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    // Save the greeting, then open the file again and read it back.
#line 14 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::ofstream greeting_file("greeting.txt");
#line 15 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    if (!greeting_file) {
#line 16 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
        std::cout << "Could not create greeting.txt\n";
#line 17 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
        return 1;
#line 18 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    }
#line 19 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    greeting_file << "Hello, " << name << "!\n";
#line 20 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    greeting_file.close();
#line 21 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"

#line 22 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::ifstream saved_greeting("greeting.txt");
#line 23 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::string greeting_from_file;
#line 24 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::getline(saved_greeting, greeting_from_file);
#line 25 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::cout << "Read back from greeting.txt: " << greeting_from_file << "\n";
#line 26 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"

#line 27 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    int count = 3;
#line 28 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    std::cout << "Counting to " << count << ":\n";
#line 29 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    for (int i = 1; i <= count; ++i) {
#line 30 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
        std::cout << i << "\n";
#line 31 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    }
#line 32 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"

#line 33 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
    return 0;
#line 34 "/Users/radu-stefan/Desktop/Folder1./lplay.rework/examples/hello.cmod"
}
