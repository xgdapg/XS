// NewLang.cpp : 定义控制台应用程序的入口点。
//

#include "lang.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

void saveLex(std::vector<Lang::Token> tokens);

int main()
{
	auto lexer = new Lang::Lexer("test.txt");
	try {
		auto tokens = lexer->getTokens();
		saveLex(tokens);
	} catch (exception e) {
		cout << e.what() << endl;
		system("pause");
	}
	
    return 0;
}

void saveLex(std::vector<Lang::Token> tokens) {
	if (tokens.size() == 0) return;
	fstream fs("test.lex.txt", ios::out);
	stringstream ss;
	ss << tokens[tokens.size() - 1].row;
	int width = ss.str().length();
	for (auto token : tokens) {
		if (token.type == Lang::Token::Type::LineComment) continue;
		if (token.type == Lang::Token::Type::BlockComment) continue;
		stringstream ss;
		ss << token.type << " [";
		ss.width(width);
		ss << token.row << ",";
		ss.width(2);
		ss << token.col << "] ";
		fs << ss.str() + token.value << endl;
	}
	fs.close();
}

