#include "GrammarAnalyzer.h"
#include "PL0.h"

GrammarAnalyzer::GrammarAnalyzer(const std::map<std::string, V>& G, const std::string& gStart,
	const TABLE& table): 
	G(G), gStart(gStart), table(table) { }

GrammarAnalyzer::StepResult GrammarAnalyzer::start(std::istream& input, std::ostream& output, const PL0& pl0) {
	std::vector<std::string> stk;
	std::vector<TreeNode*> trstk;
	StepResult result{ TreeNode(0) };

	trstk.push_back(&result.process);
	trstk[trstk.size() - 1]->v = &G.at(GrammarAnalyzer::G_EMPTY);

	trstk.push_back(new TreeNode(0, &G.at(gStart)));
	trstk[trstk.size() - 2]->push(trstk[trstk.size() - 1]);

	stk.push_back(GrammarAnalyzer::G_END);
	stk.push_back(gStart);

	const auto check = [&] (std::string code, int line) -> bool {
		while (true) {
			for (const auto& is : stk) {
				PERR("%s ", is.data());
			}
			PERR(" ---- %s\n", code.data());
			if (stk.size() == 1) { 
				if (stk[0] == GrammarAnalyzer::G_END) {
					return true;
				} else { return false; }
			}
			const V& v = G.at(stk[stk.size() - 1]);
			if (v.type == Type::Terminal) {
				if (v.value == code) {
					trstk.pop_back();
					stk.pop_back();
					return true;
				} else {
					PERR("Error T != code. At %s : %s, line %d\n", v.value.data(), code.data(), line);
					return false;
				}
			} else {
				if (table.find({ v.value, code }) != table.end()) {
					TreeNode* p = trstk[trstk.size() - 1];
					trstk.pop_back();
					stk.pop_back();
					std::list<std::string> right = table.at({ v.value, code });
					std::stack<std::string> helper;
					for (const auto& str : right) {
						if (str != GrammarAnalyzer::G_EMPTY) {
							helper.push(str);
						}
					}
					
					while (!helper.empty()) {
						stk.push_back(helper.top());
						trstk.push_back(new TreeNode(line, &G.at(helper.top())));
						p->push(trstk[trstk.size() - 1]);
						helper.pop();
					}

				} else {
					PERR("Error table(NT, code) is empty. At %s : %s, line %d\n", v.value.data(), 
						 code.data(), line);
					return false;
				}
			}
		}
	};

	do {
		std::string token;
		std::getline(input, token);

		std::vector<std::string> tpair = ssplit(token, " ");

		if (tpair.size() == 3) {
			PERR("%s(%s) ", tpair[0].data(), tpair[1].data());
		} else if (tpair.size() != 0 && token.size() != 0) {
			PERR("\nERROR token pair %s\n", token.data());
			break;
		} if (token.size() == 0) {
			tpair = { G_EMPTY, G_EMPTY, "0"};
		}

		if (tpair[1] == pl0.numberName || tpair[1] == pl0.idname) {
			for (const auto c : tpair[0]) {
				char str[] = { c, 0 };
				if (!check(str, std::atoi(tpair[2].data()))) {
					exit(-1);
				}
			}
		} else {
			if (!check(tpair[0], std::atoi(tpair[2].data()))) {
				exit(-1);
			}
		}

	} while (!input.eof());
	PERR("\n");

	return result;
}
