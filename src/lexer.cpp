#include "lang.h"
#include <fstream>
#include <iostream>

#define EOL (0)
#define EOF (0)

namespace Lang {
	vector<Token> Lexer::getTokens() {
		if (tokens.size() == 0) {
			if (!loadFile(file)) {
				throwException("Fail to open file");
			}
			while (loadToken());
		}
		return tokens;
	}

	bool Lexer::loadFile(string file) {
		lines = vector<string>();
		string endl = getEndl(file);
		
		fstream fs(file);
		if (fs.is_open()) {
			string str;
			while (getline(fs, str)) {
				cout << "lines: " << lines.size() << ", length: " << str.length() << endl;
				lines.push_back(str + endl);
			}
			row = 0;
			col = 0;
			fs.close();

			return true;
		}
		return false;
	}

	bool Lexer::loadToken() {
		if (row >= lines.size()) return false;

		char c = getLineChar(0);
		//cout << "row: " << row + 1 << ", c: " << c << endl;
		if (c == EOL) {
			row++;
			col = 0;
			return true;
		}

		stringstream ss;
			
		if (state == State::Normal) {
			char nc = getLineChar(1);
			if (c == '"') {
				state = State::String;
				movePtr(1);
				return true;
			}
			if (c == '/' && nc == '/') {
				int i = 0;
				while (c != EOL && c != '\r' && c != '\n') {
					ss << c;
					c = getLineChar(++i);
				}
				addToken(ss.str(), Token::Type::LineComment);
				row++;
				col = 0;
				return true;
			}
			if (c == '/' && nc == '*') {
				state = State::Comment;
				return true;
			}
			if (c == '*' && nc == '/') {
				throwException("error: unmatched block comment");
			}
			if (c >= '0' && c <= '9') {
				state = State::Number;
				return true;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				movePtr(1);
				return true;
			}
			if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == ',' || c == '.' || c == '&') {
				ss << c;
				addToken(ss.str());
				return true;
			}
			if (c == '-' && nc == '>') {
				addToken("->");
				return true;
			}
			if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '!' || c == '=' || c == '>' || c == '<') {
				ss << c;
				if (nc == '=') {
					ss << '=';
				}
				addToken(ss.str());
				return true;
			}
			
			int i = 0;
			while (c == '_' || c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c < 0) {
				ss << c;
				c = getLineChar(++i);
			}
			addToken(ss.str());
			return true;
		}

		else if (state == State::String) {
			int i = 0;
			int escapeCnt = 0;
			while (c != EOF) {
				if (c == '"') {
					addToken(ss.str(), Token::Type::String);
					state = State::Normal;
					movePtr(1 + escapeCnt);
					return true;
				}
				else if (c == '\\') {
					char nc = getChar(++i);
					if (nc == '\\') ss << '\\';
					else if (nc == 't') ss << '\t';
					else if (nc == 'r') ss << '\r';
					else if (nc == 'n') ss << '\n';
					else if (nc == '"') ss << '"';
					else {
						stringstream e;
						e << "error: unknown escape " << '\\' << nc;
						throwException(e.str());
					}
					escapeCnt++;
				}
				else {
					ss << c;
				}
				c = getChar(++i);
			}
			throwException("error: string EOF");
		}

		else if (state == State::Comment) {
			int clv = 0;
			int i = 0;
			
			while (c != EOF) {
				ss << c;
				char nc = getChar(++i);
				if (c == '/' && nc == '*') { ss << nc; i++; clv++; }
				if (c == '*' && nc == '/') { ss << nc; i++; clv--; }
				if (clv == 0) {
					state = State::Normal;
					addToken(ss.str(), Token::Type::BlockComment);
					return true;
				}
				c = getChar(i);
			}
			throwException("error: unclosed block comment");
			//addToken(ss.str(), Token::Type::BlockComment);
			return false;
		}

		else if (state == State::Number) {
			int i = 0;
			bool hasDot = false;
			while (c != EOF) {
				if (c == '.') {
					if (hasDot) {
						throwException("error: number format 2dot");
					}
					hasDot = true;
					ss << c;
				}
				else if (c >= '0' && c <= '9') {
					ss << c;
				}
				else {
					state = State::Normal;
					addToken(ss.str(), hasDot ? Token::Type::Float : Token::Type::Integer);
					return true;
				}
				c = getChar(++i);
			}
		}

		else {
			throwException("error: unknown state");
		}

		return false;
	}

	void Lexer::addToken(string value, Token::Type type) {
		tokens.push_back(Token(type, value, row + 1, col + 1));
		movePtr(value.length());
	}

	void Lexer::movePtr(unsigned int offset) {
		col += offset;
		while (row < lines.size()) {
			auto str = lines[row];
			if (col >= str.length()) {
				col -= str.length();
				row++;
				continue;
			}
			break;
		}
	}

	char Lexer::getChar(unsigned int offset) {
		auto r = row;
		auto c = col + offset;
		while (r < lines.size()) {
			auto str = lines[r];
			if (c >= str.length()) {
				c -= str.length();
				r++;
				continue;
			}
			return str[c];
		}
		return EOF;
	}

	char Lexer::getLineChar(unsigned int offset) {
		auto c = col + offset;
		auto str = lines[row];
		if (c < str.length()) {
			return str[c];
		}
		return EOL;
	}

	string Lexer::getEndl(string file) {
		stringstream ss;

		fstream fs(file, ios::in | ios::binary);
		if (fs.is_open()) {
			while (!fs.eof()) {
				char c = fs.get();
				if (c == '\r') {
					ss << c;
					c = fs.get();
					if (c == '\n') ss << c;
					break;
				} else if (c == '\n') {
					ss << c;
					break;
				}
			}
			fs.close();
		}
		if (ss.str().length() == 0) ss << endl;
		return ss.str();
	}

	void Lexer::throwException(string e) {
		cout << "[" << row + 1 << "," << col + 1 << "] ";
		throw exception(e.c_str());
	}
}