#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>

std::string get_ram_dir() {
    // macOS and Linux both use /tmp as a fast RAM-backed or temp directory
    return "/tmp/lplayxpr";
}

int main(int argc, char* argv[]) {
    std::string ram_dir = get_ram_dir();
    std::system(("mkdir -p " + ram_dir).c_str());

    if (argc < 2) {
        std::cout << "\033[1;36m=== LangPlay RAM Sandbox (lplayxpr) ===\033[0m\n";
        std::cout << "Usage:\n";
        std::cout << "  lplayxpr +file <file.(lpx|cmod|cx)> Create sandbox file in RAM\n";
        std::cout << "  lplayxpr comp <file.(lpx|cmod|cx)>  Compile & run directly in RAM\n";
        std::cout << "  lplayxpr promote <file> <dest>      Move RAM file to disk\n";
        std::cout << "  lplayxpr purge                      Wipe RAM sandbox\n";
        return 0;
    }

    std::string cmd = argv[1];

    // COMMAND: Create XPR File
    if (cmd == "+file" && argc >= 3) {
        std::string filename = argv[2];
        std::string filepath = ram_dir + "/" + filename;
        
        std::ofstream out(filepath);
        out << "// Experimental C% RAM File: " << filename << "\n";
        out << "<iostream>\n\nnum main() {\n    output << \"[XPR RAM Execution Successful]\\n\";\n    return 0;\n}\n";
        out.close();

        std::cout << "\033[1;32m[lplayxpr]\033[0m Created RAM file: " << filepath << "\n";
        return 0;
    }

    // COMMAND: Compile & Run in RAM
    if (cmd == "comp" && argc >= 3) {
        std::string filename = argv[2];
        std::string src_path = ram_dir + "/" + filename;
        std::string out_bin = ram_dir + "/xpr_bin";

        // Call base lplay engine targeting RAM paths
        std::string exec_cmd = "lplay " + src_path + " -o " + out_bin + " && " + out_bin;
        
        std::cout << "\033[1;33m[lplayxpr]\033[0m Compiling and executing in RAM...\n";
        return std::system(exec_cmd.c_str());
    }

    // COMMAND: Save to Disk
    if (cmd == "promote" && argc >= 4) {
        std::string filename = argv[2];
        std::string dest = argv[3];
        std::string src_path = ram_dir + "/" + filename;

        std::string cp_cmd = "cp " + src_path + " " + dest;
        if (std::system(cp_cmd.c_str()) == 0) {
            std::cout << "\033[1;32m[lplayxpr]\033[0m Saved " << filename << " to disk at " << dest << "\n";
        } else {
            std::cout << "\033[1;31m[lplayxpr] Error saving file.\033[0m\n";
        }
        return 0;
    }

    // COMMAND: Purge Sandbox
    if (cmd == "purge") {
        std::system(("rm -rf " + ram_dir + "/*").c_str());
        std::cout << "\033[1;32m[lplayxpr]\033[0m RAM sandbox purged!\n";
        return 0;
    }

    return 0;
}
