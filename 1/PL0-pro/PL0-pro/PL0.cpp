#include "PL0.h"
#include "debug.h"
#include "split.h"
#include "debug.h"

typedef std::set<std::string> SET;
const auto minus = [](const SET& a, const SET& b) -> SET {
	SET res;
	for (const auto& i : a) { if (b.find(i) == b.end()) { res.insert(i); } }
	return res;
};

PL0::PL0(const std::map<std::string, std::string>& keywords, std::map<std::string, std::string>& operators,
		std::map<std::string, std::string>& boundary, std::set<std::string>& numbers, 
		const std::string& numberName, const std::string& idname,
		const std::map<std::string, GrammarAnalyzer::V>& G) {
	this->keywords = keywords;
	this->operators = operators;
	this->boundary = boundary;
	this->numbers = numbers;
	this->numberName = numberName;
	this->idname = idname;
	this->G = G;
}

PL0::PL0(const std::string keywords, const std::string operators, const std::string boundary, 
	const std::string numbers, const std::string idname, const std::string G) {
	makeLexicalTable(keywords, operators, boundary, numbers, idname);
	makeGrammarTable(G);
	makeFirstFollow();
	makeTable();
}

void PL0::makeLexicalTable(const std::string& keywords, const std::string& operators, const std::string& boundary,
	const std::string& numbers, const std::string& idname) {
	std::string buffer = "";
	std::vector<std::string> strs;

	strs.clear();
	for (size_t i = 0; i < keywords.size(); i++) {
		if (keywords[i] == ' ') {
			if (buffer.size() > 0) {
				strs.push_back(buffer);
				buffer.clear();
			}
		} else {
			buffer += keywords[i];
		}
	}
	if (buffer.size() > 0) {
		strs.push_back(buffer);
		buffer.clear();
	}
	for (size_t i = 0; i < strs.size(); i += 2) {
		this->keywords.insert(std::make_pair(strs[i], strs[i + 1]));
	}

	for (auto k : this->keywords) {
		LOG("keyword: " << k.first << " name: " << k.second << "\n");
	}

	strs.clear();
	buffer.clear();
	for (size_t i = 0; i < operators.size(); i++) {
		if (operators[i] == ' ') {
			if (buffer.size() > 0) {
				strs.push_back(buffer);
				buffer.clear();
			}
		} else {
			buffer += operators[i];
		}
	}
	if (buffer.size() > 0) {
		strs.push_back(buffer);
		buffer.clear();
	}
	for (size_t i = 0; i < strs.size(); i += 2) {
		this->operators.insert(std::make_pair(strs[i], strs[i + 1]));
	}

	for (auto k : this->operators) {
		LOG("operator: " << k.first << " name: " << k.second << "\n");
	}

	strs.clear();
	buffer.clear();
	for (size_t i = 0; i < boundary.size(); i++) {
		if (boundary[i] == ' ') {
			if (buffer.size() > 0) {
				strs.push_back(buffer);
				buffer.clear();
			}
		} else {
			buffer += boundary[i];
		}
	}
	if (buffer.size() > 0) {
		strs.push_back(buffer);
		buffer.clear();
	}
	for (size_t i = 0; i < strs.size(); i += 2) {
		this->boundary.insert(std::make_pair(strs[i], strs[i + 1]));
	}

	for (auto k : this->boundary) {
		LOG("boundary: " << k.first << " name: " << k.second << "\n");
	}

	strs.clear();
	buffer.clear();
	for (size_t i = 0; i < numbers.size(); i++) {
		if (numbers[i] == ' ') {
			if (buffer.size() > 0) {
				strs.push_back(buffer);
				buffer.clear();
			}
		} else {
			buffer += numbers[i];
		}
	}
	if (buffer.size() > 0) {
		strs.push_back(buffer);
		buffer.clear();
	}
	for (size_t i = 1; i < strs.size(); i += 1) {
		this->numbers.insert(strs[i]);
	}
	if (strs.size() > 0) { numberName = strs[0]; }
	LOG(numberName << ": \n");
	for (auto k : this->numbers) {
		LOG("number: " << k << "\n");
	}

	this->idname = idname;
}

