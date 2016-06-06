#include "ast.h"

namespace Lang {
	void AST::buildWithTokens(vector<Token*> tokens) {
		unsigned int i = 0;
		while (i < tokens.size()) {
			auto token = tokens[i];
			
			if (token->type == Token::Token::tDeclareVar) {
				
			}
		}
	}
}