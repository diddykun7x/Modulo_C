#include <iostream>

class student{
    std::string funame(std::string fn, std::string mn, std::string ln) {
        return fn + mn + ln;
    }
    public:
        std::string fn; std::string mn; std::string ln;
};
int main() {
    student d;
    d.fn = "a";
    d.mn = "b";
    d.ln = "c";
    
    std::cout << d.fn << " " << d.mn << " " << d.ln << '\n';
}