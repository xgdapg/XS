#pragma once

#include <string>
#include <vector>
#include "token.h"

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

		Node* Root;
		vector<Token*> tokens;
		unsigned int index;
		Token* emptyToken;

		AST(vector<Token*> ts) {
			Root = new Node();
			tokens = ts;
			index = 0;
			emptyToken = new Token(Token::Kind::kUnknown, Token::Type::tUnknown, "", 0, 0);
		}

		Token* token(unsigned int i = 0);

		void buildWithTokens(vector<Token*> tokens);

		Node* createDeclareVar();
		Node* createAssign();
		Node* createExpr();
	};
	

	
}