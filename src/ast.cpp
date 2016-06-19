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

	AST::Node* AST::parseBlock(string end/*="}"*/, string sep/*=";"*/) {
		auto block = new Node(new Token(Token::Kind::kBlock, Token::Type::tUnknown, "Block", tk()->row, tk()->col));
		if (tk()->isOperator("{")) index += 1;

		while (tk() != Token::Empty) {
			auto t = tk();
			if (t->isOperator(end)) {
				index += 1;
				break;
			}

			if (t->isOperator(sep)) {
				index += 1;
				continue;
			}

			if (t->isKeyword("var")) {
				block->addChild(parseDeclVar());
				continue;
			}

			if (t->isKeyword("const")) {
				block->addChild(parseDeclConst());
				continue;
			}

			if (t->isKeyword("if")) {
				block->addChild(parseIfExpr());
				continue;
			}

			if (t->isKeyword("while")) {
				block->addChild(parseWhileLoop());
				continue;
			}

			if (t->isKeyword("for")) {
				block->addChild(parseForLoop());
				continue;
			}

			if (t->isKeyword("fn")) {
				block->addChild(parseDefineFunc());
				continue;
			}

			if (t->isKeyword("struct")) {
				block->addChild(parseDefineStruct());
				continue;
			}

			if (t->isKeyword("impl")) {
				block->addChild(parseDefineImpl());
				continue;
			}

			if (t->isKeyword("return")) {
				auto node = new Node(tk());
				index += 1;
				auto expr = parseExpression(true);
				if (expr != nullptr) {
					node->addChild(expr);
				}
				block->addChild(node);
				continue;
			}

			if (t->isOperator("=") || 
				t->isOperator("+=") || 
				t->isOperator("-=") || 
				t->isOperator("*=") || 
				t->isOperator("/=") || 
				t->isOperator("%=")) {
				parseAssign(block);
				continue;
			}

			if (t->isOperator("{")) {
				block->addChild(parseBlock());
				continue;
			}
			//...
			auto n = parseExpression(true);
			if (n != nullptr) {
				block->addChild(n);
				continue;
			}

			throwException(t, "unmatched statement, got `" + t->value + "`");
		}
		return block;
	}

	AST::Node* AST::parseFuncCall() {
		auto node = new Node(new Token(Token::Kind::kOperator, Token::Type::tFnCallParams, "FnCallParams", tk()->row, tk()->col));
		index += 1; // eat (
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
		index += 1; // eat [
		auto expr = parseExpression();

		if (!tk()->isOperator("]")) {
			throwException(tk(), "expect `]`, got `" + tk()->value + "`");
		}
		index += 1;

		return expr;
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

	AST::Node* AST::parsePrimaryExpr() {
		auto t = tk();
		if (t->isLiteral() || t->isIdentifier()) {
			index += 1;
			return new Node(t);
		}
		if (t->isOperator("(")) {
			return parseParenExpr();
		}
		return nullptr;
	}

	AST::Node* AST::parseExpression(bool enableEmpty/*=false*/) {
		enum State {
			sOperator,
			sPrimaryExpr,
		};

		auto list = vector<Node*>();
		State state = sOperator;

		while (tk() != Token::Empty) {
			auto t = tk();
			if (t->isBinaryOperator() || t->isUnaryOperator()) {
				if (state == sOperator && !t->convertToUnaryOperator()) throwException(t, "expect unary operator, got `" + t->value + "`");
				list.push_back(new Node(t));
				state = sOperator;
				index += 1;
				continue;
			}
			if (t->isOperator("(") && state == sPrimaryExpr) {
				list.push_back(new Node(new Token(Token::Kind::kOperator, Token::Type::tFnCall, "FnCall", t->row, t->col)));
				list.push_back(parseFuncCall());
				state = sPrimaryExpr;
				continue;
			}
			if (t->isOperator("[") && state == sPrimaryExpr) {
				list.push_back(new Node(new Token(Token::Kind::kOperator, Token::Type::tSubscript, "Subscript", t->row, t->col)));
				list.push_back(parseSubscript());
				state = sPrimaryExpr;
				continue;
			}
			if (state == sPrimaryExpr) break;
			auto expr = parsePrimaryExpr();
			if (expr != nullptr) {
				list.push_back(expr);
				state = sPrimaryExpr;
				continue;
			}
			break;
		}
		if (list.empty()) {
			if (enableEmpty) return nullptr;
			throwException(tk(), "expect expression, got `" + tk()->value + "`");
		}
		if (state == sOperator) {
			throwException(list.back()->token, "incomplete expression, end with `" + list.back()->token->value + "`");
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
		auto node = new Node(tk());
		index += 1;

		auto name = tk();
		if (!name->isIdentifier()) {
			throwException(name, "expect identifier, got `" + name->value + "`");
		}

		node->addChild(new Node(name));
		index += 1;

		if (!(tk()->isOperator(":") && tk(1)->isIdentifier()) && !tk()->isOperator("=") && !tk()->isKeyword("in")) {
			throwException(name, "cannot determine the type of variable `" + name->value + "`");
		}

		if (tk()->isOperator(":") && tk(1)->isIdentifier()) {
			node->addChild(new Node(tk(1)));
			index += 2;
		} else {
			node->addChild(new Node(Token::Empty));
		}

		return node;
	}

	AST::Node* AST::parseDeclConst() {
		auto node = new Node(tk());
		index += 1;
		
		auto name = tk();
		if (!name->isIdentifier()) {
			throwException(name, "expect identifier, got `" + name->value + "`");
		}

		node->addChild(new Node(name));
		index += 1;

		if (tk()->isOperator(":") && tk(1)->isIdentifier()) {
			node->addChild(new Node(tk(1)));
			index += 2;
		}

		if (!tk()->isOperator("=")) {
			throwException(name, "constant value required");
		}

		return node;
	}

	AST::Node* AST::parseIfExpr() {
		auto node = new Node(tk());
		index += 1;

		node->addChild(parseExpression());

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}

		node->addChild(parseBlock());
		
		if (tk()->isKeyword("else")) {
			index += 1;
			if (tk()->isKeyword("if")) {
				node->addChild(parseIfExpr());
			} else if (tk()->isOperator("{")) {
				node->addChild(parseBlock());
			} else {
				throwException(tk(), "expect `{` or `if`, got `" + tk()->value + "`");
			}
		}

		return node;
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

	AST::Node* AST::parseWhileLoop() {
		auto node = new Node(tk());
		index += 1;

		node->addChild(parseExpression());

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}

		node->addChild(parseBlock());

		return node;
	}

	AST::Node* AST::parseForLoop() {
		auto node = new Node(tk());
		index += 1;

		node->addChild(parseBlock(";", ","));

		node->addChild(parseExpression());
		if (!tk()->isOperator(";")) {
			throwException(tk(), "expect `;`, got `" + tk()->value + "`");
		}
		index += 1;

		node->addChild(parseBlock("{", ","));

		node->addChild(parseBlock());

		return node;
	}

	AST::Node* AST::parseType() {
		Node* ref = nullptr;
		Node* name = nullptr;
		Node* subscript = nullptr;

		if (tk()->isOperator("&")) {
			ref = new Node(tk());
			index += 1;
		} else {
			ref = new Node(Token::Empty);
		}

		if (tk()->isIdentifier()) {
			name = new Node(tk());
			index += 1;
		} else {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}

		if (tk()->isOperator("[")) {
			subscript = new Node(new Token(Token::Kind::kOperator, Token::Type::tSubscript, "Subscript", tk()->row, tk()->col));
			index += 1;
			if (tk()->isLiteral(Token::Type::tInteger)) {
				subscript->addChild(new Node(tk()));
				index += 1;
			}
			if (!tk()->isOperator("]")) {
				throwException(tk(), "expect `]`, got `" + tk()->value + "`");
			}
			index += 1;
		} else {
			subscript = new Node(Token::Empty);
		}

		name->addChild(ref);
		name->addChild(subscript);

		return name;
	}

	AST::Node* AST::parseField() {
		auto node = new Node(new Token(Token::Kind::kKeyword, Token::Type::tField, "Field", tk()->row, tk()->col));

		auto flag = tk();
		while (1) {
			if (flag->isKeyword("const")) {
				//save the flag
				index += 1;
				continue;
			}
			break;
		}

		//name
		if (tk()->isIdentifier()) {
			node->addChild(new Node(tk()));
			index += 1;
		} else {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}

		if (!tk()->isOperator(":")) {
			throwException(tk(), "expect `:`, got `" + tk()->value + "`");
		}
		index += 1;

		node->addChild(parseType());

		//default value
		if (tk()->isOperator("=")) {
			node->addChild(parseExpression());
			index += 1;
		} else {
			node->addChild(new Node(Token::Empty));
		}

		return node;
	}

	AST::Node* AST::parseDefineFunc() {
		auto node = new Node(tk());
		index += 1;
		
		//name
		if (tk()->isIdentifier()) {
			node->addChild(new Node(tk()));
			index += 1;
		} else {
			node->addChild(new Node(Token::Empty));
		}

		if (!tk()->isOperator("(")) {
			throwException(tk(), "expect `(`, got `" + tk()->value + "`");
		}
		index += 1;

		auto params = new Node(new Token(Token::Kind::kBlock, Token::Type::tUnknown, "Params", tk()->row, tk()->col));
		node->addChild(params);
		while (!tk()->isOperator(")")) {
			if (tk()->isOperator(",")) { index += 1; continue; }
			params->addChild(parseField());
		}

		index += 1; // eat )

		if (tk()->isOperator("->")) {
			index += 1;
			if (tk()->isIdentifier()) {
				node->addChild(new Node(tk()));
				index += 1;
			} else {
				throwException(tk(), "expect function return type, got `" + tk()->value + "`");
			}
		} else {
			node->addChild(new Node(Token::Empty));
		}

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}
		node->addChild(parseBlock());

		return node;
	}

	AST::Node* AST::parseDefineStruct() {
		auto node = new Node(tk());
		index += 1;

		//name
		if (tk()->isIdentifier()) {
			node->addChild(new Node(tk()));
			index += 1;
		} else {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}
		index += 1;

		while (!tk()->isOperator("}")) {
			if (tk()->isOperator(",")) { index += 1; continue; }
			node->addChild(parseField());
		}
		index += 1; // eat }

		return node;
	}
	
	AST::Node* AST::parseDefineImpl() {
		auto node = new Node(tk());
		index += 1;

		//name
		if (tk()->isIdentifier()) {
			node->addChild(new Node(tk()));
			index += 1;
		} else {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}

		//interface
		if (tk()->isOperator(":") && tk(1)->isIdentifier()) {
			node->addChild(new Node(tk(1)));
			index += 2;
		} else {
			node->addChild(new Node(Token::Empty));
		}

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}
		index += 1;

		while (!tk()->isOperator("}")) {
			auto t = tk();
			if (t->isKeyword("fn")) {
				node->addChild(parseDefineFunc());
				continue;
			}

			throwException(t, "expect function definition, got `" + t->value + "`");
		}
		index += 1; // eat }

		return node;
	}
}