#include "ast.h"
#include <iostream>
#include <stack>

namespace Lang {
	void throwException(Token* t, string e) {
		cout << "[" << t->row << "," << t->col << "] ";
		throw exception(e.c_str());
	}

	Token* AST::tk(int offset) {
		int i = index + offset;
		if (i >= 0 && i < (int)lex->tokens.size()) return lex->tokens[i];
		return Token::Empty;
	}

	void AST::parse() {
		while (tk() != Token::Empty) {
			auto t = tk();
			if (t->isKeyword("var")) {
				root->addChild(parseDeclVar());
				continue;
			}
			//...
			auto n = parseExpression();
			if (n != nullptr) {
				root->addChild(n);
			}
		}
	}

	AST::Node* AST::parsePrimaryExpr() {
		auto t = tk();
		if (t->isLiteral()) {
			index += 1;
			return new Node(t);
		}
		if (t->isIdentifier()) {
			if (tk(1)->isOperator("(")) {
				return parseFuncCall();
			}
			if (tk(1)->isOperator("[")) {
				return parseSubscript();
			}
			index += 1;
			return new Node(t);
		}
		if (t->isOperator("(")) {
			return parseParenExpr();
		}
		return nullptr;
	}

	AST::Node* AST::parseFuncCall() {
		auto name = tk();
		auto node = new Node(new Token(Token::Kind::kOperator, Token::Type::tFuncCall, "FuncCall", name->row, name->col));
		node->addChild(new Node(name));
		index += 2;

		while (!tk()->isOperator(")")) {
			node->addChild(parseExpression());
			if (tk()->isOperator(",")) { index += 1; continue; }
			if (tk()->isOperator(")")) break;
			throwException(tk(), "expect , or ), got " + tk()->value);
		}
		index += 1;
		return node;
	}

	AST::Node* AST::parseSubscript() {
		auto name = tk();
		auto node = new Node(new Token(Token::Kind::kOperator, Token::Type::tSubscript, "Subscript", name->row, name->col));
		node->addChild(new Node(name));

		index += 2;
		node->addChild(parseExpression());

		if (!tk()->isOperator("]")) {
			throwException(tk(), "expect ], got " + tk()->value);
		}
		index += 1;
		return node;
	}

	AST::Node* AST::parseParenExpr() {
		index += 1;
		auto node = parseExpression();

		if (!tk()->isOperator(")")) {
			throwException(tk(), "expect ), got " + tk()->value);
		}

		index += 1;
		return node;
	}

	AST::Node* AST::parseExpression() {
		auto list = vector<Node*>();
		bool isOp = true;
		while (tk() != Token::Empty) {
			auto t = tk();
			if (t->isBinaryOperator() || t->isUnaryOperator()) {
				if (isOp) t->convertToUnaryOperator();
				isOp = true;

				list.push_back(new Node(t));
				index += 1;
				continue;
			}
			isOp = false;

			auto expr = parsePrimaryExpr();
			if (expr != nullptr) {
				list.push_back(expr);
				continue;
			}
			break;
		}
		if (list.empty()) return nullptr;

		return buildTree(list, 0, list.size() - 1);
	}

	AST::Node * AST::buildTree(vector<Node*> list, int begin, int end) {
		int opi = -1;
		int priority = 99;
		for (auto i = begin; i <= end; i++) {
			if (!list[i]->children.empty()) continue;
			auto t = list[i]->token;
			if (t->isUnaryOperator()  && t->getPriority() <  priority ||
				t->isBinaryOperator() && t->getPriority() <= priority)
			{
				opi = i;
				priority = t->getPriority();
			}
		}
		if (opi != -1) {
			auto op = list[opi];
			if (op->token->isBinaryOperator()) {
				op->addChild(buildTree(list, begin, opi - 1));
				op->addChild(buildTree(list, opi + 1, end));
			} else if (op->token->isUnaryOperator()) {
				op->addChild(buildTree(list, opi + 1, end));
			} else {
				//error
				throwException(op->token, "error: unexpected operator " + op->token->value);
			}
			return op;
		}
		return list[begin];
	}

	AST::Node* AST::parseDeclVar() {
		index += 1;

		auto name = tk();
		if (!name->isIdentifier()) {
			throwException(name, "expect identifier, got " + name->value);
		}

		auto dv = new Node(new Token(Token::Kind::kOperator, Token::Type::tDeclareVar, "DeclareVar", name->row, name->col));
		dv->addChild(new Node(name));
		index += 1;

		if (tk()->isOperator(":") && tk(1)->isIdentifier()) {
			dv->addChild(new Node(tk(1)));
			index += 2;
		}

		if (tk()->isOperator("=")) {
			auto assign = new Node(tk());
			index += 1;
			assign->addChild(dv);
			auto t = tk();
			auto e = parseExpression();
			if (e == nullptr) {
				throwException(t, "expect expression, got " + t->value);
			}
			assign->addChild(e);
			return assign;
		}

		return dv;
	}

	
	

}