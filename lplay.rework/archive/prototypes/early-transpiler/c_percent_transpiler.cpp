#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

class CPercentTranspiler {
private:
    bool has_rng = false;
    bool has_filesys = false;

    // Load custom .libm files.
    std::string process_custom_libraries(const std::string& source_code, const std::string& current_dir) {
        std::stringstream processed_source;
        std::stringstream ss(source_code);
        std::string line;

        // Find imports written with either quote style.
        std::regex lib_regex(R"(<\s*lib\s*["']([^"']+)["']\s*>)");
        std::smatch match;

        while (std::getline(ss, line)) {
            if (std::regex_search(line, match, lib_regex)) {
                std::string lib_path = match[1].str();
                fs::path full_path = fs::path(current_dir) / lib_path;

                std::ifstream lib_file(full_path);
                if (lib_file.is_open()) {
                    std::string lib_content((std::istreambuf_iterator<char>(lib_file)),
                                             std::istreambuf_iterator<char>());
                    lib_file.close();

                    processed_source << "// --- Included Library: " << lib_path << " ---\n";
                    // Process imports inside the library too.
                    processed_source << lib_content << "\n";
                    processed_source << "// --- End Library: " << lib_path << " ---\n";
                } else {
                    std::cerr << "\033[1;31m[lplay Warning]\033[0m Could not open library file: " << lib_path << "\n";
                }
            } else {
                processed_source << line << "\n";
            }
        }

        return processed_source.str();
    }

public:
    std::string transpile(const std::string& raw_source_code, const std::string& file_dir = ".") {
        // Expand custom library imports first.
        std::string source_code = process_custom_libraries(raw_source_code, file_dir);

        std::stringstream c_out;

        c_out << "// Auto-generated C++ code from C% Source\n";
        c_out << "#include <iostream>\n";
        c_out << "#include <string>\n";
        c_out << "#include <vector>\n";
        c_out << "#include <iomanip>\n";

        if (source_code.find("<rng>") != std::string::npos) {
            has_rng = true;
            c_out << "#include <random>\n";
        }
        if (source_code.find("<filesys>") != std::string::npos) {
            has_filesys = true;
            c_out << "#include <fstream>\n";
            c_out << "#include <cstdio>\n";
        }

        c_out << "\nconstexpr double O_PI = 3.14159265358979323846;\n";
        c_out << "constexpr double O_EULER = 2.71828182845904523536;\n\n";

        if (has_rng) {
            c_out << "struct CPercentRNG {\n";
            c_out << "    std::random_device rd;\n";
            c_out << "    std::mt19937 gen{rd()};\n";
            c_out << "    int range(int min, int max) {\n";
            c_out << "        std::uniform_int_distribution<int> distrib(min, max);\n";
            c_out << "        return distrib(gen);\n";
            c_out << "    }\n";
            c_out << "} rdnGen;\n\n";
        }

        c_out << "namespace clr {\n";
        c_out << "    const std::string reset = \"\\033[0m\";\n";
        c_out << "    const std::string bold = \"\\033[1m\";\n";
        c_out << "    const std::string red = \"\\033[31m\";\n";
        c_out << "    const std::string green = \"\\033[32m\";\n";
        c_out << "    const std::string yellow = \"\\033[33m\";\n";
        c_out << "    const std::string cyan = \"\\033[36m\";\n";
        c_out << "    inline void clear() { std::cout << \"\\033[2J\\033[1;1H\"; }\n";
        c_out << "}\n\n";

        std::stringstream ss(source_code);
        std::string line;
        bool user_has_main = false;

        while (std::getline(ss, line)) {
            // Remove imports handled by the C++ compiler.
            if (line.find("<iostream>") != std::string::npos ||
                line.find("<clrnsp>") != std::string::npos ||
                line.find("<logicgate>") != std::string::npos ||
                line.find("<filesys>") != std::string::npos ||
                line.find("<rng>") != std::string::npos) {
                continue;
            }

            if (line.find("main") != std::string::npos) user_has_main = true;

            // Translate the basic C% aliases.
            line = std::regex_replace(line, std::regex("\\bnum main\\b"), "int main");
            line = std::regex_replace(line, std::regex("\\bconstvar\\b"), "const");
            line = std::regex_replace(line, std::regex("\\bnum\\b"), "int");
            line = std::regex_replace(line, std::regex("\\bdec\\b"), "double");
            line = std::regex_replace(line, std::regex("\\bcstring\\b"), "std::string");
            line = std::regex_replace(line, std::regex("\\boutput\\b"), "std::cout");
            line = std::regex_replace(line, std::regex("\\binput\\b"), "std::cin");
            line = std::regex_replace(line, std::regex("\\bANDL\\b"), "&&");
            line = std::regex_replace(line, std::regex("\\bORL\\b"), "||");
            line = std::regex_replace(line, std::regex("\\bNOTL\\b"), "!");

            c_out << line << "\n";
        }

        if (!user_has_main) {
            c_out << "\nint main() { return 0; }\n";
        }

        return c_out.str();
    }
};
