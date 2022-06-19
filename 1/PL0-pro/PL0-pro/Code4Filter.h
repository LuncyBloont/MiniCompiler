#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include "PL0.h"
#include "SemanticAnalyzer.h"

struct Ins {
    std::string moperator;
    std::string levelOffset;
    std::string offset;
};

class Code4Filter {
private:
    static std::map<uint32_t, uint32_t> S; // Address: old ==> new

    static void init();
    static void connect(uint32_t old, uint32_t newa);
    static void genCode(std::vector<Ins>& post, const std::vector<Code4>& raw);
    static void complete(std::vector<Ins>& post, const std::vector<Code4>& raw);

public:
    static void compile(std::vector<Ins>& post, const std::vector<Code4>& raw);
};

