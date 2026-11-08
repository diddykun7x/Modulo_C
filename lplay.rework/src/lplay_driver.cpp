#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <filesystem>
#include <chrono>
#include <cctype>
#include <cstdint>

std::string cpp_string_literal(const std::string& value) {
    std::string escaped;
    for (char c : value) {
        if (c == '\\') escaped += "\\\\";
        else if (c == '"') escaped += "\\\"";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else escaped += c;
    }
    return escaped;
}

// Translate C% source into C++.
class CPercentTranspiler {
private:
    bool has_rng = false;
    bool has_filesys = false;
    bool has_fstream = false;
    bool has_units = false;
    bool has_chronos = false;

public:
    std::string transpile(const std::string& source_code, const std::string& source_name = "<input>") {
        std::stringstream c_out;

        has_rng = source_code.find("<rng>") != std::string::npos;
        has_filesys = source_code.find("<filesys>") != std::string::npos;
        has_fstream = source_code.find("<fstream>") != std::string::npos;
        has_chronos = source_code.find("<chronos>") != std::string::npos;
        has_units = source_code.find("<units>") != std::string::npos ||
                    source_code.find("std::measure_unit") != std::string::npos ||
                    source_code.find("std::scale ") != std::string::npos ||
                    source_code.find("std::scale_character_set") != std::string::npos ||
                    source_code.find("std::conversion_onOperation") != std::string::npos ||
                    source_code.find("std::a_of") != std::string::npos ||
                    source_code.find("std::V_of") != std::string::npos;

        // Add the headers every generated program needs.
        c_out << "// Auto-generated C++ code from C% Source\n";
        c_out << "#include <iostream>\n";
        c_out << "#include <string>\n";
        c_out << "#include <vector>\n";
        c_out << "#include <iomanip>\n";
        if (has_units) c_out << "#include <type_traits>\n#include <utility>\n#include <typeinfo>\n#include <map>\n#include <cmath>\n#include <algorithm>\n#include <stdexcept>\n";
        if (has_chronos) c_out << "#include <chrono>\n#include <ctime>\n#include <thread>\n#include <functional>\n#include <algorithm>\n#include <cstddef>\n";

        // Add headers for the imported C% libraries.
        if (has_rng) {
            c_out << "#include <random>\n";
        }
        if (has_filesys) {
            c_out << "#include <fstream>\n";
            c_out << "#include <cstdio>\n";
        }
        if (has_fstream && !has_filesys) c_out << "#include <fstream>\n";

        // Keep the C% math constants available to every program.
        c_out << "\n// --- C% Math Constants ---\n";
        c_out << "constexpr double O_PI = 3.14159265358979323846;\n";
        c_out << "constexpr double O_EULER = 2.71828182845904523536;\n";

        // Store unit values in base units and check dimensions during arithmetic.
        if (has_units) {
            c_out << R"CPP(
namespace cpercent {
template<int L, int T, int M> struct dimension {};
using length_dimension = dimension<1, 0, 0>;
using area_dimension = dimension<2, 0, 0>;
using volume_dimension = dimension<3, 0, 0>;

template<class D> constexpr const char* base_symbol();
template<class D> struct unit {
    long double factor; const char* symbol; const char* base_symbol;
    constexpr unit(long double f, const char* s, const char* b = nullptr)
        : factor(f), symbol(s), base_symbol(b ? b : cpercent::base_symbol<D>()) {}
};
template<class D> struct display_value;
template<class D> struct dimension_info;
template<int L, int T, int M> struct dimension_info<dimension<L, T, M>> {
    static constexpr int length = L, time = T, mass = M;
};
template<class A, class B> using product_dimension = dimension<
    dimension_info<A>::length + dimension_info<B>::length,
    dimension_info<A>::time + dimension_info<B>::time,
    dimension_info<A>::mass + dimension_info<B>::mass>;
template<class A, class B> using quotient_dimension = dimension<
    dimension_info<A>::length - dimension_info<B>::length,
    dimension_info<A>::time - dimension_info<B>::time,
    dimension_info<A>::mass - dimension_info<B>::mass>;
template<class D> constexpr const char* base_symbol() {
    if constexpr (std::is_same<D, length_dimension>::value) return "m";
    else if constexpr (std::is_same<D, area_dimension>::value) return "am";
    else if constexpr (std::is_same<D, volume_dimension>::value) return "Vm";
    else return "compound";
}
template<class D> struct quantity {
    long double base;
    std::string base_unit;
    explicit quantity(long double v, const char* symbol = cpercent::base_symbol<D>()) : base(v), base_unit(symbol) {}
    template<class U> auto as(U u) const {
        static_assert(std::is_same<D, typename U::dimension_type>::value, "cannot convert between different dimensions");
        return display_value<D>(base, u.factor, u.symbol);
    }
    template<class U> auto convert(U u) const { return as(u); }
};
template<class D> struct typed_unit : unit<D> {
    using dimension_type = D;
    constexpr typed_unit(long double f, const char* s, const char* b = nullptr) : unit<D>(f, s, b) {}
};
struct scale_definition {
    long double range = 10.0L, base_factor = 1.0L;
    std::map<std::string, long double> prefixes;
    void add_prefix(int marker, const std::string& name) {
        if (marker == 0 || range <= 0.0L) return;
        const long double steps = std::log(static_cast<long double>(std::abs(marker))) / std::log(range);
        long double multiplier = std::pow(range, steps);
        if (marker < 0) multiplier = 1.0L / multiplier;
        const std::string code = name == "deca" ? "da" : name.substr(0, 1);
        prefixes[code] = multiplier;
    }
    long double factor(const std::string& code) const {
        auto it = prefixes.find(code);
        if (it == prefixes.end()) throw std::invalid_argument("prefix is not defined by this scale: " + code);
        return it->second;
    }
};
template<class D> struct display_value { long double base, factor; const char* symbol;
    display_value(long double b, long double f, const char* s) : base(b), factor(f), symbol(s) {}
};

template<class D> quantity<D> operator+(quantity<D> a, quantity<D> b) { return quantity<D>(a.base + b.base, a.base_unit.c_str()); }
template<class D> quantity<D> operator-(quantity<D> a, quantity<D> b) { return quantity<D>(a.base - b.base, a.base_unit.c_str()); }
template<class D> bool operator==(quantity<D> a, quantity<D> b) { return a.base == b.base; }
template<class D> bool operator!=(quantity<D> a, quantity<D> b) { return !(a == b); }
template<class D> bool operator<(quantity<D> a, quantity<D> b) { return a.base < b.base; }
template<class D> bool operator<=(quantity<D> a, quantity<D> b) { return a.base <= b.base; }
template<class D> bool operator>(quantity<D> a, quantity<D> b) { return a.base > b.base; }
template<class D> bool operator>=(quantity<D> a, quantity<D> b) { return a.base >= b.base; }
template<class A, class B> quantity<product_dimension<A, B>> operator*(quantity<A> a, quantity<B> b) {
    using D = product_dimension<A, B>;
    std::string symbol = base_symbol<D>();
    if constexpr (std::is_same<D, area_dimension>::value) {
        if (a.base_unit == b.base_unit) symbol = "a" + a.base_unit;
    } else if constexpr (std::is_same<D, volume_dimension>::value) {
        const std::string base = !a.base_unit.empty() && a.base_unit[0] == 'a' ? a.base_unit.substr(1) : a.base_unit;
        if (base == b.base_unit) symbol = "V" + base;
    }
    return quantity<D>(a.base * b.base, symbol.c_str());
}
template<class A, class B> quantity<quotient_dimension<A, B>> operator/(quantity<A> a, quantity<B> b) { return quantity<quotient_dimension<A, B>>(a.base / b.base); }
template<class D> quantity<D> operator*(quantity<D> a, long double n) { return quantity<D>(a.base * n, a.base_unit.c_str()); }
template<class D> quantity<D> operator*(long double n, quantity<D> a) { return a * n; }
template<class D> quantity<D> operator/(quantity<D> a, long double n) { return quantity<D>(a.base / n, a.base_unit.c_str()); }

struct result_policy {
    long double factor = 1.0L; const char* symbol = ""; const std::type_info* dimension = &typeid(void);
    template<class U> void result(U u) { factor = u.factor; symbol = u.symbol; dimension = &typeid(typename U::dimension_type); }
};
inline result_policy conversion_onOperation;

template<class D> std::ostream& operator<<(std::ostream& out, quantity<D> q) {
    const bool requested = *conversion_onOperation.symbol && *conversion_onOperation.dimension == typeid(D);
    const long double factor = requested ? conversion_onOperation.factor : 1.0L;
    out << static_cast<double>(q.base / factor);
    if (requested) out << conversion_onOperation.symbol;
    else out << q.base_unit;
    return out;
}
template<class D> std::ostream& operator<<(std::ostream& out, display_value<D> q) {
    return out << static_cast<double>(q.base / q.factor) << q.symbol;
}

template<class D> quantity<D> make_quantity(long double value, typed_unit<D> u) { return quantity<D>(value * u.factor, u.base_symbol); }
namespace units {
inline constexpr typed_unit<length_dimension> m(1.0L, "m"), dm(.1L, "dm"), cm(.01L, "cm"), mm(.001L, "mm"), km(1000.0L, "km"), dam(10.0L, "dam"), hm(100.0L, "hm");
inline constexpr typed_unit<area_dimension> am(1.0L, "am"), adm(.01L, "adm"), acm(.0001L, "acm"), akm(1000000.0L, "akm");
inline constexpr typed_unit<volume_dimension> Vm(1.0L, "Vm"), Vdm(.001L, "Vdm"), Vcm(.000001L, "Vcm"), Vkm(1000000000.0L, "Vkm");
inline constexpr typed_unit<length_dimension> u(1.0L, "u", "u"), du(.1L, "du", "u"), cu(.01L, "cu", "u"), mu(.001L, "mu", "u"), ku(1000.0L, "ku", "u"), dau(10.0L, "dau", "u"), hu(100.0L, "hu", "u");
inline constexpr typed_unit<area_dimension> au(1.0L, "au", "au"), acu(.0001L, "acu", "au"), adu(.01L, "adu", "au");
inline constexpr typed_unit<volume_dimension> Vu(1.0L, "Vu", "Vu"), Vcu(.000001L, "Vcu", "Vu"), Vdu(.001L, "Vdu", "Vu");
}
}
)CPP";
        }

