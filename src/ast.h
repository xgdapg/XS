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

		AST(Lexer* l) {
			lex = l;
		}

		Node* root = nullptr;

		void parse();

	private:
		Lexer* lex  = nullptr;
		int    index = 0;
		Token* tk(int offset = 0);

		enum DefFuncMode {
			dfmNormal,
			dfmInterface,
			dfmType,
			dfmValue,
		};

		Node* parseBlock(string end = "}", string sep = ";");
		Node* parsePrimaryExpr();
		Node* parseFuncCall();
		Node* parseSubscript();
		Node* parseParenExpr();
		Node* parseExpression(bool enableEmpty = false);
		Node* parseDeclVar(bool inLoop = false);
		Node* parseDeclConst(bool inLoop = false);
		Node* parseIfExpr();
		void parseAssign(Node* block);
		//Node* parseWhileLoop();
		//Node* parseForLoop();
		Node* parseLoop();
		Node* parseTypeName();
		Node* parseType();
		Node* parseField();
		Node* parseDefineName();
		Node* parseDefineFunc(DefFuncMode mode = dfmNormal);
		Node* parseDefineStruct();
		Node* parseDefineImpl();
		Node* parseDefineInterface();

		Node* buildTree(vector<Node*> list, int begin, int end);
	};
	

	
}