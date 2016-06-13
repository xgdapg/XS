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

			//Keyword
			tDefineFn,
			tDefineStruct,
			tDefineInterface,

			tDeclareVar,
			tDeclareConst,

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

			tFuncCall, //
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

		//二元操作符
		bool isBinaryOperator() {
			switch (type) {
			case Token::Type::tPlus:
			case Token::Type::tMinus:
			case Token::Type::tMulit:
			case Token::Type::tDivide:
			case Token::Type::tEqual:
			case Token::Type::tLessThan:
			case Token::Type::tLessEqual:
			case Token::Type::tNotEqual:
			case Token::Type::tGreaterThan:
			case Token::Type::tGreaterEqual:
			case Token::Type::tLogicAnd:
			case Token::Type::tLogicOr:;
			case Token::Type::tDot: return true;
			}
			return false;
		}

		//一元操作符
		bool isUnaryOperator() {
			switch (type) {
			case Token::Type::tLogicNot:
			case Token::Type::tMinus:
			case Token::Type::tPlus:
			case Token::Type::tBinOpNot: return true;
			}
			return false;
		}

		int getPriority() {
			switch (type) {
			case Token::Type::tLogicOr: return 1;
			case Token::Type::tLogicAnd: return 2;
			case Token::Type::tMinus:
			case Token::Type::tPlus: return 3;
			case Token::Type::tMulit:
			case Token::Type::tDivide: return 4;
			case Token::Type::tLogicNot: return 5;
			case Token::Type::tDot: return 6;
			}
			return 1;
		}
	};
}