void PL0::makeGrammarTable(const std::string& Gstr) {
	size_t i = 0;

	const auto doforline = [&] (size_t indice) -> size_t {
		size_t lindice = indice;
		bool start = false;
		bool ignore = false;
		while (true) {
			if (indice > Gstr.size()) { break; }
			if (Gstr[indice] == '\n') { break; }

			if (!start && Gstr[indice] == '#') { ignore = true; }
			else if (!start && Gstr[indice] != ' ' && Gstr[indice] != '\t') {
				start = true;
			}

			indice += 1;
		}

		if (!ignore && indice > lindice && start) {
			std::string line = Gstr.substr(lindice, indice - lindice).data();
			PERR("[%s]\n", line.data());
			size_t mid = line.find(TRANS_SIGN);
			size_t opspos = line.find(OPS_SIGN);

			std::string left = line.substr(0, mid - 0);
			delblank(left);

			if (this->G.size() == 0) { gStart = left; PERR("START: %s\n", left.data()); }

			std::string right = line.substr(mid + TRANS_SIGN_SIZE, opspos - mid - TRANS_SIGN_SIZE);
			delblank(right);

			std::string opsstr = line.substr(opspos + OPS_SIGN_SIZE, line.size() - opspos - OPS_SIGN_SIZE);
			delblank(opsstr);
			PERR("%llu: %s\n", opspos, opsstr.data());

			PERR("%s --> %s    {%s}\n", left.data(), right.data(), opsstr.data());

			/** 
			  * Save this v -->aB|cDe|f|... into map G
			  */
			std::vector<std::string> choose = ssplit(right, TRANS_SPLIT);			
			GrammarAnalyzer::V v;
			v.type = GrammarAnalyzer::Type::NonTerminal;
			v.value = left;

			for (auto c : choose) {
				std::vector<std::string> ss = ssplit(c, " ");
				std::list<std::string> postc;

				for (auto s : ss) {
					if (s.size() == 0) { continue; }
					postc.push_back(s);
					if (s[0] != '<' || s[s.size() - 1] != '>') {
						GrammarAnalyzer::V n;
						n.value = s;
						n.type = GrammarAnalyzer::Type::Terminal;
						n.SYM = std::isdigit(s[0]) ? numberName : idname;
						if (keywords.find(s) != keywords.end()) {
							n.SYM = keywords[s];
						}
						if (boundary.find(s) != boundary.end()) {
							n.SYM = boundary[s];
						}
						if (operators.find(s) != operators.end()) {
							n.SYM = operators[s];
						}
						this->G.insert({s, n});
					}
				}

				v.trans.push_back(postc);
			}
			this->G.insert({ left, v });

			/** 
			  * Save every mini program into map rules
			  */
			if (opsstr.size() > 0) {
				std::vector<std::string> codes = ssplit(opsstr, TRANS_SPLIT);
				auto it = G.at(left).trans.begin();
				for (size_t i = 0; i < codes.size(); i++) {
					if (i >= G.at(left).trans.size()) {
						PERR("Grammar %s rules are not match mini program.\n", left.data()); exit(-1);
					}

					std::string right;
					for (const auto& p : *it) {
						right.append(p);
						right.append(" ");
					}
					delblank(right);
					if (right == GrammarAnalyzer::G_EMPTY) {
						right = "";
					}

					if (codes[i].size() > 0) {
						rules.insert({ std::make_pair(left, right), codes[i]});
					}

					it++;
				}
			}
		}

		return indice + 1;
	};

	while (true) {
		i = doforline(i);
		if (i >= Gstr.size()) break;
	}

	for (auto& v : this->G) {
		PERR("V: %s (%s: %llu) { ", v.first.data(), v.second.type == GrammarAnalyzer::Type::Terminal ? v.second.SYM.data() : "-NT-", v.first.size());
		if (v.second.trans.size() > 0) { PERR("\n"); }
		for (auto& sub : v.second.trans) {
			PERR("    [ ");
			for (auto& t : sub) {
				PERR("%s ", t.data());
			}
			PERR("]\n");
		}
		PERR("}\n\n");
	}

	for (const auto& r : rules) {
		PERR("%s --> %s", r.first.first.data(), r.first.second.data());
		PERR("    CODE STEP DO { %s }\n", r.second.data());
	}

	// PERR("Test %s\n", rules.at({ "<E>", ", <ID> <E>" }).data());
}

