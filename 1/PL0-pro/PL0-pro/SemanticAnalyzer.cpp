#include "SemanticAnalyzer.h"
#include "Code4Filter.h"

template <typename...Args>
std::string __sem_format(const char* fmt, const Args&... args) {
    char buf[4096];
    sprintf_s(buf, fmt, args...);
    return std::string(buf);
}

SemanticAnalyzer::SemanticAnalyzer(const std::map<std::string, GrammarAnalyzer::V>& G) : G(G) {}

void SemanticAnalyzer::start(std::istream& input, std::ostream& output) {
    const auto tabnum = [](std::string s) -> uint32_t {
        uint32_t n = 0;
        for (const auto& c : s) { if (c != '\t' && c != ' ') { break; } n++; }
        return n;
    };

    std::map<uint32_t, PNode*> stk;
    std::list<PNode> nodes;
    std::string linestr;
    PNode root = { "tree ROOT", { "body" }, {}, "" };
    stk[-1] = &root;

    while (true) {
        std::string line;
        std::getline(input, line);
        if (input.eof()) { break; }
        uint32_t now = tabnum(line);

        size_t numPos = line.find(" ");
        size_t pos = line.find(TRANS_SIGN);
        pos = (pos == std::string::npos ? line.size() : pos);
        size_t pos2 = line.find(OPS_SIGN);
        pos2 = (pos2 == std::string::npos ? line.size() : pos2);
        std::string lineNum = line.substr(0, numPos);
        std::string left = line.substr(numPos, pos - numPos);
        std::string right = "";
        std::string ops = "";
        if (pos < line.size()) {
            right = line.substr(pos + TRANS_SIGN_SIZE, pos2 - (pos + TRANS_SIGN_SIZE));
            if (pos2 < line.size()) {
                ops = line.substr(pos2 + OPS_SIGN_SIZE, line.size() - (pos2 + OPS_SIGN_SIZE));
            }
        }
        delblank(lineNum);
        delblank(left);
        delblank(right);
        delblank(ops);

        PERR("[%s] [%s] [%s] [%s]\n", lineNum.data(), left.data(), right.data(), ops.data());

        PNode node;
        node.left = left;
        node.right = right;
        node.mprog = ops;
        node.line = std::atoi(lineNum.data());
        nodes.push_back(node);
        stk[now] = &(*nodes.rbegin());
        stk[now - 1]->children.push_back(stk[now]);
    }

    while (!tables.empty()) { tables.pop_back(); }
    code4.clear();
    code4.push_back({ "ERROR" });
    codeaddr.clear();

    analyze(stk[-1]);

    uint32_t addr = 0;
    for (const auto& c : code4) {
        PERR("%u\t: %s\n", addr, std::string(c).data());
        output << std::string(c) << "\n";
        addr += 1;
    }

    std::vector<Ins> codes;
    Code4Filter::compile(codes, code4);
    addr = 0;
    PERR("\nFINAL:\n");
    for (const auto& c : codes) {
        PERR("%u\t: %s\n", addr, (c.moperator + " " + c.levelOffset + " " + c.offset).data());
        addr += 1;
    }
}

void SemanticAnalyzer::runMiniProg(const PNode* node, const std::string& code, bool start) {
    if (code.size() == 0) return;
    std::vector<std::string> progstr = ssplit(code, " ");
    for (const auto& c : progstr) {
        std::string func = c;
        std::vector<std::string> args;
        if (c.find("(") != c.npos) {
            size_t start = c.find("(") + strlen("(");
            size_t end = c.find(")");
            std::string astr = c.substr(start, end - start);
            args = ssplit(astr, ",");
            func = c.substr(0, c.find("("));
        }
        call(node, func, args, start);
    }
}

void SemanticAnalyzer::analyze(const PNode* root, int sp) {
    const auto psp = [](int s) { while (s > 0) { PERR(" "); s--; } };
    if (G.find(root->left) != G.end()) {
        psp(sp); PERR("%s [ %s ]{\n", root->left.data(), root->mprog.data());

        runMiniProg(root, root->mprog, true);

        for (const auto& c : root->children) {
            analyze(c, sp + 4);
        }

        runMiniProg(root, root->mprog, false);

        psp(sp); PERR("} %s\n", root->left.data());
    } else {
        psp(sp); PERR("<<%s>>\n", root->left.data());
        for (const auto& c : root->children) {
            analyze(c, sp + 4);
        }
    }
}

