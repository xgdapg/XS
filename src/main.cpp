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
	auto lexer = new Lang::Lexer("test.txt");
	try {
		auto tokens = lexer->getTokens();
		auto ast = new Lang::AST(tokens);
		int i = ast->detectExprEnd(0);
		stringstream ss;
		for (int j = 0; j < i; j++) ss << tokens[j]->value;
		auto node = ast->parseExpr(0, i);
		saveTree(node, ss.str());
		saveLex(tokens);
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
	int width = ss.str().length();
	for (auto token : tokens) {
		if (token->kind == Lang::Token::Kind::kComment) continue;
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

void saveTree(Lang::AST::Node* node, string expr) {
	fstream fs("test.tree.html", ios::out);
	fs << "<!DOCTYPE html><html><head><style type=\"text/css\">";
	fs << "td{ vertical-align: top; }";
	fs << "span{ display: block; text-align:center; }";
	fs << "div{ border-style: solid; border-width: 1px; text-align:center; padding:2px; }";
	fs << "</style></head><body><table><tr><td>";
	fs << "<span>" + expr + "</span>";
	fs << printNode(node);
	fs << "</td></tr></table></body></html>";
	fs.close();
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

