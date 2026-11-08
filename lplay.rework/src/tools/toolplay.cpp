#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <map>
#include <vector>
#include <regex>
#include <chrono>

#if __has_include(<filesystem>)
    #include <filesystem>
    namespace fs = std::filesystem;
#else
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#endif

std::string get_home() {
    const char* h = std::getenv("HOME");
    return h ? std::string(h) : ".";
}

fs::path find_project_root() {
    if (const char* configured = std::getenv("LPLAY_PROJECT_DIR")) {
        const fs::path root(configured);
        if (fs::exists(root / "src" / "lplay_driver.cpp")) return fs::absolute(root);
    }

    std::error_code error;
    fs::path current = fs::current_path(error);
    if (error) return {};
    for (fs::path candidate = current; !candidate.empty(); candidate = candidate.parent_path()) {
        if (fs::exists(candidate / "src" / "lplay_driver.cpp")) return candidate;
        const fs::path parent = candidate.parent_path();
        if (parent == candidate) break;
    }
    return {};
}

void print_header(const std::string& title) {
    std::cout << "\033[2J\033[1;1H";
    std::cout << "\033[1;36m====================================================\033[0m\n";
    std::cout << "\033[1;36m  🚀 TOOLPLAY — " << title << "\033[0m\n";
    std::cout << "\033[1;36m====================================================\033[0m\n\n";
}

// Set up the user's LangPlay directory.
void run_wizard() {
    print_header("LPLAY TOOL WIZARD");
    std::cout << "Welcome to the LangPlay Installation & Setup Wizard!\n\n";

    const fs::path lplay_dir = fs::path(get_home()) / ".lplay";
    std::error_code error;
    fs::create_directories(lplay_dir, error);
    if (error) {
        std::cerr << "Could not create " << lplay_dir << ": " << error.message() << "\n";
        return;
    }

    std::cout << "\033[1;32m[✓]\033[0m Workspace directories verified.\n";

    // Create default settings when none exist.
    const fs::path sets_path = lplay_dir / "toolsets.json";
    if (!fs::exists(sets_path)) {
        std::ofstream out(sets_path);
        out << "{\n  \"enable_colors\": true,\n  \"compiler_optimization_level\": \"-O2\"\n}\n";
        out.close();
        std::cout << "\033[1;32m[✓]\033[0m Created default toolsets.json.\n";
    }

    std::cout << "\nChecking core binaries...\n";
    std::system("which lplay > /dev/null 2>&1 || echo '  ! lplay missing from PATH'");
    std::system("which lplayprofile > /dev/null 2>&1 || echo '  ! lplayprofile missing from PATH'");

    std::cout << "\n\033[1;36mSetup Complete!\033[0m You are ready to use LangPlay.\n";
}

// Import or export compiler profiles.
void handle_profiles_io(const std::string& action, const std::string& filepath) {
    const fs::path config_path = fs::path(get_home()) / ".lplay" / "config.json";
    std::error_code error;
    fs::create_directories(config_path.parent_path(), error);
    if (error) {
        std::cerr << "[toolplay] Could not create " << config_path.parent_path()
                  << ": " << error.message() << "\n";
        return;
    }

    if (action == "export") {
        print_header("PROFILE EXPORT");
        fs::copy_file(config_path, filepath, fs::copy_options::overwrite_existing, error);
        if (!error) {
            std::cout << "\033[1;32m[Success]\033[0m Profiles exported to: " << filepath << "\n";
        } else {
            std::cout << "\033[1;31m[Error]\033[0m Failed to export profiles: " << error.message() << "\n";
        }
    } else if (action == "import") {
        print_header("PROFILE IMPORT");
        fs::copy_file(filepath, config_path, fs::copy_options::overwrite_existing, error);
        if (!error) {
            std::cout << "\033[1;32m[Success]\033[0m Profiles imported successfully!\n";
        } else {
            std::cout << "\033[1;31m[Error]\033[0m Failed to import profiles from: "
                      << filepath << ": " << error.message() << "\n";
        }
    }
}

// Manage tool settings and cleanup.
void manage_settings() {
    print_header("TOOL SETTINGS (toolsets.json)");
    const fs::path sets_path = fs::path(get_home()) / ".lplay" / "toolsets.json";

    std::ifstream in(sets_path);
    if (in.is_open()) {
        std::cout << "\033[1;33mCurrent Configuration:\033[0m\n\n";
        std::cout << in.rdbuf() << "\n";
    } else {
        std::cout << "No settings file found. Run 'toolplay wiz' to create one.\n";
    }
}

