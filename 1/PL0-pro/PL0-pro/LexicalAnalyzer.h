#pragma once
#include <ostream>
#include <istream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include "debug.h"

class PL0;

class LexicalAnalyzer {
	struct LexicalError{
		size_t pos = 0;
		size_t lineStart = 0;
		bool error = false;
	};
public:
	LexicalAnalyzer(const std::map<std::string, std::string>& keywords, 
		const std::map<std::string, std::string>& operators, 
		const std::map<std::string, std::string>& boundary,
		const std::set<std::string>& numbers, std::string& numberSYM, std::string& idSYM);
	void start(std::istream& input, std::ostream& output, const PL0& pl0);

private:
	std::string wordBuffer;
	std::string buffer;
	size_t wordLPos = 0;
	size_t wordPos = 0;
	LexicalError error;
	const std::map<std::string, std::string>& keywords;
	const std::map<std::string, std::string>& operators;
	const std::map<std::string, std::string>& boundary;
	const std::string& numberSYM;
	const std::set<std::string>& numbers;
	const std::string idSYM;
	const std::string errSYM = "ERROR";
	std::vector<size_t> lineNumber;
	std::vector<size_t> lineStart;
	std::vector<size_t> lineSize;

	std::string getSYM();
	void delSpace();
	void analyze();
};