void PL0::makeFirstFollow() {

	const auto getFirst = [&] (auto&& self, const std::string& vname) -> SET {
		GrammarAnalyzer::V& v = G[vname];
		if (v.type == GrammarAnalyzer::Type::Terminal) { return SET({ vname }); }
		if (v.gFirst.size() != 0) { return v.gFirst; }
		if (v.type == GrammarAnalyzer::Type::NonTerminal) {
			for (const auto& poss : v.trans) {
				// every v --> poss (poss: [ # ] or [ x1 x2 x3 ... xn, x* != # ])
				size_t k = 0;
				for (const auto& u : poss) {
					const SET&& uFirst = self(self, u);
					SET ins = (k + 1 == uFirst.size()) ? uFirst : minus(uFirst, { GrammarAnalyzer::G_EMPTY });
					v.gFirst.insert(uFirst.begin(), uFirst.end());
					if (uFirst.find(GrammarAnalyzer::G_EMPTY) == uFirst.end()) {
						break;
					}
					k += 1;
				}
			}
		}

		PERR("[%s]FIRST(", v.value.data());
		for (auto& fi : v.gFirst) {
			PERR("%s|", fi.data());
		}
		PERR(")\n");
		return v.gFirst;
	};

	const auto getFollow = [&](auto&& self, const std::string& vname) {
		GrammarAnalyzer::V& v = G[vname];
		// if (v.type == GrammarAnalyzer::Type::Terminal) { return SET({}); }
		if (v.gFollow.size() > 0) { return v.gFollow; }
		if (vname == gStart) { v.gFollow.insert(GrammarAnalyzer::G_END); }

		for (const auto& trans : G) {
			// every ' u --> poss0 | poss1 | ... ' in G
			GrammarAnalyzer::V& u = G[trans.first];
			for (const auto& poss : u.trans) {
				// every u --> poss in ' u --> poss0 | poss1 | ... '
				
				// search v in poss
				bool follow = false;
				size_t k = 0;
				for (auto it = poss.begin(); it != poss.end(); it++) {
					if (follow) {
						SET itFirst = getFirst(getFirst, *it);
						SET firstWithoutEmpty = minus(itFirst, { GrammarAnalyzer::G_EMPTY });
						v.gFollow.insert(firstWithoutEmpty.begin(), firstWithoutEmpty.end());
						if (itFirst.find(GrammarAnalyzer::G_EMPTY) == itFirst.end()) {
							follow = false;
						}
					}
					if (*it == vname) {
						follow = true;
					}
					if (vname != u.value && follow && k == poss.size() - 1) {
						SET uFollow = self(self, u.value);
						v.gFollow.insert(uFollow.begin(), uFollow.end());
					}
					k += 1;
				}
			}
		}

		PERR("[%s]FOLLOW(", v.value.data());
		for (auto& fi : v.gFollow) {
			PERR("%s|", fi.data());
		}
		PERR(")\n");
		return v.gFollow;
	};

	for (auto& v : G) {
		PERR("\n\nFIND %s FIRST\n\n", v.first.data());
		v.second.gFirst = getFirst(getFirst, v.first);
		PERR("\n\nFIND %s FOLLOW\n\n", v.first.data());
		v.second.gFollow = getFollow(getFollow, v.first);

		PERR("%s FIRST: { ", v.first.data());
		for (const auto& fi : v.second.gFirst) {
			PERR("%s ", fi.data());
		}
		PERR("}\n");
		PERR("%s FOLLOW: { ", v.first.data());
		for (const auto& fi : v.second.gFollow) {
			PERR("%s ", fi.data());
		}
		PERR("}\n");

		if (v.second.gFirst.find(GrammarAnalyzer::G_EMPTY) != v.second.gFirst.end()) {
			for (auto& i : v.second.gFirst) {
				if (v.second.gFollow.find(i) != v.second.gFollow.end()) {
					PERR("%s First * Follow != empty (both has %s)\n", v.first.data(), i.data());
					exit(-1);
				}
			}
		}
	}
}