        // Time units, timers, counters, and tick loops.
        if (has_chronos) {
            c_out << R"CPP(
namespace chronos {
using namespace std::chrono_literals;
using duration = std::chrono::duration<double>;
struct calendar_period { int months; };
inline calendar_period months(int n) { return {n}; }
inline calendar_period years(int n) { return {n * 12}; }
inline std::time_t add_to(std::time_t value, calendar_period p) {
    std::tm* local = std::localtime(&value);
    if (!local) return value;
    std::tm tm = *local;
    const int original_day = tm.tm_mday;
    tm.tm_mday = 1;
    tm.tm_mon += p.months;
    tm.tm_isdst = -1;
    std::mktime(&tm);
    const int year = tm.tm_year + 1900;
    const int month = tm.tm_mon + 1;
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int days[] = {31, leap ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    tm.tm_mday = std::min(original_day, days[month - 1]);
    tm.tm_isdst = -1;
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
        for (std::size_t i = 0; i < count; ++i) {
            next += step;
            fn(i);
            if (i + 1 < count) std::this_thread::sleep_until(next);
        }
    }
};
inline std::ostream& operator<<(std::ostream& out, duration d) { return out << d.count() << "s"; }
}
)CPP";
            c_out << "using namespace std::chrono_literals;\n";
        }

        // Random-number support.
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

        // Terminal colors.
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

        // File helpers.
        if (has_filesys) {
            c_out << "// --- C% <filesys> Library ---\n";
            c_out << "inline void createFile(const std::string& name) { std::ofstream f(name); }\n";
            c_out << "inline void deleteFile(const std::string& name) { std::remove(name.c_str()); }\n";
            c_out << "#define iFile(name) std::ofstream(name, std::ios::app)\n";
            c_out << "#define oFile(name) std::ifstream(name)\n";
            c_out << "#define +File(name) createFile(name)\n";
            c_out << "#define -File(name) deleteFile(name)\n\n";
        }

        // Translate C% source lines into C++.
        std::stringstream ss(source_code);
        std::string line;
        bool user_has_main = false;
        bool in_scale_block = false;
        bool in_scale_character_set = false;
        std::string active_scale;
        unsigned scale_init_index = 0;
        size_t source_line_number = 0;
        std::map<std::string, std::string> custom_unit_scales;
        const std::regex custom_unit_decl(R"(std::measure_unit\s*<\s*(?:std::)?([A-Za-z_]\w*)\s*>\s*([A-Za-z_]\w*)\s*\(\s*([A-Za-z_]\w*)\s*\)\s*;)");
        for (std::sregex_iterator i(source_code.begin(), source_code.end(), custom_unit_decl), end; i != end; ++i) {
            if ((*i)[1] != "default_scale" && (*i)[1] != "square_scale" && (*i)[1] != "cubic_scale")
                custom_unit_scales[(*i)[3]] = (*i)[1];
        }
        auto custom_unit_expression = [&](const std::string& suffix) -> std::string {
            for (const auto& entry : custom_unit_scales) {
                std::string rest = suffix;
                int power = 1;
                if (!rest.empty() && rest[0] == 'a') { power = 2; rest.erase(0, 1); }
                else if (!rest.empty() && rest[0] == 'V') { power = 3; rest.erase(0, 1); }
                std::string prefix;
                for (const std::string& code : {"da", "d", "c", "m", "h", "k"})
                    if (rest.rfind(code, 0) == 0 && rest.substr(code.size()) == entry.first) { prefix = code; break; }
                if (rest != entry.first && prefix.empty()) continue;
                const std::string dim = power == 1 ? "length_dimension" : (power == 2 ? "area_dimension" : "volume_dimension");
                std::string factor = entry.second + ".base_factor";
                if (!prefix.empty()) factor = entry.second + ".factor(\"" + prefix + "\")";
                if (power == 2) factor = "(" + factor + " * " + factor + ")";
                if (power == 3) factor = "(" + factor + " * " + factor + " * " + factor + ")";
                const std::string base = power == 1 ? entry.first : (power == 2 ? "a" + entry.first : "V" + entry.first);
                return "cpercent::typed_unit<cpercent::" + dim + ">(" + factor + ", \"" + suffix + "\", \"" + base + "\")";
            }
            return "";
        };

        while (std::getline(ss, line)) {
            ++source_line_number;
            c_out << "#line " << source_line_number << " \"" << cpp_string_literal(source_name) << "\"\n";
            if (line.find("<iostream>") != std::string::npos ||
                line.find("<clrnsp>") != std::string::npos ||
                line.find("<logicgate>") != std::string::npos ||
                line.find("<filesys>") != std::string::npos ||
                line.find("<fstream>") != std::string::npos ||
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
            line = std::regex_replace(line, std::regex(R"((\d+)\s*(?:day|days)\b)"), "$1d");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*(?:month|months)\b)"), "chronos::months($1)");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*(?:year|years)\b)"), "chronos::years($1)");
            line = std::regex_replace(line, std::regex(R"((\d+)\s*decades?\b)"), "chronos::years(($1) * 10)");
            line = std::regex_replace(line, std::regex(R"(chronos::timer\s+([A-Za-z_]\w*)\s*=\s*)"), "chronos::timer $1 = ");

            std::smatch scale_match;
            if (std::regex_search(line, scale_match, std::regex(R"(std::scale\s+([A-Za-z_]\w*)\s*\{)"))) {
                active_scale = scale_match[1];
                in_scale_block = true;
                c_out << "cpercent::scale_definition " << active_scale << ";\n";
                continue;
            }
            if (line.find("std::scale_character_set") != std::string::npos) {
                in_scale_character_set = true;
                continue;
            }
            if (in_scale_character_set) {
                if (line.find('}') != std::string::npos) { in_scale_character_set = false; continue; }
                std::smatch prefix_match;
                if (!active_scale.empty() && std::regex_search(line, prefix_match,
                    std::regex(R"(([+-]\s*\d+)\s*\.?\s*\"([^\"]+)\")"))) {
                    std::string marker = std::regex_replace(prefix_match[1].str(), std::regex(R"(\s+)"), "");
                    c_out << "static const bool _cpercent_scale_init_" << scale_init_index++ << " = [] { "
                          << active_scale << ".add_prefix(" << marker << ", \"" << prefix_match[2].str() << "\"); return true; }();\n";
                }
                continue;
            }
            if (line.find("multiple_smultiple.range") != std::string::npos) {
                std::smatch range_match;
                if (!active_scale.empty() && std::regex_search(line, range_match, std::regex(R"(=\s*(\d+))")))
                    c_out << "static const bool _cpercent_scale_init_" << scale_init_index++ << " = [] { "
                          << active_scale << ".range = " << range_match[1] << "; return true; }();\n";
                continue;
            }
            if (line.find(".data =") != std::string::npos || line.find("scale.done;") != std::string::npos) continue;
            if (in_scale_block && line.find('}') != std::string::npos) { in_scale_block = false; continue; }

            // Area and volume use the dimensions built into the unit types.
            if (line.find("std::a_of.formula") != std::string::npos ||
                line.find("std::V_of.formula") != std::string::npos) continue;
            std::smatch unit_match;
            if (std::regex_search(line, unit_match, custom_unit_decl)) {
                const std::string scale_name = unit_match[1];
                const std::string unit_name = unit_match[2];
                const std::string symbol = unit_match[3];
                if (scale_name == "default_scale" || scale_name == "square_scale" || scale_name == "cubic_scale") {
                    std::string generated = symbol;
                    if (scale_name == "square_scale") generated = "a" + symbol;
                    if (scale_name == "cubic_scale") generated = "V" + symbol;
                    c_out << "auto " << unit_name << " = cpercent::units::" << generated << ";\n";
                } else {
                    c_out << "auto " << unit_name << " = cpercent::typed_unit<cpercent::length_dimension>(" << scale_name << ".base_factor, \"" << symbol << "\", \"" << symbol << "\");\n";
                }
                continue;
            }
            const std::regex result_target(R"(std::conversion_onOperation\.result\s*\(\s*into\s*:\s*([A-Za-z_]\w*)\s*\)\s*;)");
            const std::regex convert_target(R"(\.convert\s*\(\s*into\s*:\s*([A-Za-z_]\w*)\s*\))");
            std::smatch target_match;
            if (std::regex_search(line, target_match, result_target)) {
                const std::string expr = custom_unit_expression(target_match[1]);
                if (!expr.empty()) line = std::regex_replace(line, result_target, "cpercent::conversion_onOperation.result(" + expr + ");");
                else line = std::regex_replace(line, result_target, "cpercent::conversion_onOperation.result(cpercent::units::$1);");
            }
            if (std::regex_search(line, target_match, convert_target)) {
                const std::string expr = custom_unit_expression(target_match[1]);
                if (!expr.empty()) line = std::regex_replace(line, convert_target, ".as(" + expr + ")");
                else line = std::regex_replace(line, convert_target, ".as(cpercent::units::$1)");
            }

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
                for (const auto& custom : custom_unit_scales) {
                    std::vector<std::string> suffixes = {"V" + custom.first, "a" + custom.first, custom.first};
                    for (const std::string& code : {"da", "d", "c", "m", "h", "k"}) {
                        suffixes.push_back("V" + code + custom.first);
                        suffixes.push_back("a" + code + custom.first);
                        suffixes.push_back(code + custom.first);
                    }
                    std::sort(suffixes.begin(), suffixes.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });
                    for (const auto& suffix : suffixes) {
                        const std::string expr = custom_unit_expression(suffix);
                        if (expr.empty()) continue;
                        const std::string dim = expr.find("length_dimension") != std::string::npos ? "length_dimension" :
                            (expr.find("area_dimension") != std::string::npos ? "area_dimension" : "volume_dimension");
                        line = std::regex_replace(line, std::regex(R"((\b\d+(?:\.\d+)?))" + suffix + R"(\b)"),
                            "cpercent::make_quantity<cpercent::" + dim + ">($1L, " + expr + ")");
                    }
                }
                for (const auto& entry : unit_literals) {
                    std::regex literal(R"((\b\d+(?:\.\d+)?))" + std::string(entry.suffix) + R"(\b)");
                    line = std::regex_replace(line, literal,
                        std::string("cpercent::make_quantity<cpercent::") + entry.dimension + ">($1L, cpercent::typed_unit<cpercent::" + entry.dimension + ">(" + entry.factor + ", \"" + entry.symbol + "\"))");
                }
                line = std::regex_replace(line, std::regex(R"(\b(?:num|int|dec|double)\s+(\w+)\s*=\s*(cpercent::make_quantity))"), "auto $1 = $2");
            }

            if (line.find("main") != std::string::npos) user_has_main = true;

            // Translate common type names.
            line = std::regex_replace(line, std::regex("\\bnum main\\b"), "int main");
            line = std::regex_replace(line, std::regex("\\bconstvar\\b"), "const");
            line = std::regex_replace(line, std::regex("\\bnum\\b"), "int");
            line = std::regex_replace(line, std::regex("\\bdec\\b"), "double");
            line = std::regex_replace(line, std::regex("\\bcstring\\b"), "std::string");

            // Translate input and output names.
            line = std::regex_replace(line, std::regex("\\boutput\\b"), "std::cout");
            line = std::regex_replace(line,
                std::regex(R"((^|[^A-Za-z0-9_:])getline\s*\(\s*([A-Za-z_]\w*)\s*\))"),
                "$1std::getline(std::cin, $2)");
            line = std::regex_replace(line,
                std::regex(R"((^|[^A-Za-z0-9_:])getline(\s*\())"),
                "$1std::getline$2");
            line = std::regex_replace(line, std::regex("\\binput\\b"), "std::cin");

            // Translate C% logic words.
            line = std::regex_replace(line, std::regex("\\bANDL\\b"), "&&");
            line = std::regex_replace(line, std::regex("\\bORL\\b"), "||");
            line = std::regex_replace(line, std::regex("\\bNOTL\\b"), "!");
            line = std::regex_replace(line, std::regex("\\bXORL\\b"), "^");

            // Translate C% boolean words.
            line = std::regex_replace(line, std::regex("\\bfact\\b"), "bool");
            line = std::regex_replace(line, std::regex("\\bTrue\\b"), "true");
            line = std::regex_replace(line, std::regex("\\bFalse\\b"), "false");
            line = std::regex_replace(line, std::regex("\\bT\\b"), "true");
            line = std::regex_replace(line, std::regex("\\bF\\b"), "false");

            // Translate math constants and reference markers.
            line = std::regex_replace(line, std::regex("#O_PI"), "O_PI");
            line = std::regex_replace(line, std::regex("#O_EULER"), "O_EULER");
            line = std::regex_replace(line, std::regex("@"), "&");

            c_out << line << "\n";
        }

        if (!user_has_main) {
            c_out << "#line 1 \"<generated by lplay>\"\n";
            c_out << "\nint main() { return 0; }\n";
        }

        return c_out.str();
    }
};

