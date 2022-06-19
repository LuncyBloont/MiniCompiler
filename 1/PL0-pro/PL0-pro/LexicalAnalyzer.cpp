#include "LexicalAnalyzer.h"
#include "PL0.h"

LexicalAnalyzer::LexicalAnalyzer(const std::map<std::string, std::string>& keywords,
		const std::map<std::string, std::string>& operators,
		const std::map<std::string, std::string>& boundary,
		const std::set<std::string>& numbers, std::string& numberSYM, std::string& idSYM):
	keywords(keywords), operators(operators), boundary(boundary), numberSYM(numberSYM), numbers(numbers),
	idSYM(idSYM) { }

void LexicalAnalyzer::start(std::istream& input, std::ostream& output, const PL0& pl0) {
	buffer.clear();
	wordBuffer.clear();
	while (true) {
		std::string line;
		std::getline(input, line);
		
		delblank(line);
		if (line.size() == 0 || line.find(COM_SIGN) == 0) { continue; }
		buffer.append(line);
		buffer.append("\n");

		if (input.eof()) { break; }
	}
	std::string raw = buffer;
	delSpace();
	std::string backup = buffer;
	analyze();
	if (error.error) {
		size_t lpos = lineStart[error.lineStart];
		size_t lsize = lineSize[error.lineStart];
		std::string errstr = raw.substr(lpos, lsize);
		std::cerr << "ERROR at line " << error.pos + 1 << ": " << errstr << "\n";
		exit(-1);
	} else {
		output << buffer;
	}
}

std::string LexicalAnalyzer::getSYM() {
	auto bit = boundary.find(wordBuffer);
	auto kit = keywords.find(wordBuffer);
	auto oit = operators.find(wordBuffer);
	std::string numtest;
	numtest.push_back(wordBuffer[0]);
	if (numbers.find(numtest) != numbers.end()) {
		for (const auto& c : wordBuffer) {
			numtest.clear();
			numtest.push_back(c);
			if (numbers.find(numtest) == numbers.end()) {
				error.error = true;
				error.pos = wordLPos;
				error.lineStart = wordPos;
				return errSYM;
			}
		}
		return numberSYM;
	} else if (bit != boundary.end()) {
		return bit->second;
	} else if (kit != keywords.end()) {
		return kit->second;
	} else if (oit != operators.end()) {
		return oit->second;
	}
	for (const auto& c : wordBuffer) {
		if (!std::isalnum(c)) {
			error.error = true;
			error.pos = wordLPos;
			error.lineStart = wordPos;
			return errSYM;
		}
	}
	return idSYM;
}

void LexicalAnalyzer::delSpace() {
	std::string raw = buffer;
	buffer.clear();
	lineNumber.clear();
	auto isSpace = [] (char c) {
		return c == ' ' || c == '\n';
	};

	size_t ln = 0;
	size_t lpos = 0;
	rsize_t lsize = 0;
	for (size_t i = 0; i < raw.size(); i++) {
		if (raw[i] == '\n') {
			ln++;
			lpos = i + 1;
			lsize = 0;
			for (size_t j = i + 1; j < raw.size(); j++) {
				if (raw[j] == '\n') {
					break;
				}
				lsize++;
			}
		}
		if ((buffer.size() == 0 || isSpace(buffer[buffer.size() - 1])) && isSpace(raw[i])) { continue; }
		if (raw[i] == '\n') {
			buffer.push_back(' ');
			lineNumber.push_back(ln);
			lineStart.push_back(lpos);
			lineSize.push_back(lsize);
			continue; 
		}
		buffer.push_back(raw[i]);
		lineNumber.push_back(ln);
		lineStart.push_back(lpos);
		lineSize.push_back(lsize);
	}
}

void LexicalAnalyzer::analyze() {
	std::string result;
	for (size_t i = 0; i < buffer.size(); i++) {
		wordLPos = lineNumber[i];
		wordPos = i;
		if (std::isalpha(buffer[i])) {
			do {
				wordBuffer.push_back(buffer[i]);
				i++;
			} while (std::isalnum(buffer[i]));
			i--;
			result.append(wordBuffer + " " + getSYM() + " " + std::to_string(wordLPos + 1) + "\n");
			wordBuffer.clear();
		} else if (buffer[i] == ' ') {
			continue;
		} else if (std::isdigit(buffer[i])) {
			do {
				wordBuffer.push_back(buffer[i]);
				i++;
			} while (std::isalnum(buffer[i]));
			i--;
			result.append(wordBuffer + " " + getSYM() + " " + std::to_string(wordLPos + 1) + "\n");
			wordBuffer.clear();
		} else {
			wordBuffer.push_back(buffer[i]);
			if (boundary.find(wordBuffer) != boundary.end()) {
				result.append(wordBuffer + " " + getSYM() + " " + std::to_string(wordLPos + 1) + "\n");
				wordBuffer.clear();
			} else {
				while (i < buffer.size() && operators.find(wordBuffer + buffer[i + 1]) != operators.end()) {
					wordBuffer.push_back(buffer[i + 1]);
					i++;
				}
				result.append(wordBuffer + " " + getSYM() + " " + std::to_string(wordLPos + 1) + "\n");
				wordBuffer.clear();
			}
		}
	}

	buffer = result;
}
