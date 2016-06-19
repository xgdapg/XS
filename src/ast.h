#pragma once

#include <string>
#include <vector>
#include "token.h"
#include "lexer.h"

using namespace std;

namespace Lang {
	class AST {
	public:
		class Node {
		public:
			Token* token = nullptr;
			vector<Node*> children;
			Node* parent = nullptr;

			Node() {
				children = vector<Node*>();
			}

			Node(Token* t) {
				token = t;
				children = vector<Node*>();
			}

			void addChild(Node* node) {
				children.push_back(node);
				if (node->parent == nullptr) node->parent = this;
			}
		};


		Node*  root = nullptr;
		Lexer* lex  = nullptr;

		AST(Lexer* l) {
			lex = l;
		}

		int    index = 0;
		Token* tk(int offset = 0);

		void parse();

		Node* parseBlock(string end = "}", string sep = ";");
		Node* parsePrimaryExpr();
		Node* parseFuncCall();
		Node* parseSubscript();
		Node* parseParenExpr();
		Node* parseExpression(bool enableEmpty = false);
		Node* parseDeclVar();
		Node* parseDeclConst();
		Node* parseIfExpr();
		Node* parseAssign(Node* block);
		Node* parseWhileLoop();
		Node* parseForLoop();
		Node* parseField();
		Node* parseDefineFunc();
		Node* parseDefineStruct();
		Node* parseDefineImpl();

		Node* buildTree(vector<Node*> list, int begin, int end);
	};
	

	
}