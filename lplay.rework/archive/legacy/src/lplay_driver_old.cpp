#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <cstdlib>
#include <cstdio>

// ========================================================================
//  C% TRANSPILER ENGINE
// ========================================================================
class CPercentTranspiler {
private:
    bool has_rng = false;
    bool has_filesys = false;

public:
    std::string transpile(const std::string& source_code) {
        std::stringstream c_out;

        // 1. Core Mandatory C++ Headers
        c_out << "// Auto-generated C++ code from C% Source\n";
        c_out << "#include <iostream>\n";
        c_out << "#include <string>\n";
        c_out << "#include <vector>\n";
        c_out << "#include <iomanip>\n";

        // 2. Scan Header Declarations
        if (source_code.find("<rng>") != std::string::npos) {
            has_rng = true;
            c_out << "#include <random>\n";
        }
        if (source_code.find("<filesys>") != std::string::npos) {
            has_filesys = true;
            c_out << "#include <fstream>\n";
            c_out << "#include <cstdio>\n";
        }

        // 3. Built-in Math Constants
        c_out << "\n// --- C% Math Constants ---\n";
        c_out << "constexpr double O_PI = 3.14159265358979323846;\n";
        c_out << "constexpr double O_EULER = 2.71828182845904523536;\n";

        // 4. Custom RNG Engine Implementation
        if (has_rng) {
            c_out << "\n// --- C% <rng> Library ---\n";
            c_out << "struct CPercentRNG {\n";
            c_out << "    std::random_device rd;\n";
            c_out << "    std::mt19937 gen{rd()};\n";
            c_out << "    int range(int min, int max) {\n";
            c_out << "        std::uniform_int_distribution<int> distrib(min, max);\n";
            c_out << "        return distrib(gen);\n";
            c_out << "    }\n";
            c_out << "} rdnGen;\n";
        }

        // 5. Custom Terminal Color Namespace Engine
        c_out << "\n// --- C% <clrnsp> Library ---\n";
        c_out << "namespace clr {\n";
        c_out << "    const std::string reset = \"\\033[0m\";\n";
        c_out << "    const std::string bold = \"\\033[1m\";\n";
        c_out << "    const std::string dim = \"\\033[2m\";\n";
        c_out << "    const std::string italic = \"\\033[3m\";\n";
        c_out << "    const std::string underline = \"\\033[4m\";\n";
        c_out << "    const std::string red = \"\\033[31m\";\n";
        c_out << "    const std::string green = \"\\033[32m\";\n";
        c_out << "    const std::string yellow = \"\\033[33m\";\n";
        c_out << "    const std::string cyan = \"\\033[36m\";\n";
        c_out << "    const std::string bgRed = \"\\033[41m\";\n";
        c_out << "    const std::string bgBlack = \"\\033[40m\";\n";
        c_out << "    inline void clear() { std::cout << \"\\033[2J\\033[1;1H\"; }\n";
        c_out << "}\n\n";

        // 6. Custom Filesystem Helper Implementations
        if (has_filesys) {
            c_out << "// --- C% <filesys> Library ---\n";
            c_out << "inline void createFile(const std::string& name) { std::ofstream f(name); }\n";
            c_out << "inline void deleteFile(const std::string& name) { std::remove(name.c_str()); }\n";
            c_out << "#define iFile(name) std::ofstream(name, std::ios::app)\n";
            c_out << "#define oFile(name) std::ifstream(name)\n";
            c_out << "#define +File(name) createFile(name)\n";
            c_out << "#define -File(name) deleteFile(name)\n\n";
        }

        // 7. Process Source Code Line by Line
        std::stringstream ss(source_code);
        std::string line;
        bool user_has_main = false;

        while (std::getline(ss, line)) {
            // Strip out header imports
            if (line.find("<iostream>") != std::string::npos ||
                line.find("<clrnsp>") != std::string::npos ||
                line.find("<logicgate>") != std::string::npos ||
                line.find("<filesys>") != std::string::npos ||
                line.find("<memManagement>") != std::string::npos ||
                line.find("<rng>") != std::string::npos) {
                continue;
            }

            // Detect if main is defined in the source
            if (line.find("main") != std::string::npos) {
                user_has_main = true;
            }

            // Function Signatures & Types
            line = std::regex_replace(line, std::regex("\\bnum main\\b"), "int main");
            line = std::regex_replace(line, std::regex("\\bconstvar\\b"), "const");
            line = std::regex_replace(line, std::regex("\\bnum\\b"), "int");
            line = std::regex_replace(line, std::regex("\\bdec\\b"), "double");
            line = std::regex_replace(line, std::regex("\\bcstring\\b"), "std::string");

            // Input / Output Streams
            line = std::regex_replace(line, std::regex("\\boutput\\b"), "std::cout");
            line = std::regex_replace(line, std::regex("\\binput\\b"), "std::cin");

            // Logic Gates
            line = std::regex_replace(line, std::regex("\\bANDL\\b"), "&&");
            line = std::regex_replace(line, std::regex("\\bORL\\b"), "||");
            line = std::regex_replace(line, std::regex("\\bNOTL\\b"), "!");
            line = std::regex_replace(line, std::regex("\\bXORL\\b"), "^");

            // Boolean Literals
            line = std::regex_replace(line, std::regex("\\bfact\\b"), "bool");
            line = std::regex_replace(line, std::regex("\\bTrue\\b"), "true");
            line = std::regex_replace(line, std::regex("\\bFalse\\b"), "false");
            line = std::regex_replace(line, std::regex("\\bT\\b"), "true");
            line = std::regex_replace(line, std::regex("\\bF\\b"), "false");

            // Constants & Address Symbols
            line = std::regex_replace(line, std::regex("#O_PI"), "O_PI");
            line = std::regex_replace(line, std::regex("#O_EULER"), "O_EULER");
            line = std::regex_replace(line, std::regex("@"), "&");

            c_out << line << "\n";
        }

        // 8. Default main injection if file is empty or lacks main()
        if (!user_has_main) {
            c_out << "\nint main() { return 0; }\n";
        }

        return c_out.str();
    }
};

