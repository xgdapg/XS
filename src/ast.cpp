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
		root = parseBlock();
	}

	AST::Node* AST::parseBlock() {
		auto block = new Node(new Token(Token::Kind::kBlock, Token::Type::tUnknown, "Block", tk()->row, tk()->col));
		if (tk()->isOperator("{")) index += 1;

		while (tk() != Token::Empty) {
			auto t = tk();
			if (t->isKeyword("var")) {
				block->addChild(parseDeclVar());
				continue;
			}

			if (t->isKeyword("if")) {
				block->addChild(parseIfExpr());
				continue;
			}

			if (t->isOperator("=")) {
				parseAssign(block);
				continue;
			}

			if (t->isOperator("{")) {
				block->addChild(parseBlock());
				continue;
			}
			if (t->isOperator("}")) {
				index += 1;
				break;
			}
			//...
			auto n = parseExpression();
			if (n != nullptr) {
				block->addChild(n);
				continue;
			}

			throwException(t, "unmatched statement, got `" + t->value + "`");
		}
		return block;
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
			throwException(tk(), "expect `,` or `)`, got `" + tk()->value + "`");
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
			throwException(tk(), "expect `]`, got `" + tk()->value + "`");
		}
		index += 1;
		return node;
	}

	AST::Node* AST::parseParenExpr() {
		index += 1;
		auto node = parseExpression();

		if (!tk()->isOperator(")")) {
			throwException(tk(), "expect `)`, got `" + tk()->value + "`");
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
				if (isOp && !t->convertToUnaryOperator()) throwException(t, "expect unary operator, got `" + t->value + "`");
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
		if (list.empty()) {
			throwException(tk(), "expect expression, got `" + tk()->value + "`");
		}

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
				throwException(op->token, "unexpected operator `" + op->token->value + "`");
			}
			return op;
		}
		return list[begin];
	}

	AST::Node* AST::parseDeclVar() {
		auto dv = new Node(tk());
		index += 1;

		auto name = tk();
		if (!name->isIdentifier()) {
			throwException(name, "expect identifier, got `" + name->value + "`");
		}

		dv->addChild(new Node(name));
		index += 1;

		if (!(tk()->isOperator(":") && tk(1)->isIdentifier()) && !tk()->isOperator("=")) {
			throwException(tk(-1), "cannot determine the type of variable `" + tk(-1)->value + "`");
		}

		if (tk()->isOperator(":") && tk(1)->isIdentifier()) {
			dv->addChild(new Node(tk(1)));
			index += 2;
		}

		return dv;
	}

	AST::Node* AST::parseIfExpr() {
		auto ifExpr = new Node(tk());
		index += 1;

		ifExpr->addChild(parseExpression());

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}

		ifExpr->addChild(parseBlock());
		
		if (tk()->isKeyword("else")) {
			index += 1;
			if (tk()->isKeyword("if")) {
				ifExpr->addChild(parseIfExpr());
			} else if (tk()->isOperator("{")) {
				ifExpr->addChild(parseBlock());
			} else {
				throwException(tk(), "expect `{` or `if`, got `" + tk()->value + "`");
			}
		}

		return ifExpr;
	}

	AST::Node* AST::parseAssign(Node* block) {
		if (block->children.empty()) throwException(tk(), "lvalue not found");

		auto lv = block->children.back();
		//if (lv->token->isLiteral()) throwException(tk(), "invalid lvalue");

		auto assign = new Node(tk());
		index += 1;

		assign->addChild(lv);
		lv->parent = assign;
		block->children[block->children.size() - 1] = assign;

		assign->addChild(parseExpression());

		return assign;
	}

	
	

}