// Show installed extensions and themes.
void manage_exts() {
    print_header("EXTENSIONS & THEMES");
    std::cout << "\033[1;33mAvailable UI Color Schemes:\033[0m\n";
    std::cout << "  - Default Neon  (\\033[1;36m Cyan / Green \\033[0m)\n";
    std::cout << "  - Matrix Monochrome (\\033[1;32m Green Focus \\033[0m)\n";
    std::cout << "  - Sunset Amber (\\033[1;33m Yellow / Red \\033[0m)\n\n";

    std::cout << "\033[1;33mInstalled Language Modules:\033[0m\n";
    std::cout << "  • C% Transpiler Engine v2.0\n";
    std::cout << "  • Standard Library Injector (<rng>, <filesys>, <clrnsp>)\n";
}

// Developer utilities.
void handle_dev_tools(const std::string& action) {
    const fs::path project_root = find_project_root();
    if (project_root.empty()) {
        std::cerr << "[dev_tool] Project root not found. Run this command from the LangPlay checkout or set LPLAY_PROJECT_DIR.\n";
        return;
    }
    const std::string project_dir = project_root.string();

    if (action == "open_source_code") {
        print_header("DEV TOOL: OPEN SOURCE CODE");
        std::cout << "Opening project folder in VS Code...\n";
        std::string cmd = "code \"" + project_dir + "\" || open \"" + project_dir + "\"";
        std::system(cmd.c_str());
        return;
    }

    if (action == "ls_source_code") {
        print_header("DEV TOOL: LIST SOURCE CODE");
        std::cout << "\033[1;33mFiles in " << project_dir << ":\033[0m\n\n";
        std::string cmd = "ls -la \"" + project_dir + "\"";
        std::system(cmd.c_str());
        return;
    }

    if (action == "build_all") {
        print_header("DEV TOOL: BUILD ENTIRE SUITE");
        std::cout << "Building the suite into the project-local build/bin directory...\n\n";

        std::error_code fs_error;
        fs::create_directories(project_root / "build" / "bin", fs_error);
        if (fs_error) {
            std::cerr << "[dev_tool] Could not create build/bin: " << fs_error.message() << "\n";
            return;
        }
        
        std::string build_cmd = 
            "cd \"" + project_dir + "\" && "
            "clang++ -std=c++17 -O2 src/lplay_driver.cpp -o build/bin/lplay && "
            "clang++ -std=c++17 -O2 src/tools/lplayprofile.cpp -o build/bin/lplayprofile && "
            "clang++ -std=c++17 -O2 src/tools/toolplay.cpp -o build/bin/toolplay";

        if (std::system(build_cmd.c_str()) == 0) {
            std::cout << "\n\033[1;32m[✓] The three active tools built in build/bin.\033[0m\n";
        } else {
            std::cout << "\n\033[1;31m[!] Build failed during compilation.\033[0m\n";
        }
        return;
    }

    if (action == "backup") {
        print_header("DEV TOOL: CREATE BACKUP ARCHIVE");
        std::string backup_file = get_home() + "/lplay_rework_backup.tar.gz";
        std::string cmd = "tar -czf \"" + backup_file + "\" -C \"" + project_dir + "\" .";
        
        if (std::system(cmd.c_str()) == 0) {
            std::cout << "\033[1;32m[✓]\033[0m Backup saved to: \033[1m" << backup_file << "\033[0m\n";
        } else {
            std::cout << "\033[1;31m[!] Failed to create backup archive.\033[0m\n";
        }
        return;
    }

    std::cout << "\033[1;31m[dev_tool] Unknown action:\033[0m " << action << "\n";
    std::cout << "Available dev options: open_source_code | ls_source_code | build_all | backup\n";
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_header("MAINBOARD");
        std::cout << "Commands:\n";
        std::cout << "  toolplay wiz                            Run Installation Wizard\n";
        std::cout << "  toolplay profile export <file.json>    Export profiles\n";
        std::cout << "  toolplay profile import <file.json>    Import profiles\n";
        std::cout << "  toolplay tool_settings                  Manage JSON settings\n";
        std::cout << "  toolplay tool_exts                      View UI themes & extensions\n";
        std::cout << "  toolplay dev_tool <action>              Developer utilities\n";
        return 0;
    }

    std::string subcmd = argv[1];

    if (subcmd == "wiz") run_wizard();
    else if (subcmd == "profile" && argc >= 4) handle_profiles_io(argv[2], argv[3]);
    else if (subcmd == "tool_settings") manage_settings();
    else if (subcmd == "tool_exts") manage_exts();
    else if (subcmd == "dev_tool" && argc >= 3) handle_dev_tools(argv[2]);
    else {
        std::cout << "\033[1;31m[toolplay] Unknown subcommand:\033[0m " << subcmd << "\n";
        return 1;
    }
    return 0;
}
