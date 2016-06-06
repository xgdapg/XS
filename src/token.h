#pragma once

#include <string>

using namespace std;

namespace Lang {
	class Token {
	public:
		enum Type {
			Unknown,

			Integer,
			Float,
			String,
			Boolean,
			Identifier,

			LineComment,
			BlockComment,

			Plus, // +
			Minus, // -
			Mulit, // *
			Divide, // /

			Assign, // =
			Equal, // ==
			LessThan, // <
			LessEqual, // <=
			NotEqual, // !=
			GreaterThan, // >
			GreaterEqual, // >=

			LogicAnd, // &&
			LogicOr, // ||
			LogicNot, // !

			BinOpAnd, // &
			BinOpOr, // |
			BinOpNot, // ~
			BinOpXor, // ^
			BinOpLShift, // <<
			BinOpRShift, // >>

			Tilde, // ~
			Dot, // .
			Comma, // ,
			Semicolon, // ;
			Colon, // :
			RArrow, // ->
			LArrow, // <-
			FatArrow, // =>
			Pound, // #
			Dollar, // $
			Question, // ?
			Underscore, // _

			LParen, // (
			RParen, // )
			LBracket, // [
			RBracket, // ]
			LBrace, // {
			RBrace, // }
		};

		Type   type  = Type::Unknown;
		string value = "";
		int    row   = 0;
		int    col   = 0;

		Token(Type t, string v, int r, int c) {
			type = t;
			value = v;
			row = r;
			col = c;
		}
	};
}