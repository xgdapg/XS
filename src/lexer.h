#pragma once

#include <string>
#include <vector>
#include <list>
#include <sstream>
#include "token.h"

using namespace std;

namespace Lang {
	class Lexer {
	public:
		Lexer(string path) {
			tokens = vector<Token*>();
			tokenList = list<Token*>();
			file = path;
			parse();
			makeArray();
		}

		vector<Token*> tokens;

	private:
		string         file;

		vector<string> lines;
		unsigned int   row = 0;
		unsigned int   col = 0;

		list<Token*> tokenList;

		bool loadFile(string file);

		char c;
		stringstream ss;
		void parse();
		void fetchString();
		void fetchLineComment();
		void fetchBlockComment();
		void fetchNumber();

		void addToken(string value, Token::Kind kind = Token::Kind::kUnknown, Token::Type type = Token::Type::tUnknown);
		void movePtr(unsigned int offset);
		char getChar(unsigned int offset);
		char getLineChar(unsigned int offset);
		string getEndl(string file);
		void throwException(string e);

		void makeArray();
	};
}
