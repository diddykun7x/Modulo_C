#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <cstdlib>
#include <cstdio>
#include <map>

std::string get_home() {
    const char* h = std::getenv("HOME");\
    return h ? std::string(h) : ".";
}

struct Profile {
    std::string name;
    std::string compiler_file;
    std::string run_cmd;
};

// ========================================================================
// 📋 RELIABLE JSON CONFIG MANAGER
// ========================================================================
class ConfigManager {
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
    ConfigManager(const std::string& path) : config_path(path) {}

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
        if (!out.is_open()) {
            std::cout << "\033[1;31m[lplay] Error:\033[0m Could not write to " << config_path << "\n";
            return;
        }

        out << "{\n  \"profiles\": {\n";
        size_t count = 0;
        for (const auto& [key, prof] : profiles) {
            out << "    \"" << escape_json(key) << "\": {\n";
            out << "      \"name\": \"" << escape_json(prof.name) << "\",\n";
            out << "      \"compiler_file\": \"" << escape_json(prof.compiler_file) << "\",\n";
            out << "      \"run_cmd\": \"" << escape_json(prof.run_cmd) << "\"\n";
            out << "    }" << (++count < profiles.size() ? "," : "") << "\n";
        }
        out << "  }\n}\n";
        out.close();

        std::cout << "\033[1;32m[lplay]\033[0m Config file updated -> \033[1m" << config_path << "\033[0m\n";
    }
};

