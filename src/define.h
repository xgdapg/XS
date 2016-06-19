#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "token.h"
#include "lexer.h"
#include <unordered_set>

using namespace std;

namespace Lang {
	static unordered_set<string> Keywords ={ "fn", "if", "else", "var", "const", "return", "struct", "impl", "while", "for", "in", "continue", "break", "interface", "true", "false" };
	static vector<string> Operators;

}