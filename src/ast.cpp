#include "ast.h"
#include <iostream>

namespace Lang {
	void throwException(Token* t, string e) {
		cout << "[" << t->row << "," << t->col << "] ";
		throw exception(e.c_str());
	}

	Token * AST::tokenAt(unsigned int i) {
		if (i >= 0 && i < tokens.size()) return tokens[i];
		return emptyToken;
	}

	Token * AST::token(unsigned int i) {
		return tokenAt(i + index);
	}

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

	AST::Node* AST::parseExpr(unsigned int begin, unsigned int end) {
		int opi = EOF;
		int priority = 99;
		for (auto i = begin; i < end; i++) cout << tokenAt(i)->value; cout << endl;
		for (auto i = begin; i < end; i++) {
			auto t = tokenAt(i);
			if (t->type == Token::Type::tLParen) {
				int pcnt = 0;
				for (auto j = i; j < end; j++) {
					if (tokenAt(j)->type == Token::Type::tLParen) pcnt++;
					if (tokenAt(j)->type == Token::Type::tRParen) pcnt--;
					if (pcnt == 0) {
						i = j;
						break;
					}
				}
				continue;
			}
			if (t->kind == Token::Kind::kOperator) {
				auto pt = tokenAt(i - 1);
				if (pt->kind != Token::Kind::kIdentifier &&
					pt->kind != Token::Kind::kLiteral &&
					pt->type != Token::Type::tRParen) {
					if (!t->convertToUnaryOperator()) {
						cout << "error: unexpected operator " << t->value << endl;
					}
				}
				if (t->isUnaryOperator() && t->getPriority() < priority ||
					t->isBinaryOperator() && t->getPriority() <= priority) {
					opi = i;
					priority = t->getPriority();
				}
			}
		}

		if (opi != EOF) {
			auto op = token(opi);
			cout << "op: " << op->value << " u:" << op->isUnaryOperator() << endl;
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
				cout << "error: unexpected operator " << op->value << endl;
			}
			return node;
		} else {
			auto t = tokenAt(begin);
			if (t->type == Token::Type::tLParen) {
				return parseExpr(begin + 1, end - 1);
			}
			else if (t->kind == Token::Kind::kIdentifier) {
				if (tokenAt(begin + 1)->type == Token::Type::tLParen) {
					return parseFunc(begin, end);
				} else {
					return new Node(t);
				}
			}
			else if (t->kind == Token::Kind::kLiteral) {
				return new Node(t);
			}
			else {
				//error
				cout << "error: unexpected token " << t->value << endl;
			}
		}
		return nullptr;
	}

	AST::Node* AST::parseFunc(unsigned int begin, unsigned int end) {
		auto name = tokenAt(begin);
		auto node = new Node(new Token(Token::Kind::kOperator, Token::Type::tFuncCall, "FuncCall", name->row, name->col));
		node->addChild(new Node(name));
		begin += 2;
		for (auto i = begin; i < end; i++) {
			auto t = tokenAt(i);
			if (t->type == Token::Type::tLParen) {
				int pcnt = 0;
				for (auto j = i; j < end; j++) {
					if (tokenAt(j)->type == Token::Type::tLParen) pcnt++;
					if (tokenAt(j)->type == Token::Type::tRParen) pcnt--;
					if (pcnt == 0) {
						i = j;
						break;
					}
				}
				continue;
			}
			if (t->type == Token::Type::tComma || t->type == Token::Type::tRParen) {
				node->addChild(parseExpr(begin, i));
				begin = i + 1;
			}
		}
		return node;
	}


	int AST::detectExprEnd(unsigned int begin) {
		enum State {
			sEvaluable,
			sOperator,
		};
		auto state = sEvaluable;
		auto i = begin;
		auto t = tokenAt(i);
		while (t != emptyToken) {
			if (state == sEvaluable) {
				if (t->isLiteral()) {
					t = tokenAt(++i);
					if (t->isBinaryOperator()) {
						state = sOperator;
						continue;
					}
					break;
				}
				else if (t->isIdentifier()) {
					if (tokenAt(i + 1)->isOperator("(")) { //function
						int pcnt = 0;
						for (auto j = i + 1; j < tokens.size(); j++) {
							if (tokenAt(j)->isOperator("(")) pcnt++;
							if (tokenAt(j)->isOperator(")")) pcnt--;
							if (pcnt == 0) {
								i = j;
								break;
							}
						}
					}
					else if (tokenAt(i + 1)->isOperator("[")) { //array
						int pcnt = 0;
						for (auto j = i + 1; j < tokens.size(); j++) {
							if (tokenAt(j)->isOperator("[")) pcnt++;
							if (tokenAt(j)->isOperator("]")) pcnt--;
							if (pcnt == 0) {
								i = j;
								break;
							}
						}
					}
					else { //variable

					}

					t = tokenAt(++i);
					if (t->isBinaryOperator()) {
						state = sOperator;
						continue;
					}
					break;
				}
				else if (t->isOperator("(")) {
					int pcnt = 0;
					for (auto j = i; j < tokens.size(); j++) {
						if (tokenAt(j)->isOperator("(")) pcnt++;
						if (tokenAt(j)->isOperator(")")) pcnt--;
						if (pcnt == 0) {
							i = j;
							break;
						}
					}

					t = tokenAt(++i);
					if (t->isBinaryOperator()) {
						state = sOperator;
						continue;
					}
					break;
				}
				else if (t->convertToUnaryOperator()) {
					t = tokenAt(++i);
					continue;
				}
				else {
					//error
					cout << "error: expect evaluable token, got " << t->value << endl;
					break;
				}
			}

			else if (state == sOperator) {
				t = tokenAt(++i);
				state = sEvaluable;
				continue;
			}

			else {
				cout << "error: unknown state" << state << endl;
				break;
			}
		}

		if (state != sEvaluable) {
			throwException(t, "incomplete expression");
		}
		return i;
	}
}