void SemanticAnalyzer::call(const PNode* node, const std::string& func, const std::vector<std::string>& args, bool start) {
    PERR("%s meybe CALL %s(...%llu):\n", start ? ">> at start " : ">> at end ", func.data(), args.size());
    const auto mkStr = [&] (auto&& self, const PNode* n, std::string& str) -> void {
        if (G.at(n->left).type == GrammarAnalyzer::Type::Terminal) {
            str.append(n->left);
        }
        for (const auto& c : n->children) {
            self(self, c, str);
        }
    };

    const auto getNode = [] (const PNode* parent, int32_t index) {
        auto it = parent->children.begin();
        for (int32_t i = 0; i < index; i++) { it++; }
        return *it;
    };

    const auto top = [&] (uint32_t off = 0) {
        return "$" + std::to_string(tmpIndice - off);
    };

    const auto getSYM = [&] (const std::string& name) {
        AbsSym res;
        res.sym.addr = 0;
        res.level = UINT32_MAX;
        uint32_t l = 0;
        for (auto it = tables.rbegin(); it != tables.rend(); it++) {
            for (auto v = it->table.begin(); v != it->table.end(); v++) {
                if (v->name == name) {
                    res.sym = *v;
                    res.level = l;
                    return res;
                }
            }
            l++;
        }
        return res;
    };

    const auto offsetName = [&] (const std::string& name) {
        AbsSym sym = getSYM(name);
        return "@" + std::to_string(sym.level) + ":" + std::to_string(sym.sym.addr);
    };

    char stage = func[0];
    uint32_t offset = 0;
    if (stage == '$') {
        offset = 1;
        if (start) return;
    } else {
        if (!start) return;
    }
    if (stage == '*') {
        offset = 1;
    }

    std::string fcname = func.substr(offset, func.size() - offset);

    if (fcname == "TABLE") {
        tables.push_back({});
        PERR("new Table.\n");
        tableaddr.push_back(code4.size());
        code4.push_back({"INT", "--toComplete--"});

    } else if (fcname == "DEFC") {
        std::string name;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
        std::string val;
        mkStr(mkStr, getNode(node, std::atoi(args[1].data())), val);
        Sym sym{ name, Sym::Type::Const, std::atoi(val.data()), tables.back().nextAddr(), 0 };
        tables.back().push(sym);
        PERR("define const %s = %d\n", name.data(), std::atoi(val.data()));

    } else if (fcname == "DEF") {
        std::string name;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
        Sym sym{ name, Sym::Type::Var, 0, tables.back().nextAddr(), 4 };
        tables.back().push(sym);
        PERR("define var %s\n", name.data());

    } else if (fcname == "DEFF") {
        std::string name;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
        Sym sym{ name, Sym::Type::Func, 0, tables.back().nextAddr(), 0 };
        sym.value = static_cast<int32_t>(code4.size());
        tables.back().push(sym);
        PERR("define function %s\n", name.data());

    } else if (fcname == "TESTNDEF") {
        std::string name;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
        if (tables.back().exist(name)) {
            PERR("Symbol %s is defined! line %d\n", name.data(), node->line);
            exit(-1);
        }
        PERR("Make sure %s is undefined.\n", name.data());

    } else if (fcname == "TESTDEF") {
        std::string name;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
        if (getSYM(name).level == UINT32_MAX) {
            PERR("Symbol %s is not defined! line %d\n", name.data(), node->line);
            exit(-1);
        }
        PERR("Make sure %s is defined.\n", name.data());

    } else if (fcname == "TESTM") {
        std::string name;
        mkStr(mkStr, getNode(node, std::stoi(args[0].data())), name);
        AbsSym sym = getSYM(name);
        if (sym.sym.type == Sym::Type::Const || sym.sym.type == Sym::Type::Func) {
            PERR("Wrong... %s is const or function... line %d\n", name.data(), node->line);
            exit(-1);
        }
        PERR("Is %s const or function?\n", name.data());

    } else if (fcname == "MKMEM") {
        code4.push_back({ "P" });
        tmpIndice = TMP_ADDR;

    } else if (fcname == "RG") {
        // code4.push_back({ "RG" });
        tmpIndice += 1;

    } else if (fcname == "WRG") {
        std::string name;
        mkStr(mkStr, getNode(node, atoi(args[0].data())), name);
        AbsSym m = getSYM(name);
        if (m.sym.type == Sym::Type::Const) {
            code4.push_back({ "WI", top(), std::to_string(m.sym.value) });
        } else {
            code4.push_back({ "W", top(), offsetName(name) });
        }

    } else if (fcname == "WRGI") {
        std::string num;
        mkStr(mkStr, getNode(node, atoi(args[0].data())), num);
        code4.push_back({ "WI", top(), num });

    } else if (fcname == "CODEADDR") {
        // code4.push_back({ "#---- push empty addr to code addr stack" });
        codeaddr.push_back(code4.size());

    } else if (fcname == "RESPC") {
        // Do nothing...

    } else if (fcname == "SET") {
        std::string vname;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), vname);
        code4.push_back({ "SET", offsetName(vname), top() });

    } else if (fcname == "POPRG") {
        // code4.push_back({ "PRG" });
        tmpIndice -= 1;

    } else if (fcname == "CMP") {
        std::string op;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), op);
        code4.push_back({ op, top(2), top(1), top() });

    } else if (fcname == "ODD") {
        code4.push_back({ "ODD", top(1), top() });

    } else if (fcname == "INVRG") {
        code4.push_back({ "*", top(), top(), "-1" });

    } else if (fcname == "OP") {
        std::string op;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), op);
        code4.push_back({ op, top(1), top(1), top() });

    } else if (fcname == "WADRRTOCODEADDR") {
        // code4.push_back({ "#----- write this addr to codeaddrstack" });
        uint32_t offset = std::atoi(args[0].data());
        if (codeaddr.size() == 0) {
            PERR("code addr stack is empty. line %d\n", node->line);
            exit(-1);
        }
        PERR("code addr stack top %llu, code4 size %llu\n", codeaddr.back(), code4.size());
        code4[codeaddr.back()].p[0] = std::to_string(code4.size() + offset);

    } else if (fcname == "POPCODEADRR") {
        // code4.push_back({ "#----- pop code addr stack" });
        codeaddr.pop_back();

    } else if (fcname == "JP") {
        code4.push_back({ "JP", "-pushedaddr-PRE-", top() });

    } else if (fcname == "JNP") {
        code4.push_back({ "JNP", "-pushedaddr-PRE-", top() });

    } else if (fcname == "POPTABLE") {
        PERR("Table:\n");
        for (const auto& c : tables.back().table) {
            PERR("name[%s] type[%s], value[%d], addr[%d], size[%d]\n",
                c.name.data(), (c.type == Sym::Type::Const ? "const" : (c.type == Sym::Type::Var ? "var" : "function")), c.value, c.addr, c.size
            );
        }
        PERR("Pop!\n");
        uint32_t size = 0;
        for (auto it = tables.back().table.begin(); it != tables.back().table.end(); it++) {
            size += it->size;
        }
        code4[tableaddr.back()].p[0] = std::to_string(size);
        tableaddr.pop_back();
        tables.pop_back();
        code4.push_back({"POPT"});

    } else if (fcname == "WTOPTOBTOP") {
        code4.push_back({ "WR", top(1), top() });

    } else if (fcname == "SAVEPC") {
        code4.push_back({ "PCPUSH" });

    } else if (fcname == "J") {
        code4.push_back({ "J", "-pushedaddr-" });

    } else if (fcname == "WCODEADDR") {
        std::string n;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), n);
        AbsSym fsym = getSYM(n);
        code4[codeaddr.back()].p[0] = std::to_string(fsym.sym.value);

    } else if (fcname == "JBACK") {
        code4.push_back({ "JBACKANDPOPPC" }); // and POP PC STACK

    } else if (fcname == "CREAD") {
        std::string v;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), v);
        code4.push_back({ "CR", offsetName(v) });

    } else if (fcname == "CWRITE") {
        std::string v;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), v);
        code4.push_back({ "CW", offsetName(v) });

    } else if (fcname == "WCODEADDRTOSYM") {
        std::string sym;
        mkStr(mkStr, getNode(node, std::atoi(args[0].data())), sym);
        for (auto it = tables.rbegin(); it != tables.rend(); it++) {
            if (it->exist(sym)) {
                it->at(sym).value = static_cast<uint32_t>(code4.size());
                break;
            }
        }

    } else if (fcname == "WPREADDR") {
        uint32_t offset = std::atoi(args[0].data());
        preaddr.push_back(static_cast<uint32_t>(code4.size() + offset));

    } else if (fcname == "USEPREADDR") {
        code4.back().p[0] = std::to_string(preaddr.back());

    } else if (fcname == "POPPREADRR") {
        preaddr.pop_back();

    } else {
        PERR("Unknown cmd [%s]...\n", fcname.data());
        exit(-1);
    }
}

void SemanticAnalyzer::Table::push(const Sym& sym) {
    table.push_back(sym);
}

SemanticAnalyzer::Sym& SemanticAnalyzer::Table::at(std::string name) {
    for (auto& v : table) {
        if (v.name == name) { return v; }
    }
    throw std::runtime_error("No such Symbol.");
}

bool SemanticAnalyzer::Table::exist(std::string name) {
    for (const auto& v : table) {
        if (v.name == name) { return true; }
    }
    return false;
}

const SemanticAnalyzer::Sym& SemanticAnalyzer::Table::top() {
    if (table.size() > 0) {
        return table.back();
    }
    throw std::runtime_error("Empty table!");
}

uint32_t SemanticAnalyzer::Table::nextAddr() {
    if (table.empty()) { return 0; }
    return top().addr + top().size;
}

Code4::Code4(std::string op, std::string p0, std::string p1, std::string p2): op(op),
                                                                              p({ p0, p1, p2 }) {
    
}

Code4::operator std::string() const {
    return op + " " + p[0] + " " + p[1] + " " + p[2];
}
