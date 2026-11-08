#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <regex>
#include <cstdlib>

std::string get_home() {
    const char* h = std::getenv("HOME");
    return h ? std::string(h) : ".";
}

struct Profile {
    std::string name;
    std::string compiler_file;
    std::string run_cmd;
};

class LegacyProfileManager {
private:
    std::string config_path;

    std::string escape_json(const std::string& s) {
        std::string res = "";
        for (char c : s) {
            if (c == '"') res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else res += c;
        }
        return res;
    }

public:
    LegacyProfileManager(const std::string& path) : config_path(path) {}

    std::map<std::string, Profile> load_profiles() {
        std::map<std::string, Profile> profiles;
        std::ifstream in(config_path);
        if (!in.is_open()) return profiles;

        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        std::regex prof_regex(R"(\"([^\"]+)\"\s*:\s*\{[^}]*\"name\"\s*:\s*\"([^\"]+)\"[^}]*\"compiler_file\"\s*:\s*\"([^\"]+)\"[^}]*\"run_cmd\"\s*:\s*\"([^\"]+)\")");
        auto begin = std::sregex_iterator(content.begin(), content.end(), prof_regex);
        auto end = std::sregex_iterator();

        for (std::sregex_iterator i = begin; i != end; ++i) {
            std::smatch m = *i;
            profiles[m[1]] = Profile{m[2], m[3], m[4]};
        }

        return profiles;
    }

    void save_profile(const std::string& nick, const Profile& p) {
        auto profiles = load_profiles();
        profiles[nick] = p;

        std::ofstream out(config_path, std::ios::trunc);
        if (!out.is_open()) return;

        out << "{\n  \"profiles\": {\n";
        size_t count = 0;
        for (const auto& pair : profiles) {
            const std::string& key = pair.first;
            const Profile& prof = pair.second;
            out << "    \"" << escape_json(key) << "\": {\n";
            out << "      \"name\": \"" << escape_json(prof.name) << "\",\n";
            out << "      \"compiler_file\": \"" << escape_json(prof.compiler_file) << "\",\n";
            out << "      \"run_cmd\": \"" << escape_json(prof.run_cmd) << "\"\n";
            out << "    }" << (++count < profiles.size() ? "," : "") << "\n";
        }
        out << "  }\n}\n";
        out.close();
    }
};

int main(int argc, char* argv[]) {
    std::string config_path = get_home() + "/.lplay/config.json";
    LegacyProfileManager pm(config_path);

    if (argc < 2) {
        std::cout << "\033[1;36m=== Legacy Profile Manager (lplayprofile) ===\033[0m\n";
        std::cout << "Usage:\n";
        std::cout << "  lplayprofile list\n";
        std::cout << "  lplayprofile add <nick> <name> <compiler> \"<run_cmd>\"\n";
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "list") {
        auto profiles = pm.load_profiles();
        std::cout << "\033[1;36m=== Legacy Config Profiles ===\033[0m\n";
        for (const auto& pair : profiles) {
            std::cout << " - \033[1m" << pair.first << "\033[0m (" << pair.second.name << ")\n";
        }
        return 0;
    }

    if (cmd == "add" && argc >= 6) {
        pm.save_profile(argv[2], Profile{argv[3], argv[4], argv[5]});
        std::cout << "\033[1;32m[lplayprofile]\033[0m Legacy profile saved!\n";
        return 0;
    }

    return 0;
}
