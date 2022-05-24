#include "split.h"

std::vector<std::string> ssplit(const std::string& str, const std::string& c) {
	std::vector<std::string> res;
	char* buffer = new char[str.size() + 1];
	buffer[str.size()] = '\0';
	size_t bi = 0;
	
	for (size_t i = 0; i < str.size() + 1;) {
		char now = i < str.size() ? str[i] : '\0';

		buffer[bi] = now;
		bi += 1;

		size_t ci = 0;
		size_t si = 0;
		while (ci < c.size() && si + i < str.size() && c[ci] == str[si + i]) {
			ci += 1; si += 1;
		}

		if (ci == c.size() || now == '\0') {
			buffer[bi - 1] = '\0';
			bi = 0;
			res.push_back(buffer);
			ci = ci > 1 ? ci : 1;
		} else {
			ci = 1;
		}

		i += ci;
	}

	return res;
}

void delblank(std::string& s) {
	size_t l = 0;
	size_t len = 0;
	bool start = false;
	bool end = false;
	for (size_t i = 0; i < s.size(); i++) {
		if (!start && s[i] != ' ' && s[i] != '\t') {
			start = true;
			l = i;
		}
		if (start && !end && (s[i] == ' ' || s[i] == '\t')) {
			end = true;
			len = i - l;
		}
		if (end && s[i] != ' ' && s[i] != '\t') {
			end = false;
		}
	}
	if (!end) { len = s.size() - l; }
	if (!start) { len = 0; }
	s = s.substr(l, len);
}