// ========================================================================
// MAIN DRIVER
// ========================================================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "\033[1;36m=== LangPlay C% Compiler Driver (lplay) ===\033[0m\n";
        std::cout << "Usage: lplay <file.lpx | file.cmod | file.cx> [-o output_executable]\n";
        return 1;
    }

    std::string input_filename = "";
    std::string output_filename = "a.out";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_filename = argv[i + 1];
            i++;
        } else if (arg == "-v") {
            std::cout << "lplay C% Compiler v1.0 (Release)\n";
            return 0;
        } else if (input_filename.empty()) {
            input_filename = arg;
        }
    }

    if (input_filename.empty()) {
        std::cerr << "\033[1;31mError:\033[0m No input file provided.\n";
        return 1;
    }

    std::ifstream input_file(input_filename);
    if (!input_file.is_open()) {
        std::cerr << "\033[1;31mError:\033[0m Could not open file '" << input_filename << "'\n";
        return 1;
    }

    std::cout << "\033[1;32m[lplay]\033[0m Reading " << input_filename << "...\n";

    std::string source((std::istreambuf_iterator<char>(input_file)),
                        std::istreambuf_iterator<char>());
    input_file.close();

    // Transpile C% to C++
    CPercentTranspiler transpiler;
    std::string cpp_code = transpiler.transpile(source);

    // Save temporary cache
    std::ofstream temp_cpp("_build_cache.cpp");
    temp_cpp << cpp_code;
    temp_cpp.close();

    std::cout << "\033[1;33m[lplay]\033[0m Transpiling C% to C++ -> Compiling binary...\n";

    // Run backend compiler
    std::string compile_cmd = "clang++ -std=c++17 _build_cache.cpp -o " + output_filename;
    int result = std::system(compile_cmd.c_str());

    // Clean up temporary C++ cache file
    std::remove("_build_cache.cpp");

    if (result == 0) {
        std::cout << "\033[1;32m[lplay] ✅ 🤯 SUCCESS!033[0m Executable created: \033[1m" << output_filename << "\033[0m\n";
    } else {
        std::cerr << "\033[1;31m[lplay] ❌ 🤯 BUILD FAILED!\033[0m Check your C% syntax above.\n";
    }

    return 0;
}