void PL0::makeTable() {
	for (const auto& v : G) {
		PERR("TABLE: cal %s\n", v.first.data());
		for (const auto& tr : v.second.trans) {
			PERR("TABLE --> *: cal %s --> ", v.first.data());
			bool toEmpty = true;
			SET select;
			for (const auto& xstr : tr) {
				PERR("%s? \n", xstr.data());
				const GrammarAnalyzer::V& s = G[xstr];
				select.insert(s.gFirst.begin(), s.gFirst.end());
				PERR("%s --> FIRST: ", v.first.data());
				for (auto it = s.gFirst.begin(); it != s.gFirst.end(); it++) {
					PERR("%s ", (*it).data());
				}
				PERR("\n");
				if (s.gFirst.find(GrammarAnalyzer::G_EMPTY) == s.gFirst.end()) {
					toEmpty = false;
					break;
				}
			}
			if (toEmpty) {
				select = minus(select, { GrammarAnalyzer::G_EMPTY });
				select.insert(v.second.gFollow.begin(), v.second.gFollow.end());
				PERR("%s --> *#, add follow: ", v.first.data());
				for (auto it = v.second.gFollow.begin(); it != v.second.gFollow.end(); it++) {
					if (*it != GrammarAnalyzer::G_EMPTY) {  
						PERR("%s ", (*it).data());
					}
				}
				PERR("\n");
			} else {
				PERR("%s -X-> *#, delete # from select.\n", v.first.data());
				select = minus(select, { GrammarAnalyzer::G_EMPTY });
			}
			for (const auto& t : select) {
				PERR("fill %s:%s: \n", v.first.data(), t.data());
				if (table.find(std::make_pair(v.first, t)) != table.end()) {
					PERR("ERROR: table[%s, %s] \n", v.first.data(), t.data());
					PERR("OLD: { ");
					for (auto& item : table[std::make_pair(v.first, t)]) {
						PERR("%s ", item.data());
					}
					PERR("}\n");
					PERR("NEW: { ");
					for (auto& item : tr) {
						PERR("%s ", item.data());
					}
					PERR("}\n");
					exit(-1);
				}
				table[std::make_pair(v.first, t)] = tr;
			}
		}
	}

	auto last = std::make_pair(std::string(""), std::string(""));
	for (const auto& item : table) {
		if (item.first.first != last.first) {
			PERR("\n%s \t ", item.first.first.data());
		}
		last = item.first;

		PERR("%s: %s --> ", item.first.second.data(), item.first.first.data());
		for (const auto& part : item.second) {
			PERR("%s ", part.data());
		}
		PERR(" \t ");
	}
	PERR("\n");
}

