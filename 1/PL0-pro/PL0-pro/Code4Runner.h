#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "SemanticAnalyzer.h"
#include "GrammarAnalyzer.h"

class Code4Runner {
private:
    uint32_t doIns(const Code4& code);

public:
    void run(const std::vector<Code4> codes);
};

