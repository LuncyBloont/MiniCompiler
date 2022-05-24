#pragma once
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include "LexicalAnalyzer.h"
#include "GrammarAnalyzer.h"
#include "SemanticAnalyzer.h"

#define OPS_SIGN "CSDO"
#define OPS_SIGN_SIZE strlen(OPS_SIGN)
#define TRANS_SIGN "==>"
#define TRANS_SIGN_SIZE strlen(TRANS_SIGN)
#define TRANS_SPLIT "|"
#define TREE_FILE_ENDL "\n"
#define COM_SIGN "//"

class PL0 {
public:
	PL0(const std::map<std::string, std::string>& keywords, std::map<std::string, std::string>& operators,
		std::map<std::string, std::string>& boundary, std::set<std::string>& numbers, 
		const std::string& numberName, const std::string& idname, 
		const std::map<std::string, GrammarAnalyzer::V>& G);
	PL0(const std::string keywords, const std::string operators, const std::string boundary, 
		const std::string numbers, const std::string idname, const std::string G);
	
	void run(uint32_t stage);
	
private:
	friend class GrammarAnalyzer;
	friend class LexicalAnalyzer;
	std::map<std::string, std::string> keywords;
	std::map<std::string, std::string> operators;
	std::map<std::string, std::string> boundary;
	std::set<std::string> numbers;
	std::string numberName;
	std::string idname;
	std::map<std::string, GrammarAnalyzer::V> G;
	std::string gStart;
	GrammarAnalyzer::TABLE table;
	std::map<std::pair<std::string, std::string>, std::string> rules;

	void makeLexicalTable(const std::string& keywords, const std::string& operators, const std::string& boundary,
		const std::string& numbers, const std::string& idname);
	void makeGrammarTable(const std::string& G);
	void makeFirstFollow();
	void makeTable();

	void outputHTML(const GrammarAnalyzer::StepResult& result);
	void outputTree(const GrammarAnalyzer::StepResult& result);
};