void PL0::outputHTML(const GrammarAnalyzer::StepResult& result) {
	const char style[] = ".toggle { background: #FAFAFA; } \n.toggle:hover { background: #CACACA; }\n"
		".toggle .but:hover { background: #BAFABA; }\n"
		"body { overflow: auto; white-space: nowrap; }\n"
		"div { width: auto; }";
	printf("<!DOCTYPE html><html><head><meta charset=\"utf-8\" /><title>Tree</title>"
		"\n<style>\n%s</style>\n"
		"</head>\n<body>\n", style);

	const auto tospace = [&](uint32_t sp) {
		while (sp > 0) { printf("&ensp;"); sp--; }
	};

	const auto showTree = [&](auto&& self, const GrammarAnalyzer::TreeNode* td, uint32_t level) -> void {
		printf("<div style=\"cursor: pointer; user-select: none; %s\" class=\"toggle\"><div class=\"but\">",
			(td->children.size() == 0 && td->v->type == GrammarAnalyzer::Type::NonTerminal) ?
			"opacity: 0.2" : "");
		tospace(level * 4);
		std::string v = td->v->value;
		if (v.find("<") == 0 && v.find(">") + 1 == v.size()) { v = "&lt;" + v.substr(1, v.size() - 2) + "&gt;"; }
		printf("%s</div>", v.data());

		printf("<div>\n");
		for (auto it = td->children.rbegin(); it != td->children.rend(); it++) {
			self(self, *it, level + 1);
		}
		printf("</div>\n");

		printf("</div>\n");
	};

	printf("<div>\nProcess\n");
	showTree(showTree, &result.process, 0);
	printf("</div>\n");

	const char script[] = "let ps = document.getElementsByClassName('toggle')\n"
		"for (let i = 0; i < ps.length; i += 1) { if (ps[i].children.length == 0) { continue } ps[i].children[0].addEventListener('click', (ev) => { "
		"ev.target.parentNode.children[0].style.fontWeight = ev.target.parentNode.children[1].style.display != 'none' ? 'bold' : 'normal'\n"
		"ev.target.parentNode.children[1].style.display = ev.target.parentNode.children[1].style.display == 'none' ? 'block' : 'none'\n"
		" }) }\n";
	printf("<script>\n%s</script>\n</body>\n</html>", script);
}

void PL0::outputTree(const GrammarAnalyzer::StepResult& result) {
	const auto sp = [] (uint32_t s) { while (s > 0) { printf("\t"); s--; } };
	const auto mkLine = [&] (auto&& self, const GrammarAnalyzer::TreeNode* t, uint32_t l) -> void {
		sp(l);
		std::stack<std::string> stk;
		std::string right;
		for (const auto& sub : t->children) {
			stk.push(sub->v->value);
		}
		while (!stk.empty()) {
			right.append(stk.top());
			right.append(" ");
			stk.pop();
		}
		delblank(right);
		PERR("[%s][%s]: {%s}\n", t->v->value.data(), right.data(), rules.find({ t->v->value, right }) != rules.end() ? rules.at({ t->v->value, right }).data() : "");
		printf("%d %s %s %s   %s %s%s", t->line,
			t->v->value.data(), 
			t->v->type == GrammarAnalyzer::Type::NonTerminal ? TRANS_SIGN : "",
			t->v->type == GrammarAnalyzer::Type::NonTerminal ? right.data() : "",
			t->v->type == GrammarAnalyzer::Type::NonTerminal ? OPS_SIGN : "",
			rules.find({ t->v->value, right }) != rules.end() ? rules.at({ t->v->value, right }).data() : "",
			TREE_FILE_ENDL);
		for (auto it = t->children.rbegin(); it != t->children.rend(); it++) {
			self(self, *it, l + 1);
		}
	};
	mkLine(mkLine, *(result.process.children.begin()), 0);
}

void PL0::run(uint32_t stage) {

	switch (stage) {
	case 0:
	{
		LexicalAnalyzer lexicalAnalyzer(keywords, operators, boundary, numbers, numberName, idname);
		std::istream& input = std::cin;
		std::ostream& output = std::cout;
		lexicalAnalyzer.start(input, output, *this);
		break;
	}
	case 1:
	{
		GrammarAnalyzer grammarAnalyzer(G, gStart, table);
		std::istream& input = std::cin;
		std::ostream& output = std::cout;
		GrammarAnalyzer::StepResult result = grammarAnalyzer.start(input, output, *this);
		outputTree(result);
		break;
	}
	case 2:
	{
		SemanticAnalyzer semanticAnalyzer(G);
		std::istream& input = std::cin;
		std::ostream& output = std::cout;
		semanticAnalyzer.start(input, output);
		break;
	}
	case 10:
	{
		GrammarAnalyzer grammarAnalyzer(G, gStart, table);
		std::istream& input = std::cin;
		std::ostream& output = std::cout;
		GrammarAnalyzer::StepResult result = grammarAnalyzer.start(input, output, *this);
		outputHTML(result);
		break;
	}
	default:
		break;
	}
}
