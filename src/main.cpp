// NewLang.cpp : 定义控制台应用程序的入口点。
//

#include "lang.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

void saveLex(std::vector<Lang::Token*> tokens);
void saveTree(Lang::AST::Node* node, string expr = "");
string printNode(Lang::AST::Node* node);

int main()
{
	try {
		auto lexer = new Lang::Lexer("test.txt");
		auto ast = new Lang::AST(lexer);
		ast->parse();
		saveTree(ast->root);

		//int i = ast->detectExprEnd(0);
		//stringstream ss;
		//for (int j = 0; j < i; j++) ss << lexer->tokens[j]->value;
		//auto node = ast->parseExpr(0, i);
		//saveTree(node, ss.str());
		saveLex(lexer->tokens);
	} catch (exception e) {
		cout << e.what() << endl;
		system("pause");
	}
	
    return 0;
}

void saveLex(std::vector<Lang::Token*> tokens) {
	if (tokens.size() == 0) return;
	fstream fs("test.lex.txt", ios::out);
	stringstream ss;
	ss << tokens[tokens.size() - 1]->row;
	auto width = ss.str().length();
	for (auto token : tokens) {
		//if (token->kind == Lang::Token::Kind::kComment) continue;
		stringstream ss;
		ss << "(" << token->kind << "," << token->type << ") [";
		ss.width(width);
		ss << token->row << ",";
		ss.width(2);
		ss << token->col << "] ";
		fs << ss.str() + token->value << endl;
	}
	fs.close();
}

string loadFile(string file) {
	fstream fs(file, ios::in);
	if (fs.is_open()) {
		stringstream ss;
		char ch;
		while (!fs.eof()) {
			fs.read(&ch, 1);
			ss << ch;
		}
		fs.close();
		return ss.str();
	}
	return "";
}

void saveTree(Lang::AST::Node* node, string expr) {
	stringstream ss;
	ss << "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">";
	ss << "td{ vertical-align: top; }";
	ss << "span{ display: block; text-align:center; }";
	ss << "div{ border-style: solid; border-width: 1px; text-align:center; padding:2px; }";
	ss << "</style></head><body><table><tr><td>";
	ss << "<span>" + expr + "</span>";
	ss << printNode(node);
	ss << "</td></tr></table></body></html>";
	fstream fs("test.tree.html", ios::out);
	fs << ss.str();
	fs.close();

	if (loadFile("test.tree.html") != loadFile("test.html")) {
		cout << "test failed" << endl;
	}
}

string printNode(Lang::AST::Node* node) {
	string s = "<div>";
	s += "<span>" + node->token->value + "</span>";
	s += "<table><tr>";
	for (auto n : node->children) {
		s += "<td>" + printNode(n) + "</td>";
	}
	s += "</tr></table>";
	s += "</div>";
	return s;
}

