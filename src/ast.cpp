#include "ast.h"
#include <iostream>
#include <stack>

namespace Lang {
	void throwException(Token* t, string e) {
		cout << "[" << t->row << "," << t->col << "] ";
		throw exception(e.c_str());
	}

	AST::Node* newBlock(int row = 0, int col = 0) {
		return new AST::Node(new Token(Token::Kind::kBlock, Token::Type::tUnknown, "_BLOCK_", row, col));
	}

	Token* AST::tk(int offset) {
		int i = index + offset;
		if (i >= 0 && i < (int)lex->tokens.size()) return lex->tokens[i];
		return Token::Empty;
	}

	void AST::parse() {
		root = parseBlock();
		//checkNode(root);
	}

	AST::Node* AST::parseBlock(string end/*="}"*/, string sep/*=";"*/) {
		auto block = newBlock(tk()->row, tk()->col);
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

			if (t->isKeyword("loop")) {
				block->addChild(parseLoop());
				continue;
			}

			//if (t->isKeyword("while")) {
			//	block->addChild(parseWhileLoop());
			//	continue;
			//}
			//
			//if (t->isKeyword("for")) {
			//	block->addChild(parseForLoop());
			//	continue;
			//}

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

			if (t->isKeyword("interface")) {
				block->addChild(parseDefineInterface());
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

			if (t->isAssignOperator()) {
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
		auto node = new Node(new Token(Token::Kind::kOperator, Token::Type::tFnCallArgs, "_ARGS_", tk()->row, tk()->col));
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
		auto tp = tk();
		index += 1;
		auto node = parseExpression();

		if (tk()->isOperator(",")) {
			auto tuple = new Node(new Token(Token::Kind::kOperator, Token::Type::tTuple, "_TUPLE_", tp->row, tp->col));
			tuple->addChild(node);
			index += 1;
			while (!tk()->isOperator(")")) {
				if (tk()->isOperator(",")) { index += 1; continue; }
				tuple->addChild(parseExpression());
			}
			index += 1;
			return tuple;
		}

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
		if (t->isKeyword("fn")) {
			return parseDefineFunc(dfmValue);
		}
		if (t->isKeyword("var")) {
			return parseDeclVar();
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
				list.push_back(new Node(new Token(Token::Kind::kOperator, Token::Type::tFnCall, "_FNCALL_", t->row, t->col)));
				list.push_back(parseFuncCall());
				state = sPrimaryExpr;
				continue;
			}
			if (t->isOperator("[") && state == sPrimaryExpr) {
				list.push_back(new Node(new Token(Token::Kind::kOperator, Token::Type::tSubscript, "_SUBSCRIPT_", t->row, t->col)));
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
			if (t->isAssignOperator() && t->getPriority() <  priority ||
				t->isUnaryOperator()  && t->getPriority() <  priority ||
				t->isBinaryOperator() && t->getPriority() <= priority && !t->isAssignOperator())
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

	AST::Node* AST::parseDeclVar(bool inLoop/*=false*/) {
		auto node = new Node(tk());
		index += 1;

		auto name = tk();
		if (!name->isIdentifier()) {
			throwException(name, "expect identifier, got `" + name->value + "`");
		}

		node->addChild(new Node(name));
		index += 1;

		if (!tk()->isOperator(":") && !(!inLoop && tk()->isOperator("=")) && !(inLoop && tk()->isKeyword("in"))) {
			throwException(name, "cannot determine the type of variable `" + name->value + "`");
		}

		if (tk()->isOperator(":")) {
			index += 1;
			node->addChild(parseType());
		} else {
			node->addChild(new Node(Token::Empty));
		}

		return node;
	}

	AST::Node* AST::parseDeclConst(bool inLoop/*=false*/) {
		auto node = new Node(tk());
		index += 1;
		
		auto name = tk();
		if (!name->isIdentifier()) {
			throwException(name, "expect identifier, got `" + name->value + "`");
		}

		node->addChild(new Node(name));
		index += 1;

		if (tk()->isOperator(":")) {
			index += 1;
			node->addChild(parseType());
		} else {
			node->addChild(new Node(Token::Empty));
		}

		if (!(!inLoop && tk()->isOperator("=")) && !(inLoop && tk()->isKeyword("in"))) {
			throwException(name, "constant value required");
		}

		return node;
	}

	AST::Node* AST::parseIfExpr() {
		auto node = new Node(tk());
		index += 1;

		auto block = newBlock();
		node->addChild(block);

		block->addChild(parseExpression());

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}

		block->addChild(parseBlock());

		if (tk()->isKeyword("else")) {
			index += 1;
			if (tk()->isKeyword("if")) {
				block->addChild(parseIfExpr());
			} else if (tk()->isOperator("{")) {
				block->addChild(parseBlock());
			} else {
				throwException(tk(), "expect `{` or `if`, got `" + tk()->value + "`");
			}
		} else {
			block->addChild(newBlock());
		}

		return node;
	}

	void AST::parseAssign(Node* block) {
		if (block->children.empty()) throwException(tk(), "lvalue not found");

		auto assign = new Node(tk());
		index += 1;

		auto lv = block->children.back();
		//if (!lv->token->isIdentifier() &&
		//	!lv->token->isKeyword("var") &&
		//	!lv->token->isKeyword("const")) throwException(tk(), "invalid lvalue");

		assign->addChild(lv);
		lv->parent = assign;

		block->children[block->children.size() - 1] = assign;
		
		assign->addChild(parseExpression());
	}
	/*
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
	*/
	AST::Node* AST::parseLoop() {
		auto node = new Node(tk());
		index += 1;

		if (tk()->isOperator("{")) {
			node->addChild(parseBlock());
		}
		else if (tk()->isKeyword("if")) {
			auto block = newBlock();
			node->addChild(block);

			auto ifExpr = parseIfExpr();
			block->addChild(ifExpr);

			auto elseNode = ifExpr->children[0]->children[2];
			while (elseNode->token->isKeyword("if")) elseNode = elseNode->children[0]->children[2];
			elseNode->addChild(new Node(new Token(Token::Kind::kKeyword, Token::Type::tBreak, "break", 0, 0)));
		}
		else if (tk()->isKeyword("each")) {
			auto block = newBlock();
			node->addChild(block);

			auto each = new Node(tk());
			block->addChild(each);
			index += 1;

			if (tk()->isIdentifier()) {
				each->addChild(new Node(tk()));
				index += 1;
			}
			else if (tk()->isKeyword("var")) {
				each->addChild(parseDeclVar(true));
			}
			else if (tk()->isKeyword("const")) {
				each->addChild(parseDeclConst(true));
			}
			else {
				throwException(tk(), "expect `var` or `const` or identifier, got `" + tk()->value + "`");
			}

			if (!tk()->isKeyword("in")) {
				throwException(tk(), "expect `in`, got `" + tk()->value + "`");
			}
			index += 1;

			each->addChild(parseExpression());

			if (!tk()->isOperator("{")) {
				throwException(tk(), "expect `{`, got `" + tk()->value + "`");
			}
			block->addChild(parseBlock());
			
		}
		else {
			throwException(tk(), "expect `{` or `if` or `each`, got `" + tk()->value + "`");
		}

		return node;
	}

	AST::Node* AST::parseTypeName() {
		if (tk()->isIdentifier()) {
			auto name = new Node(tk());
			index += 1;
			if (tk()->isOperator("<")) {
				index += 1;
				while (!tk()->isOperator(">")) {
					if (tk()->isOperator(",")) { index += 1; continue; }
					name->addChild(parseType());
				}
				index += 1; //eat >
			}
			return name;
		}
		if (tk()->isOperator("(")) {
			auto tuple = new Node(new Token(Token::Kind::kOperator, Token::Type::tTuple, "_TUPLE_", tk()->row, tk()->col));
			index += 1;
			while (!tk()->isOperator(")")) {
				if (tk()->isOperator(",")) { index += 1; continue; }
				tuple->addChild(parseType());
			}
			index += 1; //eat )
			return tuple;
		}
		if (tk()->isKeyword("fn")) {
			return parseDefineFunc(dfmType);
		}

		throwException(tk(), "expect type name, got `" + tk()->value + "`");
		return nullptr;
	}

	AST::Node* AST::parseType() {
		Node* ref = nullptr;
		Node* name = nullptr;
		Node* subscript = nullptr;

		auto type = new Node(new Token(Token::Kind::kOperator, Token::Type::tType, "_TYPE_", tk()->row, tk()->col));

		if (tk()->isOperator("&")) {
			ref = new Node(tk());
			index += 1;
		} else {
			ref = new Node(Token::Empty);
		}

		name = parseTypeName();

		//不支持多维数组，可用泛型容器替代
		if (tk()->isOperator("[")) {
			subscript = new Node(new Token(Token::Kind::kOperator, Token::Type::tSubscript, "_ARRAY_", tk()->row, tk()->col));
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


		type->addChild(ref);
		type->addChild(name);
		type->addChild(subscript);

		return type;
	}

	AST::Node* AST::parseField() {
		auto node = new Node(new Token(Token::Kind::kKeyword, Token::Type::tField, "_FIELD_", tk()->row, tk()->col));

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

	AST::Node* AST::parseDefineName() {
		if (!tk()->isIdentifier()) {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}
		auto name = new Node(tk());
		index += 1;

		if (tk()->isOperator("<")) {
			index += 1;
			while (!tk()->isOperator(">")) {
				if (tk()->isOperator(",")) { index += 1; continue; }
				if (tk()->isIdentifier()) {
					name->addChild(new Node(tk()));
					index += 1;
					continue;
				}
				throwException(tk(), "expect identifier, got `" + tk()->value + "`");
			}
			index += 1; //eat >
		}

		return name;
	}

	AST::Node* AST::parseDefineFunc(DefFuncMode mode) {
		auto node = new Node(tk());
		index += 1;

		auto block = newBlock();
		node->addChild(block);
		
		//name
		if ((mode == dfmNormal || mode == dfmInterface) && tk()->isIdentifier()) {
			block->addChild(parseDefineName());
		} else {
			block->addChild(new Node(Token::Empty));
		}

		if (!tk()->isOperator("(")) {
			throwException(tk(), "expect `(`, got `" + tk()->value + "`");
		}
		index += 1;

		auto args = new Node(new Token(Token::Kind::kOperator, Token::Type::tFnDefArgs, "_ARGS_", tk()->row, tk()->col));
		block->addChild(args);
		while (!tk()->isOperator(")")) {
			if (tk()->isOperator(",")) { index += 1; continue; }
			args->addChild(parseField());
		}

		index += 1; // eat )

		if (tk()->isOperator("->")) {
			index += 1;
			block->addChild(parseType());
		} else {
			block->addChild(new Node(Token::Empty));
		}

		if ((mode == dfmNormal || mode == dfmInterface || mode == dfmValue) && tk()->isOperator("{")) {
			block->addChild(parseBlock());
		} else {
			if (mode == dfmInterface || mode == dfmType) block->addChild(new Node(Token::Empty));
			else throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}

		return node;
	}

	AST::Node* AST::parseDefineStruct() {
		auto node = new Node(tk());
		index += 1;

		//name
		if (tk()->isIdentifier()) {
			node->addChild(parseDefineName());
		} else {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}
		index += 1;

		while (!tk()->isOperator("}")) {
			if (tk()->isOperator(";")) { index += 1; continue; }
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
			node->addChild(parseDefineName());
		} else {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}

		//interface
		if (tk()->isOperator(":")) {
			index += 1;
			node->addChild(parseDefineName());
		} else {
			node->addChild(new Node(Token::Empty));
		}

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}
		index += 1;

		while (!tk()->isOperator("}")) {
			auto t = tk();
			if (t->isOperator(";")) { index += 1; continue; }
			if (t->isOperator(",")) { index += 1; continue; }
			if (t->isKeyword("fn")) {
				node->addChild(parseDefineFunc());
				continue;
			}

			throwException(t, "expect function definition, got `" + t->value + "`");
		}
		index += 1; // eat }

		return node;
	}

	AST::Node* AST::parseDefineInterface() {
		auto node = new Node(tk());
		index += 1;

		//name
		if (tk()->isIdentifier()) {
			node->addChild(parseDefineName());
		} else {
			throwException(tk(), "expect identifier, got `" + tk()->value + "`");
		}

		if (!tk()->isOperator("{")) {
			throwException(tk(), "expect `{`, got `" + tk()->value + "`");
		}
		index += 1;

		while (!tk()->isOperator("}")) {
			auto t = tk();
			if (t->isOperator(";")) { index += 1; continue; }
			if (t->isOperator(",")) { index += 1; continue; }

			if (t->isKeyword("fn")) {
				node->addChild(parseDefineFunc(dfmInterface));
				continue;
			}
			throwException(t, "expect function definition, got `" + t->value + "`");
		}
		index += 1; // eat }

		return node;
	}




	void AST::checkNode(Node* node) {
		if (node->token->kind == Token::Kind::kBlock) {
			node->parentBlock = currBlock;
			currBlock = node;

			for (auto n : node->children) {
				checkNode(n);
			}

			currBlock = node->parentBlock;
		}
		else {
			for (auto n : node->children) {
				checkNode(n);
			}
		}
	}
}