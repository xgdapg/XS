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

		Node* root;
		Lexer* lex;

		//vector<Token*> tokens;
		int index;
		//Token* emptyToken;

		AST(Lexer* l) {
			root = new Node(new Token(Token::Kind::kBlock, Token::Type::tUnknown, "", 0, 0));
			lex = l;
			index = 0;
		}

		Token* token(unsigned int i = 0);

		//void buildWithTokens(vector<Token*> tokens);
		//
		//Node* createDeclareVar();
		//Node* createAssign();
		//Node* createExpr();

		void parse();

		int detectExprEnd(unsigned int begin);

		Node* parseExpr(unsigned int begin, unsigned int end);
		Node* parseFunc(unsigned int begin, unsigned int end);
		Node* parseSubscript(unsigned int begin, unsigned int end);

		Node* parseDeclVar(unsigned int begin);

	};
	

	
}