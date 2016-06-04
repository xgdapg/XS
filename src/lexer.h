#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "token.h"

using namespace std;

namespace Lang {
	class Lexer {
	public:
		Lexer(string path) {
			tokens = vector<Token>();
			file = path;
		}

		vector<Token> getTokens();

	private:
		string         file;
		vector<Token>  tokens;

		vector<string> lines;
		unsigned int   row = 0;
		unsigned int   col = 0;
		
		string text;
		int    totalLine;

		bool loadFile(string file);


		enum State {
			Normal,
			String,
			Number,
			Comment,
		};

		State state = State::Normal;

		bool loadToken();

		void addToken(string value, Token::Type type = Token::Type::Unknown);

		void movePtr(unsigned int offset);

		char getChar(unsigned int offset);

		char getLineChar(unsigned int offset);

		string getEndl(string file);

		void throwException(string e);
	};
}