// ========================================================================
// 🛠️ MERGED C% TRANSPILER CORE
// ========================================================================
class CPercentTranspiler {
private:
    bool has_rng = false;
    bool has_filesys = false;
    bool has_units = false;
    bool has_chronos = false;

public:
    std::string transpile(const std::string& source_code) {
        std::stringstream c_out;

        has_rng = source_code.find("<rng>") != std::string::npos;
        has_filesys = source_code.find("<filesys>") != std::string::npos;
        has_chronos = source_code.find("<chronos>") != std::string::npos;
        has_units = source_code.find("<units>") != std::string::npos ||
                    source_code.find("std::measure_unit") != std::string::npos ||
                    source_code.find("std::conversion_onOperation") != std::string::npos ||
                    source_code.find("std::a_of") != std::string::npos ||
                    source_code.find("std::V_of") != std::string::npos;

        // 1. Mandatory C++ Headers
        c_out << "// Auto-generated C++ code from C% Source\n";
        c_out << "#include <iostream>\n";
        c_out << "#include <string>\n";
        c_out << "#include <vector>\n";
        c_out << "#include <iomanip>\n";
        if (has_units) c_out << "#include <type_traits>\n#include <utility>\n";
        if (has_chronos) c_out << "#include <chrono>\n#include <ctime>\n#include <thread>\n#include <functional>\n";

        // 2. Header Injections
        if (has_rng) {
            c_out << "#include <random>\n";
        }
        if (has_filesys) {
            c_out << "#include <fstream>\n";
            c_out << "#include <cstdio>\n";
        }

        // 3. Math Constants
        c_out << "\n// --- C% Math Constants ---\n";
        c_out << "constexpr double O_PI = 3.14159265358979323846;\n";
        c_out << "constexpr double O_EULER = 2.71828182845904523536;\n";

        // 4. Typed measurement units. Values are stored in canonical base units;
        // prefixes affect construction/display, while arithmetic checks dimensions.
        if (has_units) {
            c_out << R"CPP(
namespace cpercent {
template<int L, int T, int M> struct dimension {};
using length_dimension = dimension<1, 0, 0>;
using area_dimension = dimension<2, 0, 0>;
using volume_dimension = dimension<3, 0, 0>;

template<class D> struct unit {
    long double factor; const char* symbol;
    constexpr unit(long double f, const char* s) : factor(f), symbol(s) {}
};
template<class D> struct display_value;
template<class D> struct quantity {
    long double base;
    constexpr explicit quantity(long double v) : base(v) {}
    template<class U> auto as(U u) const {
        static_assert(std::is_same<D, typename U::dimension_type>::value, "cannot convert between different dimensions");
        return display_value<D>(base, u.factor, u.symbol);
    }
    template<class U> auto convert(U u) const { return as(u); }
};
template<class D> struct typed_unit : unit<D> {
    using dimension_type = D;
    constexpr typed_unit(long double f, const char* s) : unit<D>(f, s) {}
};
template<class D> struct display_value { long double base, factor; const char* symbol;
    display_value(long double b, long double f, const char* s) : base(b), factor(f), symbol(s) {}
};

template<class D> quantity<D> operator+(quantity<D> a, quantity<D> b) { return quantity<D>(a.base + b.base); }
template<class D> quantity<D> operator-(quantity<D> a, quantity<D> b) { return quantity<D>(a.base - b.base); }
template<class D> bool operator==(quantity<D> a, quantity<D> b) { return a.base == b.base; }
template<class D> bool operator!=(quantity<D> a, quantity<D> b) { return !(a == b); }
template<class D> bool operator<(quantity<D> a, quantity<D> b) { return a.base < b.base; }
template<class D> bool operator<=(quantity<D> a, quantity<D> b) { return a.base <= b.base; }
template<class D> bool operator>(quantity<D> a, quantity<D> b) { return a.base > b.base; }
template<class D> bool operator>=(quantity<D> a, quantity<D> b) { return a.base >= b.base; }

template<class D> struct result_policy {
    long double factor = 1.0L; const char* symbol = "";
    template<class U> void result(U u) { factor = u.factor; symbol = u.symbol; }
};
inline result_policy<length_dimension> conversion_onOperation;

template<class D> std::ostream& operator<<(std::ostream& out, quantity<D> q) {
    out << static_cast<double>(q.base / conversion_onOperation.factor);
    if (*conversion_onOperation.symbol) out << conversion_onOperation.symbol;
    return out;
}
template<class D> std::ostream& operator<<(std::ostream& out, display_value<D> q) {
    return out << static_cast<double>(q.base / q.factor) << q.symbol;
}

template<class D> quantity<D> make_quantity(long double value, typed_unit<D> u) { return quantity<D>(value * u.factor); }
namespace units {
inline constexpr typed_unit<length_dimension> m(1.0L, "m"), dm(.1L, "dm"), cm(.01L, "cm"), mm(.001L, "mm"), km(1000.0L, "km"), dam(10.0L, "dam"), hm(100.0L, "hm");
inline constexpr typed_unit<area_dimension> am(1.0L, "am"), adm(.01L, "adm"), acm(.0001L, "acm"), akm(1000000.0L, "akm");
inline constexpr typed_unit<volume_dimension> Vm(1.0L, "Vm"), Vdm(.001L, "Vdm"), Vcm(.000001L, "Vcm"), Vkm(1000000000.0L, "Vkm");
inline constexpr typed_unit<length_dimension> u(1.0L, "u"), du(.1L, "du"), cu(.01L, "cu"), mu(.001L, "mu"), ku(1000.0L, "ku"), dau(10.0L, "dau"), hu(100.0L, "hu");
inline constexpr typed_unit<area_dimension> au(1.0L, "au"), acu(.0001L, "acu"), adu(.01L, "adu");
inline constexpr typed_unit<volume_dimension> Vu(1.0L, "Vu"), Vcu(.000001L, "Vcu"), Vdu(.001L, "Vdu");
}
}
)CPP";
        }

