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
		//int index;
		//Token* emptyToken;

		AST(Lexer* l) {
			root = new Node(new Token(Token::Kind::kBlock, Token::Type::tUnknown, "", 0, 0));
			lex = l;
			//index = 0;
		}

		//Token* token(unsigned int i = 0);

		//void buildWithTokens(vector<Token*> tokens);
		//
		//Node* createDeclareVar();
		//Node* createAssign();
		//Node* createExpr();

		//void parse();

		//int detectExprEnd(unsigned int begin);

		//Node* parseExpr(int begin, int end);
		//Node* parseFunc(int begin, int end);
		//Node* parseSubscript(int begin, int end);
		//
		//Node* parseDeclVar(int begin, int end);
		//
		//int findPair(int begin, int end, string l, string r);
		//int findStatement(int begin, int end);


		int index = 0;
		Token* tk(int offset = 0);

		void parse();

		Node* parsePrimaryExpr();
		Node* parseFuncCall();
		Node* parseSubscript();
		Node* parseParenExpr();
		Node* parseExpression();
		Node* parseDeclVar();

		Node* buildTree(vector<Node*> list, int begin, int end);
	};
	

	
}