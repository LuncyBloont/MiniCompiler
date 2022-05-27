#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <cstdint>
#include "GrammarAnalyzer.h"
#include "PL0.h"
#include "split.h"

class SemanticAnalyzer {
public:
	struct PNode {
		std::string left;
		std::string right;
		std::list<PNode*> children;
		std::string mprog;
	};

	struct Sym {
		enum class Type { Const, Var, Func };
		std::string name;
		Type type;
		int32_t value = 0;
		uint32_t addr = 0;
		uint32_t size = 4;
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
	std::vector	<Table> tables;
};

