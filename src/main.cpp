// NewLang.cpp : 定义控制台应用程序的入口点。
//

#include "lang.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "util.h"

string createASTHtml(Lang::AST::Node* node);
string printNode(Lang::AST::Node* node);

int main()
{
	bool runTest = false;
	try {
		auto list = getFiles("xs");
		for (auto f : list) {
			if (runTest) cout << "testing " << f << "... ";
			else         cout << "parsing " << f << "... ";

			if (!isUTF8(getFileContent(f))) {
				cout << "FAILED" << endl;
				cout << "UTF-8 required" << endl;
				continue;
			}

			auto lexer = new Lang::Lexer(f);
			//saveLex(lexer->tokens);
			auto ast = new Lang::AST(lexer);
			ast->parse();

			if (runTest) {
				if (sameToFile(f + ".html", createASTHtml(ast->root))) {
					cout << "OK" << endl;
				} else {
					cout << "FAILED" << endl;
				}
			}
			else {
				saveToFile(f + ".html", createASTHtml(ast->root));
				cout << "OK" << endl;
			}
		}
	} catch (exception e) {
		cout << endl << e.what() << endl;
		system("pause");
	}
	
    return 0;
}

//void saveLex(std::vector<Lang::Token*> tokens) {
//	if (tokens.size() == 0) return;
//	fstream fs("test.lex.txt", ios::out);
//	stringstream ss;
//	ss << tokens[tokens.size() - 1]->row;
//	auto width = ss.str().length();
//	for (auto token : tokens) {
//		//if (token->kind == Lang::Token::Kind::kComment) continue;
//		stringstream ss;
//		ss << "(" << token->kind << "," << token->type << ") [";
//		ss.width(width);
//		ss << token->row << ",";
//		ss.width(2);
//		ss << token->col << "] ";
//		fs << ss.str() + token->value << endl;
//	}
//	fs.close();
//}

string createASTHtml(Lang::AST::Node* node) {
	stringstream ss;
	ss << "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">";
	ss << "td{ vertical-align: top; }";
	ss << "span{ display: block; text-align:center; }";
	ss << "div{ border-style: solid; border-width: 1px; text-align:center; padding:2px; }";
	ss << "</style></head><body><table><tr><td>";
	ss << printNode(node);
	ss << "</td></tr></table></body></html>";
	return ss.str();
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
