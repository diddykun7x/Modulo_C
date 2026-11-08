#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <regex>
#include <cstdlib>

std::string get_home() {
    const char* h = std::getenv("HOME");
    return h ? std::string(h) : ".";
}

class NoteManager {
private:
    std::string notes_path;

    std::string escape_json(const std::string& s) {
        std::string res = "";
        for (char c : s) {
            if (c == '"') res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else if (c == '\n') res += "\\n";
            else res += c;
        }
        return res;
    }

public:
    NoteManager(const std::string& path) : notes_path(path) {}

    std::map<std::string, std::string> load_notes() {
        std::map<std::string, std::string> notes;
        std::ifstream in(notes_path);
        if (!in.is_open()) return notes;

        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        std::regex note_regex(R"(\"([^\"]+)\"\s*:\s*\"([^\"]+)\")");
        auto begin = std::sregex_iterator(content.begin(), content.end(), note_regex);
        auto end = std::sregex_iterator();

        for (std::sregex_iterator i = begin; i != end; ++i) {
            std::smatch m = *i;
            std::string val = m[2];
            val = std::regex_replace(val, std::regex("\\\\n"), "\n");
            val = std::regex_replace(val, std::regex("\\\\\""), "\"");
            notes[m[1]] = val;
        }

        return notes;
    }

    void save_note(const std::string& key, const std::string& content) {
        auto notes = load_notes();
        notes[key] = content;

        std::ofstream out(notes_path, std::ios::trunc);
        if (!out.is_open()) {
            std::cout << "\033[1;31m[lplaynote] Error:\033[0m Could not write to " << notes_path << "\n";
            return;
        }

        out << "{\n";
        size_t count = 0;
        for (const auto& [k, v] : notes) {
            out << "  \"" << escape_json(k) << "\": \"" << escape_json(v) << "\"" 
                << (++count < notes.size() ? "," : "") << "\n";
        }
        out << "}\n";
        out.close();

        std::cout << "\033[1;32m[lplaynote]\033[0m Saved note for '\033[1m" << key << "\033[0m'!\n";
    }

    void delete_note(const std::string& key) {
        auto notes = load_notes();
        if (notes.erase(key)) {
            std::ofstream out(notes_path, std::ios::trunc);
            out << "{\n";
            size_t count = 0;
            for (const auto& [k, v] : notes) {
                out << "  \"" << escape_json(k) << "\": \"" << escape_json(v) << "\"" 
                    << (++count < notes.size() ? "," : "") << "\n";
            }
            out << "}\n";
            out.close();
            std::cout << "\033[1;32m[lplaynote]\033[0m Deleted note '\033[1m" << key << "\033[0m'.\n";
        } else {
            std::cout << "\033[1;31m[lplaynote] Error:\033[0m Note '\033[1m" << key << "\033[0m' not found.\n";
        }
    }
};

void seed_defaults(NoteManager& nm) {
    auto notes = nm.load_notes();
    if (notes.empty()) {
        nm.save_note("rng", "<rng>\n\nnum roll = rdnGen.range(1, 6); // Generates 1-6");
        nm.save_note("types", "C% Types:\n  num    -> int\n  dec    -> double\n  cstring -> std::string");
    }
}

int main(int argc, char* argv[]) {
    std::string lplay_dir = get_home() + "/.lplay";
    std::string notes_path = lplay_dir + "/notes.json";
    
    std::system(("mkdir -p " + lplay_dir).c_str());
    NoteManager nm(notes_path);
    seed_defaults(nm);

    if (argc < 2) {
        std::cout << "\033[1;36m=== LangPlay Cheat Sheet & Notes (lplaynote) ===\033[0m\n";
        std::cout << "Usage:\n";
        std::cout << "  lplaynote get <tag>                 View a note\n";
        std::cout << "  lplaynote add <tag> \"<text>\"         Add/update a note\n";
        std::cout << "  lplaynote list                      List all saved note tags\n";
        std::cout << "  lplaynote del <tag>                 Delete a note\n";
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "list") {
        auto notes = nm.load_notes();
        std::cout << "\033[1;36m=== Saved Notes ===\033[0m\n";
        for (const auto& [tag, content] : notes) {
            std::cout << " - \033[1m" << tag << "\033[0m\n";
        }
        return 0;
    }

    if (cmd == "get" && argc >= 3) {
        std::string tag = argv[2];
        auto notes = nm.load_notes();
        if (notes.find(tag) != notes.end()) {
            std::cout << "\033[1;33m[" << tag << "]\033[0m\n" << notes[tag] << "\n";
        } else {
            std::cout << "\033[1;31m[lplaynote] Error:\033[0m No note found for tag '\033[1m" << tag << "\033[0m'.\n";
        }
        return 0;
    }

    if (cmd == "add" && argc >= 4) {
        std::string tag = argv[2];
        std::string content = argv[3];
        nm.save_note(tag, content);
        return 0;
    }

    if (cmd == "del" && argc >= 3) {
        nm.delete_note(argv[2]);
        return 0;
    }

    return 0;
}
