#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <stack>
#include <set>
#include "split.h"
#include "debug.h"

class PL0;

class GrammarAnalyzer {
public:
	typedef std::map<std::pair<std::string, std::string>, std::list<std::string> > TABLE;
	static constexpr char G_EMPTY[] = "__0__";
	static constexpr char G_END[] = "__$__";
	enum class Type {
		Terminal,
		NonTerminal
	};
	struct V {
		Type type = Type::Terminal;
		std::string value;
		std::string SYM;
		std::list<std::list<std::string> > trans;
		std::set<std::string> gFirst;
		std::set<std::string> gFollow;
	};

	struct TreeNode {
		const V* v;
		std::list<TreeNode*> children;
		TreeNode* parent;
		
		TreeNode(const V* v = nullptr, TreeNode* const p = nullptr): v(v), parent(p) {}
		~TreeNode() {
			for (auto& node : children) {
				delete node;
			}
		}
		TreeNode& operator =(const TreeNode& other) {
			v = other.v;
			for (auto& node : other.children) {
				children.push_back(new TreeNode(*node));
			}
			return *this;
		}
		TreeNode(const TreeNode& other) {
			*this = other;
		}

		TreeNode* push(TreeNode* c) {
			c->parent = this;
			children.push_back(c);
			return c;
		}
	};

	struct StepResult {
		TreeNode process;
	};

public:
	GrammarAnalyzer(const std::map<std::string, V>& G, const std::string& gStart, const TABLE& table);
	StepResult start(std::istream& input, std::ostream& output, const PL0& pl0);

private:
	const std::map<std::string, V>& G;
	const std::string& gStart;
	const TABLE table;
};

