#pragma once

#include <string>

using namespace std;

namespace Lang {
	class Token {
	public:
		enum Kind {
			kUnknown,
			kKeyword,
			kLiteral,
			kIdentifier,
			kOperator,
			kComment,
		};

		enum Type {
			tUnknown,

			//Literal
			tInteger,
			tFloat,
			tString,
			tBoolean,

			//Identifier
			tVarType,
			tVarName,

			//Comment
			tLineComment,
			tBlockComment,

			//Operator
			tPlus, // +
			tMinus, // -
			tMulit, // *
			tDivide, // /

			tAssign, // =
			tEqual, // ==
			tLessThan, // <
			tLessEqual, // <=
			tNotEqual, // !=
			tGreaterThan, // >
			tGreaterEqual, // >=

			tLogicAnd, // &&
			tLogicOr, // ||
			tLogicNot, // !

			tBinOpAnd, // &
			tBinOpOr, // |
			tBinOpNot, // ~
			tBinOpXor, // ^
			tBinOpLShift, // <<
			tBinOpRShift, // >>

			tTilde, // ~
			tDot, // .
			tComma, // ,
			tSemicolon, // ;
			tColon, // :
			tRArrow, // ->
			tLArrow, // <-
			tFatArrow, // =>
			tPound, // #
			tDollar, // $
			tQuestion, // ?
			tUnderscore, // _

			tLParen, // (
			tRParen, // )
			tLBracket, // [
			tRBracket, // ]
			tLBrace, // {
			tRBrace, // }
		};

		Kind   kind  = Kind::kUnknown;
		Type   type  = Type::tUnknown;
		string value = "";
		int    row   = 0;
		int    col   = 0;

		Token(Kind k, Type t, string v, int r, int c) {
			kind  = k;
			type  = t;
			value = v;
			row   = r;
			col   = c;
		}
	};
}