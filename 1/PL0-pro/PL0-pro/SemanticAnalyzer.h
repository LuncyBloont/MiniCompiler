#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <cstdint>
#include "GrammarAnalyzer.h"
#include "PL0.h"
#include "split.h"

#define TMP_ADDR 0
#define CODE_ADDR 0

struct Code4 {
	std::string op;
	std::vector<std::string > p;
	Code4(std::string op = "_", std::string p0 = "0", std::string p1 = "0", std::string p2 = "0");
	operator std::string() const;
};

class SemanticAnalyzer {
public:
	struct PNode {
		std::string left;
		std::string right;
		std::list<PNode*> children;
		std::string mprog;
		int line = -1;
	};

	struct Sym {
		enum class Type { Const, Var, Func };
		std::string name;
		Type type = Type::Const;
		int32_t value = 0;
		uint32_t addr = 0;
		uint32_t size = 4;
	};

	struct AbsSym {
		Sym sym;
		uint32_t level = 0;
	};
  
	struct Table {
		std::vector<Sym> table;
		void push(const Sym& sym);
		Sym& at(std::string name);
		bool exist(std::string name);
		const Sym& top();
		uint32_t nextAddr();
	};

	SemanticAnalyzer(const std::map<std::string, GrammarAnalyzer::V>& G);
	void start(std::istream& input, std::ostream& output);
	void analyze(const PNode* root, int sp = 0);

	void runMiniProg(const PNode* node, const std::string& code, bool start);
	void call(const PNode* node, const std::string& func, const std::vector<std::string>& args, bool start);
	
private:
	const std::map<std::string, GrammarAnalyzer::V>& G;
	std::vector<Table> tables;
	std::vector<Code4> code4;

	std::vector<size_t> codeaddr;
	std::vector<size_t> tableaddr; // SAVE code addr will need table capacity. INT capacity 0 0; capacity is unknown untill poptable
	std::vector<uint32_t> preaddr;
	uint32_t tmpIndice = 0;
};

