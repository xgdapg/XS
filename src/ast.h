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
			Token* token;
			vector<Node*> children;

			Node() {
				token = nullptr;
				children = vector<Node*>();
			}

			Node(Token* t) {
				token = t;
				children = vector<Node*>();
			}
		};

		Node* Root;
		AST() {
			Root = new Node();
		}

		void buildWithTokens(vector<Token*> tokens);
	};
	

	
}