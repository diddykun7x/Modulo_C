#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>

namespace fs = std::filesystem;

struct CompilerProfile {
    std::string name;
    std::string extension;
    std::string command;
};

std::string home_directory() {
    const char* home = std::getenv("HOME");
    return home ? home : ".";
}

std::string json_escape(const std::string& value) {
    std::string out;
    for (char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

bool parse_json_string(const std::string& text, size_t& pos, std::string& value) {
    if (pos >= text.size() || text[pos] != '"') return false;
    ++pos;
    value.clear();
    while (pos < text.size()) {
        char c = text[pos++];
        if (c == '"') return true;
        if (c != '\\') {
            value += c;
            continue;
        }
        if (pos >= text.size()) return false;
        c = text[pos++];
        switch (c) {
            case '"': value += '"'; break;
            case '\\': value += '\\'; break;
            case '/': value += '/'; break;
            case 'n': value += '\n'; break;
            case 'r': value += '\r'; break;
            case 't': value += '\t'; break;
            case 'b': value += '\b'; break;
            case 'f': value += '\f'; break;
            default: return false;
        }
    }
    return false;
}

size_t json_object_end(const std::string& text, size_t start) {
    if (start >= text.size() || text[start] != '{') return std::string::npos;
    size_t depth = 0;
    bool in_string = false;
    bool escaped = false;
    for (size_t i = start; i < text.size(); ++i) {
        const char c = text[i];
        if (in_string) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') in_string = false;
        } else if (c == '"') {
            in_string = true;
        } else if (c == '{') {
            ++depth;
        } else if (c == '}' && depth > 0 && --depth == 0) {
            return i;
        }
    }
    return std::string::npos;
}

void skip_space(const std::string& text, size_t& pos) {
    while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\n' ||
           text[pos] == '\r' || text[pos] == '\t')) ++pos;
}

std::string json_field(const std::string& object, const std::string& field) {
    size_t pos = object.find('{');
    if (pos == std::string::npos) return {};
    ++pos;
    while (pos < object.size()) {
        skip_space(object, pos);
        if (pos >= object.size() || object[pos] == '}') break;
        std::string key;
        if (!parse_json_string(object, pos, key)) return {};
        skip_space(object, pos);
        if (pos >= object.size() || object[pos++] != ':') return {};
        skip_space(object, pos);
        if (pos < object.size() && object[pos] == '"') {
            std::string value;
            if (!parse_json_string(object, pos, value)) return {};
            if (key == field) return value;
        } else {
            while (pos < object.size() && object[pos] != ',' && object[pos] != '}') ++pos;
        }
        skip_space(object, pos);
        if (pos < object.size() && object[pos] == ',') ++pos;
    }
    return {};
}

class ProfileStore {
public:
    explicit ProfileStore(fs::path path) : path_(std::move(path)) {}

    std::map<std::string, CompilerProfile> load() const {
        std::map<std::string, CompilerProfile> profiles;
        std::ifstream in(path_);
        if (!in) return profiles;
        const std::string text((std::istreambuf_iterator<char>(in)), {});
        const size_t key_pos = text.find("\"profiles\"");
        if (key_pos == std::string::npos) return profiles;
        size_t pos = text.find('{', key_pos);
        if (pos == std::string::npos) return profiles;
        const size_t end = json_object_end(text, pos);
        if (end == std::string::npos) return profiles;
        ++pos;

        while (pos < end) {
            skip_space(text, pos);
            if (pos >= end) break;
            if (text[pos] == ',') {
                ++pos;
                continue;
            }

            std::string alias;
            if (!parse_json_string(text, pos, alias)) break;
            skip_space(text, pos);
            if (pos >= end || text[pos++] != ':') break;
            skip_space(text, pos);

            const size_t profile_end = json_object_end(text, pos);
            if (profile_end == std::string::npos || profile_end > end) break;
            const std::string object = text.substr(pos, profile_end - pos + 1);
            CompilerProfile profile;
            profile.name = json_field(object, "name");
            profile.extension = json_field(object, "extension");
            profile.command = json_field(object, "command");
            // Read profiles created by the older profile manager.
            if (profile.command.empty()) profile.command = json_field(object, "run_cmd");
            if (!profile.command.empty() && profile.command != "internal") profiles[alias] = profile;
            pos = profile_end + 1;
        }
        return profiles;
    }

    bool save(const std::map<std::string, CompilerProfile>& profiles) const {
        std::error_code error;
        fs::create_directories(path_.parent_path(), error);
        if (error) {
            std::cerr << "lplayprofile: cannot create " << path_.parent_path()
                      << ": " << error.message() << "\n";
            return false;
        }

        std::ofstream out(path_, std::ios::trunc);
        if (!out) {
            std::cerr << "lplayprofile: cannot write to " << path_ << "\n";
            return false;
        }
        out << "{\n  \"profiles\": {\n";
        size_t index = 0;
        for (const auto& [alias, profile] : profiles) {
            out << "    \"" << json_escape(alias) << "\": {\n"
                << "      \"name\": \"" << json_escape(profile.name) << "\",\n"
                << "      \"extension\": \"" << json_escape(profile.extension) << "\",\n"
                << "      \"command\": \"" << json_escape(profile.command) << "\"\n"
                << "    }" << (++index < profiles.size() ? "," : "") << "\n";
        }
        out << "  }\n}\n";
        return static_cast<bool>(out);
    }

private:
    fs::path path_;
};

