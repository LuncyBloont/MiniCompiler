#include "Code4Runner.h"

uint32_t Code4Runner::doIns(const Code4& code) {
    std::string ins = code.op;
    const auto num = [] (const std::string& s) { 
        return std::atoi(s.data());
    };

    if (ins == "RG") {

    }

    return 0;
}

void Code4Runner::run(const std::vector<Code4> codes) {
    uint32_t pc = 1;
    while (true) {
        
    }
}