        // 5. Chronos durations, elapsed timers, counters and deterministic tick loops.
        if (has_chronos) {
            c_out << R"CPP(
namespace chronos {
using namespace std::chrono_literals;
using duration = std::chrono::duration<double>;
struct calendar_period { int months; };
inline calendar_period months(int n) { return {n}; }
inline calendar_period years(int n) { return {n * 12}; }
inline std::time_t add_to(std::time_t value, calendar_period p) {
    std::tm tm = *std::localtime(&value); tm.tm_mon += p.months;
    return std::mktime(&tm);
}
struct time_counter {
    bool active = false; std::chrono::steady_clock::time_point began{}; duration accumulated{0};
    void use() { if (!active) { began = std::chrono::steady_clock::now(); active = true; } }
    void stop() { if (active) { accumulated += std::chrono::steady_clock::now() - began; active = false; } }
    void reset() { active = false; accumulated = duration::zero(); }
    duration sum() const { return accumulated + (active ? std::chrono::steady_clock::now() - began : duration::zero()); }
    duration sum(duration d) { accumulated += d; return accumulated; }
};
inline time_counter time_count;
struct timer {
    duration span{0}; std::chrono::steady_clock::time_point began = std::chrono::steady_clock::now();
    timer() = default;
    template<class Rep, class Period> timer(std::chrono::duration<Rep, Period> d) : span(d), began(std::chrono::steady_clock::now()) {}
    bool expired() const { return std::chrono::steady_clock::now() - began >= span; }
    duration elapsed() const { return std::chrono::steady_clock::now() - began; }
    duration remaining() const { auto r = span - elapsed(); return r > duration::zero() ? r : duration::zero(); }
    void restart() { began = std::chrono::steady_clock::now(); }
    void wait() const { std::this_thread::sleep_until(began + span); }
};
struct clock {
    template<class Rep, class Period, class Fn> static void for_ticks(std::size_t count, std::chrono::duration<Rep, Period> step, Fn fn) {
        auto next = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < count; ++i) { next += step; fn(i); std::this_thread::sleep_until(next); }
    }
};
}
)CPP";
            c_out << "using namespace std::chrono_literals;\n";
        }

        // 6. RNG Support
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

        // 7. Full Terminal Color Namespace (Merged OG + New)
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

        // 8. Filesystem Helper Macros
        if (has_filesys) {
            c_out << "// --- C% <filesys> Library ---\n";
            c_out << "inline void createFile(const std::string& name) { std::ofstream f(name); }\n";
            c_out << "inline void deleteFile(const std::string& name) { std::remove(name.c_str()); }\n";
            c_out << "#define iFile(name) std::ofstream(name, std::ios::app)\n";
            c_out << "#define oFile(name) std::ifstream(name)\n";
            c_out << "#define +File(name) createFile(name)\n";
            c_out << "#define -File(name) deleteFile(name)\n\n";
        }

        // 9. Line Transformation Processing
        std::stringstream ss(source_code);
        std::string line;
        bool user_has_main = false;

        while (std::getline(ss, line)) {
            if (line.find("<iostream>") != std::string::npos ||
                line.find("<clrnsp>") != std::string::npos ||
                line.find("<logicgate>") != std::string::npos ||
                line.find("<filesys>") != std::string::npos ||
                line.find("<memManagement>") != std::string::npos ||
                line.find("<rng>") != std::string::npos ||
                line.find("<chronos>") != std::string::npos ||
                line.find("<units>") != std::string::npos) {
                continue;
            }

            // Library import directives are compile-time declarations in C%.
            if (line.find("chronos::timer.import;") != std::string::npos ||
                line.find("chronos::clock.import;") != std::string::npos) continue;
            line = std::regex_replace(line, std::regex(R"(chronos::time_count\.use\s*;)"), "chronos::time_count.use();");
            line = std::regex_replace(line, std::regex(R"(chronos::time_count\.stop\s*;)"), "chronos::time_count.stop();");
            line = std::regex_replace(line, std::regex(R"(chronos::time_count\.reset\s*;)"), "chronos::time_count.reset();");
            line = std::regex_replace(line, std::regex(R"((\d+(?:\.\d+)?)\s*(?:sec|second|seconds)\b)"), "$1s");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*(?:minute|minutes)\b)"), "$1min");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*(?:hour|hours)\b)"), "$1h");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*(?:month|months)\b)"), "chronos::months($1)");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*(?:year|years)\b)"), "chronos::years($1)");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*decade\b)"), "chronos::years(($1) * 10)");
            line = std::regex_replace(line, std::regex(R"(chronos::timer\s+([A-Za-z_]\w*)\s*=\s*)"), "chronos::timer $1 = ");

            // Generated area/volume formula declarations are represented by the
            // dimension exponents in the runtime types below.
            if (line.find("std::a_of.formula") != std::string::npos ||
                line.find("std::V_of.formula") != std::string::npos) continue;
            line = std::regex_replace(line,
                std::regex(R"(std::measure_unit\s*<\s*std::default_scale\s*>\s*([A-Za-z_]\w*)\s*\(\s*([A-Za-z_]\w*)\s*\)\s*;)"),
                "auto $1 = cpercent::units::$2;");
            line = std::regex_replace(line,
                std::regex(R"(std::conversion_onOperation\.result\s*\(\s*into\s*:\s*([A-Za-z_]\w*)\s*\)\s*;)"),
                "cpercent::conversion_onOperation.result(cpercent::units::$1);");
            line = std::regex_replace(line,
                std::regex(R"(\.convert\s*\(\s*into\s*:\s*([A-Za-z_]\w*)\s*\))"),
                ".as(cpercent::units::$1)");

            if (has_units) {
                struct LiteralUnit { const char* suffix; const char* dimension; const char* factor; const char* symbol; };
                const std::vector<LiteralUnit> unit_literals = {
                    {"Vcm", "volume_dimension", "0.000001L", "Vcm"}, {"Vdm", "volume_dimension", "0.001L", "Vdm"},
                    {"Vkm", "volume_dimension", "1000000000.0L", "Vkm"}, {"Vm", "volume_dimension", "1.0L", "Vm"},
                    {"Vcu", "volume_dimension", "0.000001L", "Vcu"}, {"Vdu", "volume_dimension", "0.001L", "Vdu"}, {"Vu", "volume_dimension", "1.0L", "Vu"},
                    {"acm", "area_dimension", "0.0001L", "acm"}, {"adm", "area_dimension", "0.01L", "adm"}, {"akm", "area_dimension", "1000000.0L", "akm"}, {"am", "area_dimension", "1.0L", "am"},
                    {"acu", "area_dimension", "0.0001L", "acu"}, {"adu", "area_dimension", "0.01L", "adu"}, {"au", "area_dimension", "1.0L", "au"},
                    {"dau", "length_dimension", "10.0L", "u"}, {"hu", "length_dimension", "100.0L", "u"}, {"ku", "length_dimension", "1000.0L", "u"}, {"mu", "length_dimension", "0.001L", "u"}, {"cu", "length_dimension", "0.01L", "u"}, {"du", "length_dimension", "0.1L", "u"}, {"u", "length_dimension", "1.0L", "u"},
                    {"dam", "length_dimension", "10.0L", "m"}, {"hm", "length_dimension", "100.0L", "m"}, {"km", "length_dimension", "1000.0L", "m"}, {"dm", "length_dimension", "0.1L", "m"}, {"cm", "length_dimension", "0.01L", "m"}, {"mm", "length_dimension", "0.001L", "m"}, {"m", "length_dimension", "1.0L", "m"}
                };
                for (const auto& entry : unit_literals) {
                    std::regex literal(R"((\b\d+(?:\.\d+)?))" + std::string(entry.suffix) + R"(\b)");
                    line = std::regex_replace(line, literal,
                        "cpercent::make_quantity<cpercent::" + entry.dimension + ">($1L, cpercent::typed_unit<cpercent::" + entry.dimension + ">(" + entry.factor + ", \"" + entry.symbol + "\"))");
                }
                line = std::regex_replace(line, std::regex(R"(\b(?:num|int|dec|double)\s+(\w+)\s*=\s*(cpercent::make_quantity))"), "auto $1 = $2");
            }

            if (line.find("main") != std::string::npos) user_has_main = true;

            // Types & Signatures
            line = std::regex_replace(line, std::regex("\\bnum main\\b"), "int main");
            line = std::regex_replace(line, std::regex("\\bconstvar\\b"), "const");
            line = std::regex_replace(line, std::regex("\\bnum\\b"), "int");
            line = std::regex_replace(line, std::regex("\\bdec\\b"), "double");
            line = std::regex_replace(line, std::regex("\\bcstring\\b"), "std::string");

            // Streams
            line = std::regex_replace(line, std::regex("\\boutput\\b"), "std::cout");
            line = std::regex_replace(line, std::regex("\\binput\\b"), "std::cin");

            // Logic Operators & Gates
            line = std::regex_replace(line, std::regex("\\bANDL\\b"), "&&");
            line = std::regex_replace(line, std::regex("\\bORL\\b"), "||");
            line = std::regex_replace(line, std::regex("\\bNOTL\\b"), "!");
            line = std::regex_replace(line, std::regex("\\bXORL\\b"), "^");

            // Boolean Keywords
            line = std::regex_replace(line, std::regex("\\bfact\\b"), "bool");
            line = std::regex_replace(line, std::regex("\\bTrue\\b"), "true");
            line = std::regex_replace(line, std::regex("\\bFalse\\b"), "false");
            line = std::regex_replace(line, std::regex("\\bT\\b"), "true");
            line = std::regex_replace(line, std::regex("\\bF\\b"), "false");

            // Constants & Pointers
            line = std::regex_replace(line, std::regex("#O_PI"), "O_PI");
            line = std::regex_replace(line, std::regex("#O_EULER"), "O_EULER");
            line = std::regex_replace(line, std::regex("@"), "&");

            c_out << line << "\n";
        }

        if (!user_has_main) {
            c_out << "\nint main() { return 0; }\n";
        }

        return c_out.str();
    }
};

