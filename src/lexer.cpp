#include "lang.h"
#include <fstream>
#include <iostream>

#define EOL (-2)

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
		//cout << "row: " << row << ", c: " << c << endl;
		if (c == EOL) {
			row++;
			col = 0;
			return true;
		}

		stringstream ss;
			
		if (state == State::Normal) {
			if (c == '"') {
				state = State::String;
				movePtr(1);
				return true;
			}
			if (c == '/' && getLineChar(1) == '/') {
				row++;
				col = 0;
				return true;
			}
			if (c == '/' && getLineChar(1) == '*') {
				state = State::Comment;
				movePtr(2);
				return true;
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
			if (c == '-' && getLineChar(1) == '>') {
				addToken("->");
				return true;
			}
			if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '!' || c == '=' || c == '>' || c == '<') {
				ss << c;
				if (getLineChar(1) == '=') {
					ss << '=';
				}
				addToken(ss.str());
				return true;
			}
			//else {
				int i = 0;
				while (c == '_' || c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c < 0) {
					ss << c;
					c = getChar(++i);
				}
				addToken(ss.str());
				return true;
			//}
			return true;
		}

		else if (state == State::String) {
			int i = 0;
			char c1 = getChar(i);
			int escapeCnt = 0;
			while (c1 != EOF) {
				if (c1 == '"') {
					addToken(ss.str(), Token::Type::String);
					state = State::Normal;
					movePtr(1 + escapeCnt);
					return true;
				}
				else if (c1 == '\\') {
					char c2 = getChar(i + 1);
					if (c2 == 't') ss << '\t';
					else if (c2 == 'r') ss << '\r';
					else if (c2 == 'n') ss << '\n';
					else if (c2 == '\\') ss << '\\';
					else if (c2 == '"') ss << '"';
					else {
						stringstream e;
						e << "error: unknown escape " << '\\' << c2;
						throwException(e.str());
					}
					escapeCnt++;
					i++;
				}
				else {
					ss << c1;
				}
				i++;
				c1 = getChar(i);
			}

			throwException("error: string EOF");
		}

		else if (state == State::Comment) {
			int clv = 1;
			int i = 0;
			char c1 = getChar(i);

			while (c1 != EOF) {
				char c2 = getChar(i + 1);
				if (c1 == '/' && c2 == '*') {
					clv++;
					i++;
				}
				else if (c1 == '*' && c2 == '/') {
					clv--;
					i++;
				}

				i++;
				if (clv == 0) {
					state = State::Normal;
					movePtr(i);
					return true;
				}
				c1 = getChar(i);
			}
			return false;
		}

		else if (state == State::Number) {
			int i = 0;
			bool hasDot = false;
			char cn = getChar(i);
			while (cn != EOF) {
				if (cn == '.') {
					if (hasDot) {
						throwException("error: number format 2dot");
					}
					hasDot = true;
					ss << '.';
				}
				else if (cn >= '0' && cn <= '9') {
					ss << cn;
				}
				else {
					state = State::Normal;
					addToken(ss.str(), hasDot ? Token::Type::Float : Token::Type::Integer);
					return true;
				}
				i++;
				cn = getChar(i);
			}
		}

		else {
			throwException("error: else state");
		}
		return false;
	}

	void Lexer::addToken(string value, Token::Type type) {
		tokens.push_back(Token(type, value, row, col));
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
		cout << "[" << row << "," << col << "] ";
		throw exception(e.c_str());
	}
}