#include "ast.h"
#include <iostream>
#include <stack>

namespace Lang {
	void throwException(Token* t, string e) {
		cout << "[" << t->row << "," << t->col << "] ";
		throw exception(e.c_str());
	}

	Token * AST::token(unsigned int i) {
		if (i >= 0 && i < lex->tokens.size()) return lex->tokens[i];
		return Token::Empty;
	}

	/*
	void AST::buildWithTokens(vector<Token*> tokens) {
		while (token() != emptyToken) {
			if (token()->type == Token::Type::tDeclareVar) {
				Root->addChild(createDeclareVar());
			}
			if (token()->type == Token::Type::tAssign) {
				Root->addChild(createAssign());
			}
		}
	}

	AST::Node* AST::createDeclareVar() {
		auto node = new Node(token());
		index++;
		if (token()->kind != Token::Kind::kIdentifier) throw exception("require identifier");
		node->addChild(new Node(token()));
		index++;
		if (token()->type == Token::Type::tSemicolon && token(1)->kind == Token::Kind::kIdentifier) {
			node->addChild(new Node(token(1)));
			index += 2;
		} else {
			node->addChild(new Node(new Token(Token::Kind::kIdentifier, Token::Type::tVarType, "unknown", 0, 0)));
		}
		return node;
	}

	AST::Node* AST::createAssign() {
		auto node = new Node(token());
		index++;
		return node;
	}

	AST::Node* AST::createExpr() {
		Node* node = nullptr;
		auto t = token();
		if (t->kind == Token::Kind::kLiteral || t->kind == Token::Kind::kIdentifier) {
			if (token(1)->kind == Token::Kind::kOperator) {
				node = new Node(token(1));
				node->addChild(new Node(t));
				index += 2;
				node->addChild(createExpr());
			} else {
				node = new Node(t);
				index += 1;
			}
		}
		else if (t->type == Token::Type::tLParen) {

		}
		return node;
	}
	*/





	void AST::parse() {
		index = 0;
		while (index < (int)lex->tokens.size()) {
			auto t = token(index);

			if (t->isKeyword("var")) {
				root->addChild(parseDeclVar(index));
				continue;
			}

			int i = detectExprEnd(index);
			cout << "detectExprEnd " << index << " " << i << endl;
			if (i > index) {
				root->addChild(parseExpr(index, i));
				index = i;
				continue;
			}

			break;
		}
	}

	AST::Node* AST::parseDeclVar(unsigned int begin) {
		auto t = token(begin);
		if (!t->next()->isIdentifier()) {
			throwException(t->next(), "expect identifier, got " + t->next()->value);
		}

		auto name = t->next();
		auto tb = new Node(new Token(
			Token::Kind::kOperator,
			Token::Type::tTypeBinding,
			"TypeBinding",
			name->row, name->col));
		tb->addChild(new Node(name));

		int i = name->index + 1;
		if (name->next()->isOperator(":") && name->next(2)->isIdentifier()) {
			tb->addChild(new Node(name->next(2)));
			i += 2;
		}
		index = i;

		if (token(i)->isOperator("=")) {
			auto assign = new Node(token(i));
			assign->addChild(tb);
			int j = detectExprEnd(i + 1);
			if (j <= i + 1) {
				throwException(token(i + 1), "expect expression, got " + token(i + 1)->value);
			}
			assign->addChild(parseExpr(i + 1, j));
			index = j;
			return assign;
		}

		return tb;
	}

	int AST::detectExprEnd(unsigned int begin) {
		enum State {
			sEvaluable,
			sOperator,
		};
		auto state = sEvaluable;
		auto i = begin;
		auto t = token(i);
		while (t != Token::Empty) {
			if (state == sEvaluable) {
				if (t->isLiteral()) {
					t = token(++i);
					if (t->isBinaryOperator()) {
						state = sOperator;
						continue;
					}
					break;
				} else if (t->isIdentifier()) {
					if (token(i + 1)->isOperator("(")) { //function
						for (i += 2; i < lex->tokens.size(); i++) {
							i = detectExprEnd(i);
							if (token(i)->isOperator(",")) continue;
							if (token(i)->isOperator(")")) break;
						}
						if (!token(i)->isOperator(")")) {
							throwException(t, "error: unclosed function call");
						}
					} else if (token(i + 1)->isOperator("[")) { //subscript
						i = detectExprEnd(i + 2);
						if (!token(i)->isOperator("]")) {
							throwException(t, "error: unclosed subscript");
						}
					} else { //variable

					}

					t = token(++i);
					if (t->isBinaryOperator()) {
						state = sOperator;
						continue;
					}
					break;
				} else if (t->isOperator("(")) {
					i = detectExprEnd(i + 1);

					t = token(++i);
					if (t->isBinaryOperator()) {
						state = sOperator;
						continue;
					}
					break;
				} else if (t->convertToUnaryOperator()) {
					t = token(++i);
					continue;
				} else {
					//error
					//throwException(t, "error: expect evaluable token, got " + t->value);
					break;
				}
			}

			else if (state == sOperator) {
				t = token(++i);
				state = sEvaluable;
				continue;
			}

			else {
				throwException(t, "error: unknown state");
				break;
			}
		}

		return i;
	}

