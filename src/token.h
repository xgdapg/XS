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
			case Type::tPlus:
			case Type::tMinus:
			case Type::tMulit:
			case Type::tDivide:
			case Type::tEqual:
			case Type::tNotEqual:
			case Type::tLessThan:
			case Type::tLessEqual:
			case Type::tGreaterThan:
			case Type::tGreaterEqual:
			case Type::tLogicAnd:
			case Type::tLogicOr:;
			case Type::tDot: return true;
			}
			return false;
		}

		//一元操作符
		bool isUnaryOperator() {
			switch (type) {
			case Type::tLogicNot:
			case Type::tBinOpNot:
			case Type::tUnaryMinus:
			case Type::tUnaryPlus:;
			case Type::tRef: return true;
			}
			return false;
		}

		bool convertToUnaryOperator() {
			if (type == Type::tMinus) type = Type::tUnaryMinus;
			if (type == Type::tPlus) type = Type::tUnaryPlus;
			return isUnaryOperator();
		}

		int getPriority() {
			int priority = 1;
			if (type == Type::tLogicOr) return priority;
			priority++;
			if (type == Type::tLogicAnd) return priority;
			priority++;
			if (type == Type::tEqual) return priority;
			if (type == Type::tNotEqual) return priority;
			if (type == Type::tLessThan) return priority;
			if (type == Type::tLessEqual) return priority;
			if (type == Type::tGreaterThan) return priority;
			if (type == Type::tGreaterEqual) return priority;
			priority++;
			if (type == Type::tMinus) return priority;
			if (type == Type::tPlus) return priority;
			priority++;
			if (type == Type::tMulit) return priority;
			if (type == Type::tDivide) return priority;
			priority++;
			if (isUnaryOperator()) return priority;
			priority++;
			if (type == Type::tDot) return priority;
			priority++;
			//error
			cout << "error: operator " << value << " has no priority" << endl;
			return 0;
		}

		bool isIdentifier() {
			return kind == Kind::kIdentifier;
		}

		bool isLiteral() {
			return kind == Kind::kLiteral;
		}

		bool isOperator() {
			return kind == Kind::kOperator;
		}

		bool isOperator(Type t) {
			return isOperator() && type == t;
		}

		bool isOperator(string t) {
			return isOperator() && value == t;
		}
	};
}