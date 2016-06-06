#include <fstream>
#include <iostream>
#include "lexer.h"
#include "define.h"

#ifndef EOF
#define EOF (-1)
#endif // !EOF
#define EOL EOF

namespace Lang {
	vector<Token*> Lexer::getTokens() {
		if (tokens.size() == 0) {
			if (!loadFile(file)) {
				throwException("Fail to open file");
			}
			parse();
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

	void Lexer::parse() {
		while (row < lines.size()) {
			c = getLineChar(0);
			//cout << "row: " << row + 1 << ", c: " << c << endl;
			if (c == EOL) {
				row++;
				col = 0;
				continue;
			}

			ss.str("");
			char nc = getLineChar(1);

			if (c == '"') {
				fetchString();
				continue;
			}
			if (c == '/' && nc == '/') {
				fetchLineComment();
				continue;
			}
			if (c == '/' && nc == '*') {
				fetchBlockComment();
				continue;
			}
			if (c == '*' && nc == '/') {
				throwException("error: unmatched block comment");
			}
			if (c >= '0' && c <= '9') {
				fetchNumber();
				continue;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				movePtr(1);
				continue;
			}
			if (c == '-' && nc == '>' || c == '&' && nc == '&' || c == '|' && nc == '|') {
				ss << c << nc;
				addToken(ss.str(), Token::Kind::kOperator);
				continue;
			}
			if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '!' || c == '=' || c == '>' || c == '<') {
				ss << c;
				if (nc == '=') {
					ss << '=';
				}
				addToken(ss.str(), Token::Kind::kOperator);
				continue;
			}
			if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == ',' || c == '.' || c == '&') {
				ss << c;
				addToken(ss.str(), Token::Kind::kOperator);
				continue;
			}

			int i = 0;
			while (c == '_' || c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c < 0 && c != EOL) {
				ss << c;
				c = getLineChar(++i);
			}
			
			addToken(ss.str(), Keywords.find(ss.str()) == Keywords.end() ? Token::Kind::kIdentifier : Token::Kind::kKeyword);
			continue;
		}
	}

	void Lexer::fetchString() {
		movePtr(1);
		c = getChar(0); // skip the first "
		int i = 0;
		int escapeCnt = 0;
		while (c != EOF) {
			if (c == '"') {
				addToken(ss.str(), Token::Kind::kLiteral, Token::Type::tString);
				movePtr(1 + escapeCnt);
				return;
			} else if (c == '\\') {
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
			} else {
				ss << c;
			}
			c = getChar(++i);
		}
		throwException("error: unclosed string");
	}

	void Lexer::fetchLineComment() {
		int i = 0;
		while (c != EOL && c != '\r' && c != '\n') {
			ss << c;
			c = getLineChar(++i);
		}
		addToken(ss.str(), Token::Kind::kComment, Token::Type::tLineComment);
		row++;
		col = 0;
	}

	void Lexer::fetchBlockComment() {
		int clv = 0;
		int i = 0;
		while (c != EOF) {
			ss << c;
			char nc = getChar(++i);
			if (c == '/' && nc == '*') { ss << nc; i++; clv++; }
			if (c == '*' && nc == '/') { ss << nc; i++; clv--; }
			if (clv == 0) {
				addToken(ss.str(), Token::Kind::kComment, Token::Type::tBlockComment);
				return;
			}
			c = getChar(i);
		}
		throwException("error: unclosed block comment");
	}

	void Lexer::fetchNumber() {
		bool hasDot = false;
		int i = 0;
		while (c != EOF) {
			if (c == '.') {
				if (hasDot) {
					throwException("error: number format 2dot");
				}
				hasDot = true;
				ss << c;
			} else if (c >= '0' && c <= '9') {
				ss << c;
			} else {
				break;
			}
			c = getChar(++i);
		}
		addToken(ss.str(), Token::Kind::kLiteral, hasDot ? Token::Type::tFloat : Token::Type::tInteger);
	}


	

	void Lexer::addToken(string value, Token::Kind kind, Token::Type type) {
		auto token = new Token(kind, type, value, row + 1, col + 1);
		tokens.push_back(token);
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