// Command-line driver.
namespace {
std::string shell_quote(const std::string& value) {
    std::string quoted = "'";
    for (char c : value) {
        if (c == '\'') quoted += "'\\''";
        else quoted += c;
    }
    quoted += "'";
    return quoted;
}

void print_usage(std::ostream& out) {
    out << "lplay C% interpreter v2.0\n"
        << "Usage: lplay [options] <source-file>\n\n"
        << "Translate C% to C++, compile it, and run the result.\n\n"
        << "Recommended source extensions: .cpr and .cmod. Other extensions are accepted.\n\n"
        << "Options:\n"
        << "  -o, --output <path>       Write output here (default: input name)\n"
        << "  -c, --compiler <command>  C++ compiler (default: $CXX or clang++)\n"
        << "  -p, --transpile-only      Write generated C++ and stop\n"
        << "  -S                        Write generated assembly and stop\n"
        << "  -h, --help                Show this help\n"
        << "  -v, --version             Show version\n\n"
        << "Compiler profiles for other languages are managed by lplayprofile.\n";
}

std::string output_stem(const std::filesystem::path& input) {
    std::string stem = input.stem().string();
    return stem.empty() ? "a.out" : stem;
}
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(std::cout);
        return 0;
    }

    const std::string first = argv[1];
    if (first == "-h" || first == "--help" || first == "help") {
        print_usage(std::cout);
        return 0;
    }
    if (first == "-v" || first == "--version") {
        std::cout << "lplay C% interpreter v2.0\n";
        return 0;
    }

    std::error_code fs_error;

    std::filesystem::path input_file;
    std::filesystem::path output_file;
    bool output_was_set = false;
    bool transpile_only = false;
    bool assembly_only = false;
    const char* cxx_env = std::getenv("CXX");
    std::string compiler = (cxx_env && *cxx_env) ? cxx_env : "clang++";

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << "lplay: missing path after " << arg << "\n";
                return 2;
            }
            output_file = argv[++i];
            output_was_set = true;
        } else if (arg == "-c" || arg == "--compiler") {
            if (i + 1 >= argc) {
                std::cerr << "lplay: missing compiler command after " << arg << "\n";
                return 2;
            }
            compiler = argv[++i];
        } else if (arg == "-p" || arg == "--transpile-only") {
            transpile_only = true;
        } else if (arg == "-S") {
            assembly_only = true;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "lplay: unknown option: " << arg << "\n";
            return 2;
        } else if (input_file.empty()) {
            input_file = arg;
        } else {
            std::cerr << "lplay: more than one input file was provided\n";
            return 2;
        }
    }

    if (input_file.empty()) {
        std::cerr << "lplay: no input file provided\n";
        print_usage(std::cerr);
        return 2;
    }
    if (transpile_only && assembly_only) {
        std::cerr << "lplay: -p/--transpile-only and -S are mutually exclusive\n";
        return 2;
    }
    if (!output_was_set) {
        output_file = output_stem(input_file);
        if (transpile_only) output_file += ".cpp";
        else if (assembly_only) output_file += ".s";
    }

    const std::filesystem::path absolute_input = std::filesystem::absolute(input_file, fs_error).lexically_normal();
    if (fs_error || !std::filesystem::is_regular_file(absolute_input, fs_error) || fs_error) {
        std::cerr << "lplay: input file not found: " << input_file << "\n";
        return 1;
    }
    const std::filesystem::path absolute_output = std::filesystem::absolute(output_file, fs_error).lexically_normal();
    if (fs_error || absolute_input == absolute_output) {
        std::cerr << "lplay: output path must not overwrite the source file\n";
        return 2;
    }

    std::ifstream src_stream(absolute_input);
    if (!src_stream) {
        std::cerr << "lplay: cannot open input file: " << absolute_input << "\n";
        return 1;
    }
    const std::string source((std::istreambuf_iterator<char>(src_stream)), std::istreambuf_iterator<char>());
    if (src_stream.bad()) {
        std::cerr << "lplay: error while reading input file: " << absolute_input << "\n";
        return 1;
    }

    CPercentTranspiler transpiler;
    const std::string translated = transpiler.transpile(source, absolute_input.string());

    if (transpile_only) {
        std::ofstream cpp_out(absolute_output);
        if (!cpp_out || !(cpp_out << translated)) {
            std::cerr << "lplay: cannot write generated C++ to " << absolute_output << "\n";
            return 1;
        }
        std::cout << "lplay: generated C++ -> " << absolute_output << "\n";
        return 0;
    }

    std::filesystem::path temp_dir;
    const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    bool created_temp = false;
    for (unsigned attempt = 0; attempt < 100; ++attempt) {
        temp_dir = std::filesystem::temp_directory_path() /
            ("lplay-build-" + std::to_string(seed) + "-" + std::to_string(attempt));
        fs_error.clear();
        if (std::filesystem::create_directory(temp_dir, fs_error)) {
            created_temp = true;
            break;
        }
        if (fs_error && fs_error != std::errc::file_exists) break;
    }
    if (!created_temp) {
        std::cerr << "lplay: could not create a temporary build directory\n";
        return 1;
    }

    const auto generated_cpp = temp_dir / "generated.cpp";
    {
        std::ofstream cpp_out(generated_cpp);
        if (!cpp_out || !(cpp_out << translated)) {
            std::filesystem::remove_all(temp_dir, fs_error);
            std::cerr << "lplay: cannot write temporary C++ source\n";
            return 1;
        }
    }

    std::string compile_command = compiler + " -std=c++17" + (assembly_only ? " -S" : "") +
        " -I " + shell_quote(absolute_input.parent_path().string()) +
        " " + shell_quote(generated_cpp.string()) + " -o " + shell_quote(absolute_output.string());
    std::cout << "lplay: " << (assembly_only ? "generating assembly " : "compiling ")
              << absolute_input << " -> " << absolute_output << "\n" << std::flush;
    const int build_result = std::system(compile_command.c_str());
    std::filesystem::remove_all(temp_dir, fs_error);
    if (build_result != 0) {
        std::cerr << "lplay: build failed\n";
        return 1;
    }

    std::cout << "lplay: " << (assembly_only ? "generated " : "built ") << absolute_output << "\n";
    if (!assembly_only) {
        std::cout << "lplay: running " << absolute_output << "\n" << std::flush;
        const int run_result = std::system(shell_quote(absolute_output.string()).c_str());
        if (run_result != 0) {
            std::cerr << "lplay: program exited with an error\n";
            return 1;
        }
    }
    return 0;
}