	AST::Node* AST::parseExpr(unsigned int begin, unsigned int end) {
		int opi = EOF;
		int priority = 99;
		for (auto i = begin; i < end; i++) {
			auto t = token(i);
			if (t->isLiteral()) {
				continue;
			}
			if (t->isIdentifier()) {
				if (token(i + 1)->isOperator("(")) { //function
					int pcnt = 0;
					for (auto j = i + 1; j < end; j++) {
						if (token(j)->isOperator("(")) pcnt++;
						if (token(j)->isOperator(")")) pcnt--;
						if (pcnt == 0) {
							i = j;
							break;
						}
					}
				} else if (token(i + 1)->isOperator("[")) { //subscript
					int pcnt = 0;
					for (auto j = i + 1; j < end; j++) {
						if (token(j)->isOperator("[")) pcnt++;
						if (token(j)->isOperator("]")) pcnt--;
						if (pcnt == 0) {
							i = j;
							break;
						}
					}
				} else { //variable

				}
				continue;
			}
			if (t->isOperator("(")) {
				int pcnt = 0;
				for (auto j = i; j < end; j++) {
					if (token(j)->isOperator("(")) pcnt++;
					if (token(j)->isOperator(")")) pcnt--;
					if (pcnt == 0) {
						i = j;
						break;
					}
				}
				continue;
			}
			
			if (t->isUnaryOperator()  && t->getPriority() <  priority ||
				t->isBinaryOperator() && t->getPriority() <= priority)
			{
				opi = i;
				priority = t->getPriority();
			}
		}

		if (opi != EOF) {
			auto op = token(opi);
			auto node = new Node(op);
			if (op->isBinaryOperator()) {
				node->addChild(parseExpr(begin, opi));
				node->addChild(parseExpr(opi + 1, end));
			}
			else if (op->isUnaryOperator()) {
				node->addChild(parseExpr(opi + 1, end));
			}
			else {
				//error
				throwException(op, "error: unexpected operator " + op->value);
			}
			return node;
		} else {
			auto t = token(begin);
			if (t->isLiteral()) {
				return new Node(t);
			}
			if (t->isIdentifier()) {
				if (token(begin + 1)->isOperator("(")) { //function
					return parseFunc(begin, end);
				}
				else if (token(begin + 1)->isOperator("[")) { //subscript
					return parseSubscript(begin, end);
				}
				else { //variable
					return new Node(t);
				}
			}
			if (t->isOperator("(")) {
				return parseExpr(begin + 1, end - 1);
			}
			//error
			throwException(t, "error: unexpected token " + t->value);
		}
		return nullptr;
	}

	AST::Node* AST::parseFunc(unsigned int begin, unsigned int end) {
		auto name = token(begin);
		auto node = new Node(new Token(Token::Kind::kOperator, Token::Type::tFuncCall, "FuncCall", name->row, name->col));
		node->addChild(new Node(name));
		begin += 2;
		for (auto i = begin; i < end; i++) {
			auto t = token(i);
			if (t->isOperator("(")) {
				int pcnt = 0;
				for (auto j = i; j < end; j++) {
					if (token(j)->isOperator("(")) pcnt++;
					if (token(j)->isOperator(")")) pcnt--;
					if (pcnt == 0) {
						i = j;
						break;
					}
				}
				continue;
			}
			if (t->isOperator(",") || t->isOperator(")")) {
				node->addChild(parseExpr(begin, i));
				begin = i + 1;
			}
		}
		return node;
	}

	AST::Node* AST::parseSubscript(unsigned int begin, unsigned int end) {
		auto name = token(begin);
		auto node = new Node(new Token(Token::Kind::kOperator, Token::Type::tSubscript, "Subscript", name->row, name->col));
		node->addChild(new Node(name));
		node->addChild(parseExpr(begin + 2, end - 1));
		return node;
	}
	

}