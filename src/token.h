#pragma once

#include <string>
#include <iostream>

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
			kUnaryOperator,
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
			tUnaryPlus, // +
			tUnaryMinus, // -

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

			tRef, // &

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
			case Token::Type::tNotEqual:
			case Token::Type::tLessThan:
			case Token::Type::tLessEqual:
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
			case Token::Type::tBinOpNot:
			case Token::Type::tUnaryMinus:
			case Token::Type::tUnaryPlus:;
			case Token::Type::tRef: return true;
			}
			return false;
		}

		bool convertToUnaryOperator() {
			switch (type) {
			case Token::Type::tLogicNot:
				return true;
			case Token::Type::tMinus:
				type = Token::Type::tUnaryMinus;
			case Token::Type::tUnaryMinus:
				return true;
			case Token::Type::tPlus:
				type = Token::Type::tUnaryPlus;
			case Token::Type::tUnaryPlus:
				return true;
			case Token::Type::tBinOpNot:
				return true;
			case Token::Type::tRef:
				return true;
			}
			return false;
		}

		int getPriority() {
			int priority = 1;
			if (type == Token::Type::tLogicOr) return priority;
			priority++;
			if (type == Token::Type::tLogicAnd) return priority;
			priority++;
			if (type == Token::Type::tEqual) return priority;
			if (type == Token::Type::tNotEqual) return priority;
			if (type == Token::Type::tLessThan) return priority;
			if (type == Token::Type::tLessEqual) return priority;
			if (type == Token::Type::tGreaterThan) return priority;
			if (type == Token::Type::tGreaterEqual) return priority;
			priority++;
			if (type == Token::Type::tMinus) return priority;
			if (type == Token::Type::tPlus) return priority;
			priority++;
			if (type == Token::Type::tMulit) return priority;
			if (type == Token::Type::tDivide) return priority;
			priority++;
			if (isUnaryOperator()) return priority;
			priority++;
			if (type == Token::Type::tDot) return priority;
			priority++;
			//error
			cout << "error: operator " << value << " has no priority" << endl;
			return 0;
		}
	};
}