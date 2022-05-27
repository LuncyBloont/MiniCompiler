#include "SemanticAnalyzer.h"

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

        size_t pos = line.find(TRANS_SIGN);
        pos = (pos == std::string::npos ? line.size() : pos);
        size_t pos2 = line.find(OPS_SIGN);
        pos2 = (pos2 == std::string::npos ? line.size() : pos2);
        std::string left = line.substr(0, pos - 0);
        std::string right = "";
        std::string ops = "";
        if (pos < line.size()) {
            right = line.substr(pos + TRANS_SIGN_SIZE, pos2 - (pos + TRANS_SIGN_SIZE));
            if (pos2 < line.size()) {
                ops = line.substr(pos2 + OPS_SIGN_SIZE, line.size() - (pos2 + OPS_SIGN_SIZE));
            }
        }
        delblank(left);
        delblank(right);
        delblank(ops);

        PNode node;
        node.left = left;
        node.right = right;
        node.mprog = ops;
        nodes.push_back(node);
        stk[now] = &(*nodes.rbegin());
        stk[now - 1]->children.push_back(stk[now]);
    }

    while (!tables.empty()) { tables.pop_back(); }
    analyze(stk[-1]);


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
    PERR("CALL %s(...%llu):\n", func.data(), args.size());
    const auto mkStr = [&](auto&& self, const PNode* n, std::string& str) -> void {
        if (G.at(n->left).type == GrammarAnalyzer::Type::Terminal) {
            str.append(n->left);
        }
        for (const auto& c : n->children) {
            self(self, c, str);
        }
    };

    const auto getNode = [](const PNode* parent, int32_t index) {
        auto it = parent->children.begin();
        for (int32_t i = 0; i < index; i++) { it++; }
        return *it;
    };

    if (start) {
        if (func == "TABLE") {
            tables.push_back({});
            PERR("new Table.\n");

        } else if (func == "DEFC") {
            std::string name;
            mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
            std::string val;
            mkStr(mkStr, getNode(node, std::atoi(args[1].data())), val);
            Sym sym{ name, Sym::Type::Const, std::atoi(val.data()), tables.back().nextAddr(), 4 };
            tables.back().push(sym);
            PERR("define const %s = %d\n", name.data(), std::atoi(val.data()));

        } else if (func == "DEF") {
            std::string name;
            mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
            Sym sym{ name, Sym::Type::Var, 0, tables.back().nextAddr(), 4 };
            tables.back().push(sym);
            PERR("define var %s\n", name.data());

        } else if (func == "DEFF") {
            std::string name;
            mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
            Sym sym{ name, Sym::Type::Func, 0, tables.back().nextAddr(), 4 };
            tables.back().push(sym);
            PERR("define function %s\n", name.data());

        } else if (func == "TESTNDEF") {
            std::string name;
            mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
            if (tables.back().exist(name)) {
                PERR("Symbol %s is defined!\n", name.data());
                exit(-1);
            }
            PERR("Make sure %s is undefined.\n", name.data());

        } else if (func == "TESTDEF") {
            std::string name;
            mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
            bool defined = false;
            for (auto it = tables.rbegin(); it != tables.rend(); it++) {
                if (it->exist(name)) {
                    defined = true;
                }
            }
            if (!defined) {
                PERR("Symbol %s is not defined!\n", name.data());
                exit(-1);
            }
            PERR("Make sure %s is defined.\n", name.data());

        } else if (func == "SET") {
            std::string name;
            mkStr(mkStr, getNode(node, std::atoi(args[0].data())), name);
            std::string val;
            mkStr(mkStr, getNode(node, std::atoi(args[1].data())), val);
            PERR("(const): set %s = %d\n", name.data(), std::atoi(val.data()));
            bool exist = false;
            for (auto it = tables.rbegin(); it != tables.rend(); it++) {
                if (it->exist(name)) {
                    exist = true;
                    it->at(name).value = std::atoi(val.data());
                }
            }
            if (!exist) {
                PERR("Symbol %s is not defined!\n", name.data());
                exit(-1);
            }

        } else if (func == "POPTABLE") {
            PERR("Table:\n");
            for (const auto& c : tables.back().table) {
                PERR("name[%s] type[%s], value[%d], addr[%d], size[%d]\n",
                    c.name.data(), (c.type == Sym::Type::Const ? "const" : (c.type == Sym::Type::Var ? "var" : "function")), c.value, c.addr, c.size
                );
            }
            PERR("Pop!\n");
            tables.pop_back();

        } else if (func == "TESTM") {
            std::string name;
            mkStr(mkStr, getNode(node, std::stoi(args[0].data())), name);
            for (auto it = tables.rbegin(); it != tables.rend(); it++) {
                if (it->exist(name)) {
                    if (it->at(name).type == Sym::Type::Const) {
                        PERR("Modify a const Symbol.\n");
                        exit(-1);
                    }
                    break;
                }
            }
            PERR("Is %s const or function?\n", name.data());

        } else if (true) {
            PERR("Unknown func %s at %s.\n", func.data(), (start ? "START" : "END"));
            exit(-1);
        }
    } else {

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
