#include "token.h"
#include "lexer.h"

namespace Lang {
	Token* Token::Empty = new Token(Token::Kind::kUnknown, Token::Type::tUnknown, "__EMPTY__", 0, 0);

	Token* Token::next(int offset) {
		if (lex != nullptr) {
			int i = index + offset;
			if (i >= 0 && i < (int)lex->tokens.size()) {
				return lex->tokens[i];
			}
		}
		return Empty;
	}
}