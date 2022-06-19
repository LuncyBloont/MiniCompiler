#include "Code4Filter.h"

std::map<uint32_t, uint32_t> Code4Filter::S;

void Code4Filter::init() {
    S.clear();
}

void Code4Filter::connect(uint32_t old, uint32_t newa) {
    if (S.find(old) != S.end()) {
        PERR("remapping address...\n");
        exit(-1);
    }
    S[old] = newa;
}

#define GEN(op, level, off) post.push_back({ #op, level, off })
#define CNT connect(static_cast<uint32_t>(i), static_cast<uint32_t>(post.size()))
#define ZERO "0"

void Code4Filter::genCode(std::vector<Ins>& post, const std::vector<Code4>& raw) {
    std::map<std::string, uint32_t> ops{};
    ops.insert({ "+", 1 });
    ops.insert({ "-", 2 });
    ops.insert({ "*", 3 });
    ops.insert({ "/", 4 });
    ops.insert({ ">", 5 });
    ops.insert({ "<", 6 });
    ops.insert({ ">=", 7 });
    ops.insert({ "<=", 8 });
    ops.insert({ "ODD", 9 });
    ops.insert({ "#", 10 });
    ops.insert({ "=", 11 });

    for (size_t i = 0; i < raw.size(); i++) {
        const auto num = [] (uint32_t a) { return std::to_string(a); };
        const auto getLevel = [&] (size_t pi) {
            uint32_t l, a;
            int _ = sscanf_s(raw[i].p[pi].data(), "@%u:%u", &l, &a);
            return l;
        };
        const auto getOffset = [&](size_t pi) {
            uint32_t l, a;
            int _ = sscanf_s(raw[i].p[pi].data(), "@%u:%u", &l, &a);
            return a;
        };

        if (raw[i].op == "ERROR") {
            CNT;
            GEN(INT, ZERO, ZERO);
        } else if (raw[i].op == "P") {
            // Do nothong...
        } else if (raw[i].op == "INT") {
            CNT;
            GEN(INT, ZERO, raw[i].p[0]);
        } else if (raw[i].op == "J") {
            CNT;
            GEN(JMP, ZERO, raw[i].p[0]);
        } else if (raw[i].op == "WI") {
            CNT;
            GEN(LIT, ZERO, raw[i].p[1]);
        } else if (raw[i].op == "SET") {
            CNT;
            GEN(STO, num(getLevel(0)), num(getOffset(0)));
        } else if (raw[i].op == "W") {
            CNT;
            GEN(LOD, num(getLevel(1)), num(getOffset(1)));
        } else if (raw[i].op == "+" ||
            raw[i].op == "-" ||
            raw[i].op == "*" ||
            raw[i].op == "/" ||
            raw[i].op == "ODD" ||
            raw[i].op == ">" ||
            raw[i].op == "<" ||
            raw[i].op == ">=" ||
            raw[i].op == "<=" ||
            raw[i].op == "=" ||
            raw[i].op == "#") {
            CNT;
            GEN(OPR, ZERO, num(ops.at(raw[i].op)));
        } else if (raw[i].op == "JP") {
            CNT;
            GEN(JPC, ZERO, raw[i].p[0]);
        } else if (raw[i].op == "JNP") {
            CNT;
            GEN(LIT, ZERO, "0");
            GEN(OPR, ZERO, num(ops.at("=")));
            GEN(JPC, ZERO, raw[i].p[0]);
        } else if (raw[i].op == "PCPUSH") {
            CNT;
            i++;
            GEN(CAL, ZERO, raw[i].p[0]);
        } else if (raw[i].op == "WR") {
            // Do nothing...
        } else if (raw[i].op == "JBACKANDPOPPC") {
            CNT;
            GEN(OPR, ZERO, ZERO);
        } else if (raw[i].op == "POPT") {
            GEN(CAL, ZERO, std::to_string(i));
            CNT;
            GEN(OPR, ZERO, ZERO);
        } else if (raw[i].op == "CR") {
            CNT;
            GEN(LIT, ZERO, num(getLevel(0)));
            GEN(LIT, ZERO, num(getOffset(0)));
            GEN(CAL, "1", num(0xFF0));
        } else if (raw[i].op == "CW") {
            CNT;
            GEN(LIT, ZERO, num(getLevel(0)));
            GEN(LIT, ZERO, num(getOffset(0)));
            GEN(CAL, "1", num(0xFF1));
        } else {
            PERR("Wrong case %s\n", raw[i].op.data());
            exit(-1);
        }
    }
}

void Code4Filter::complete(std::vector<Ins>& post, const std::vector<Code4>& raw) {
    for (size_t i = 0; i < post.size(); i++) {
        if (post[i].moperator == "JMP" || post[i].moperator == "JPC" || post[i].moperator == "CAL") {
            uint32_t target = std::atoi(post[i].offset.data());
            post[i].offset = std::to_string(post[i].levelOffset == "0" ? S.at(target) : target);
        }
    }
}

void Code4Filter::compile(std::vector<Ins>& post, const std::vector<Code4>& raw) {
    init();
    genCode(post, raw);
    complete(post, raw);
}