// ========================================================================
// 🚀 MAIN DRIVER
// ========================================================================
int main(int argc, char* argv[]) {
    std::string lplay_dir = get_home() + "/.lplay";
    std::string config_path = lplay_dir + "/config.json";
    ConfigManager cfg(config_path);

    std::system(("mkdir -p \"" + lplay_dir + "/compilers\"").c_str());

    if (argc < 2) {
        std::cout << "\033[1;36m=== LangPlay CLI Engine (lplay) ===\033[0m\n";
        std::cout << "Commands:\n";
        std::cout << "  lplay <file.cmod> [-o output]                          Compile C% source file\n";
        std::cout << "  lplay upload comp <path>                               Upload compiler file\n";
        std::cout << "  lplay setprofile <nick> <name> <comp> \"<run_cmd>\"     Save profile to config.json\n";
        std::cout << "  lplay listprofiles                                    List profiles from config.json\n";
        std::cout << "  lplay useprofile <nick> <file> -o <out>                Run via profile\n";
        std::cout << "  lplay -v                                               Show version info\n";
        return 0;
    }

    std::string arg1 = argv[1];

    // Version Check
    if (arg1 == "-v") {
        std::cout << "\033[1;36mlplay C% Compiler v2.0 (Unified Build)\033[0m\n";
        return 0;
    }

    // Upload Compiler
    if (arg1 == "upload" && argc >= 4 && std::string(argv[2]) == "comp") {
        std::string src = argv[3];
        std::string cp_cmd = "cp \"" + src + "\" \"" + lplay_dir + "/compilers/\"";
        if (std::system(cp_cmd.c_str()) == 0) {
            std::cout << "\033[1;32m[lplay]\033[0m Uploaded to " << lplay_dir << "/compilers/\n";
        } else {
            std::cout << "\033[1;31m[lplay] Error uploading file.\033[0m\n";
        }
        return 0;
    }

    // List Profiles
    if (arg1 == "listprofiles") {
        auto profiles = cfg.load_profiles();
        std::cout << "\033[1;36m=== Installed Profiles (" << config_path << ") ===\033[0m\n";
        if (profiles.empty()) {
            std::cout << "No saved profiles found.\n";
        } else {
            for (const auto& [nick, p] : profiles) {
                std::cout << " - \033[1m" << nick << "\033[0m (" << p.name << ")\n";
                std::cout << "   File: " << p.compiler_file << "\n";
                std::cout << "   Cmd:  " << p.run_cmd << "\n\n";
            }
        }
        return 0;
    }

    // Set Profile
    if (arg1 == "setprofile" && argc >= 6) {
        std::string nick = argv[2];
        Profile p{argv[3], argv[4], argv[5]};
        cfg.save_profile(nick, p);
        std::cout << "\033[1;32m[lplay]\033[0m Saved profile '\033[1m" << nick << "\033[0m' to config.json!\n";
        return 0;
    }

    // Use Profile
    if (arg1 == "useprofile" && argc >= 3) {
        std::string nick = argv[2];
        auto profiles = cfg.load_profiles();

        if (profiles.find(nick) == profiles.end()) {
            std::cout << "\033[1;31m[lplay] Error:\033[0m Profile '\033[1m" << nick << "\033[0m' not found in config.json.\n";
            return 1;
        }

        Profile p = profiles[nick];
        std::string input_file = (argc >= 4) ? argv[3] : "";
        std::string output_file = "a.out";

        for (int i = 4; i < argc; ++i) {
            if (std::string(argv[i]) == "-o" && i + 1 < argc) {
                output_file = argv[i + 1];
                i++;
            }
        }

        std::string cmd = p.run_cmd;
        cmd = std::regex_replace(cmd, std::regex("\\{input\\}"), input_file);
        cmd = std::regex_replace(cmd, std::regex("\\{output\\}"), output_file);
        cmd = std::regex_replace(cmd, std::regex("\\{compiler_file\\}"), p.compiler_file);
        cmd = std::regex_replace(cmd, std::regex("\\{lplay_dir\\}"), lplay_dir);

        std::cout << "\033[1;33m[lplay]\033[0m Executing [\033[1m" << p.name << "\033[0m]...\n";
        return std::system(cmd.c_str());
    }

    // Direct Transpile and Build Routine
    std::string input_file = arg1;
    std::string output_file = "a.out";

    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            output_file = argv[i + 1];
            i++;
        }
    }

    std::ifstream src_stream(input_file);
    if (!src_stream.is_open()) {
        std::cout << "\033[1;31m[lplay] Error:\033[0m Could not open file '" << input_file << "'\n";
        return 1;
    }

    std::cout << "\033[1;32m[lplay]\033[0m Reading " << input_file << "...\n";

    std::string src_content((std::istreambuf_iterator<char>(src_stream)), std::istreambuf_iterator<char>());
    src_stream.close();

    CPercentTranspiler transpiler;
    std::string transpiled = transpiler.transpile(src_content);

    std::ofstream cache_out("_build_cache.cpp");
    cache_out << transpiled;
    cache_out.close();

    std::cout << "\033[1;33m[lplay]\033[0m Transpiling C% to C++ -> Compiling binary...\n";

    std::string compile_cmd = "clang++ -std=c++17 _build_cache.cpp -o \"" + output_file + "\"";
    int build_res = std::system(compile_cmd.c_str());
    std::remove("_build_cache.cpp");

    if (build_res == 0) {
        std::cout << "\033[1;32m[lplay] ✅ 🤯 SUCCESS!\033[0m Executable created: \033[1m" << output_file << "\033[0m\n";
    } else {
        std::cout << "\033[1;31m[lplay] ❌ 🤯 BUILD FAILED!\033[0m Check your C% syntax above.\n";
    }

    return 0;
}