std::string shell_quote(const std::string& value) {
    std::string quoted = "'";
    for (char c : value) {
        if (c == '\'') quoted += "'\\''";
        else quoted += c;
    }
    quoted += "'";
    return quoted;
}

void replace_all(std::string& text, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
}

void usage(std::ostream& out) {
    out << "lplayprofile — compiler profile manager\n"
        << "Usage:\n"
        << "  lplayprofile import <alias> <name> <extension|*> \"<command-template>\"\n"
        << "  lplayprofile compile -<alias> <source-file> [-o output]\n"
        << "  lplayprofile list\n"
        << "  lplayprofile remove <alias>\n\n"
        << "Templates support {input} and {output}. Compilers are called from PATH.\n";
}

int main(int argc, char* argv[]) {
    ProfileStore store(fs::path(home_directory()) / ".lplay" / "config.json");
    if (argc < 2 || std::string(argv[1]) == "help" ||
        std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
        usage(std::cout);
        return 0;
    }

    const std::string action = argv[1];
    auto profiles = store.load();

    if (action == "list") {
        if (profiles.empty()) {
            std::cout << "No compiler profiles. Add one with 'lplayprofile import'.\n";
            return 0;
        }
        std::cout << "Compiler profiles:\n";
        for (const auto& [alias, profile] : profiles) {
            std::cout << "  -" << alias << " (" << profile.name << ")";
            if (!profile.extension.empty() && profile.extension != "*")
                std::cout << " [" << profile.extension << "]";
            std::cout << "\n    " << profile.command << "\n";
        }
        return 0;
    }

    if (action == "import") {
        if (argc != 6) {
            usage(std::cerr);
            return 2;
        }
        const std::string alias = argv[2];
        if (alias.empty() || alias.front() == '-') {
            std::cerr << "lplayprofile: use an alias without the leading dash, e.g. java\n";
            return 2;
        }
        profiles[alias] = CompilerProfile{argv[3], argv[4], argv[5]};
        if (!store.save(profiles)) return 1;
        std::cout << "lplayprofile: registered -" << alias << " (" << argv[3] << ")\n";
        return 0;
    }

    if (action == "remove") {
        if (argc != 3) {
            std::cerr << "Usage: lplayprofile remove <alias>\n";
            return 2;
        }
        if (!profiles.erase(argv[2])) {
            std::cerr << "lplayprofile: no profile named " << argv[2] << "\n";
            return 1;
        }
        return store.save(profiles) ? 0 : 1;
    }

    if (action == "compile") {
        if (argc < 4) {
            usage(std::cerr);
            return 2;
        }
        std::string selector = argv[2];
        if (selector.size() < 2 || selector.front() != '-') {
            std::cerr << "Usage: lplayprofile compile -<alias> <source-file> [-o output]\n";
            return 2;
        }
        const std::string alias = selector.substr(1);
        const auto found = profiles.find(alias);
        if (found == profiles.end()) {
            std::cerr << "lplayprofile: compiler profile not found: -" << alias
                      << "\nAdd it with 'lplayprofile import'.\n";
            return 1;
        }

        const std::string input = argv[3];
        const std::string extension = fs::path(input).extension().string();
        if (!found->second.extension.empty() && found->second.extension != "*" &&
            found->second.extension != extension) {
            std::cerr << "lplayprofile: -" << alias << " expects " << found->second.extension
                      << " files, got " << extension << "\n";
            return 2;
        }

        std::string output = fs::path(input).stem().string();
        bool output_was_set = false;
        for (int i = 4; i < argc; ++i) {
            const std::string option = argv[i];
            if ((option == "-o" || option == "--output") && i + 1 < argc) {
                output = argv[++i];
                output_was_set = true;
            } else {
                std::cerr << "lplayprofile: unexpected or incomplete option: " << option << "\n";
                return 2;
            }
        }

        std::string command = found->second.command;
        const bool has_input = command.find("{input}") != std::string::npos;
        const bool has_output = command.find("{output}") != std::string::npos;
        if (output_was_set && !has_output) {
            std::cerr << "lplayprofile: this compiler profile does not use {output}; "
                         "add that placeholder to its command template\n";
            return 2;
        }
        replace_all(command, "{input}", shell_quote(input));
        replace_all(command, "{output}", shell_quote(output));
        if (!has_input) command += " " + shell_quote(input);

        std::cout << "lplayprofile: compiling with " << found->second.name << "\n";
        return std::system(command.c_str()) == 0 ? 0 : 1;
    }

    std::cerr << "lplayprofile: unknown command '" << action << "'\n";
    usage(std::cerr);
    return 2;
}
