#include "ast.h"

#define NODE_P(T)    ((T*)ast->createNode<T>()->setTypeName(#T)->parse())
#define NODE_C(T)    ((T*)ast->createNode<T>()->setTypeName(#T))
#define NODE_AP(T,A) ((T*)ast->createNode<T>(A)->setTypeName(#T)->parse())

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

	AST::Node* AST::parse() {
		auto ast = this;
		auto root = NODE_P(BlockNode);
		//checkNode(root);
		return root;
	}




	//oid AST::checkNode(Node* node) {
	//	if (node->token->kind == Token::Kind::kBlock) {
	//		node->parentBlock = currBlock;
	//		currBlock = node;
	//
	//		for (auto n : node->children) {
	//			checkNode(n);
	//		}
	//
	//		currBlock = node->parentBlock;
	//	}
	//	else {
	//		for (auto n : node->children) {
	//			checkNode(n);
	//		}
	//	}
	//


















	


	AST::Node* AST::BlockNode::parse() {
		string end = "}";
		string sep = ";";

		if (ast->tk()->isOperator("{")) ast->index += 1;

		while (ast->tk() != Token::Empty) {
			auto t = ast->tk();
			if (t->isOperator(end)) {
				ast->index += 1;
				break;
			}

			if (t->isOperator(sep)) {
				ast->index += 1;
				continue;
			}

			if (t->isKeyword("var")) {
				nodes.push_back(NODE_P(DeclVarNode));
				continue;
			}

			if (t->isKeyword("const")) {
				nodes.push_back(NODE_P(DeclConstNode));
				continue;
			}

			if (t->isKeyword("if")) {
				nodes.push_back(NODE_P(IfNode));
				continue;
			}

			if (t->isKeyword("loop")) {
				nodes.push_back(NODE_P(LoopNode));
				continue;
			}

			if (t->isKeyword("fn")) {
				nodes.push_back(NODE_P(DefineFuncNode));
				continue;
			}

			if (t->isKeyword("struct")) {
				nodes.push_back(NODE_P(DefineStructNode));
				continue;
			}

			if (t->isKeyword("impl")) {
				nodes.push_back(NODE_P(DefineImplNode));
				continue;
			}

			if (t->isKeyword("interface")) {
				nodes.push_back(NODE_P(DefineInterfaceNode));
				continue;
			}

			if (t->isKeyword("return")) {
				nodes.push_back(NODE_P(ReturnNode));
				continue;
			}

			if (t->isAssignOperator()) {
				NODE_AP(AssignNode, dynamic_cast<BlockNode*>(this));
				//parseAssign(block);
				continue;
			}

			if (t->isOperator("{")) {
				nodes.push_back(NODE_P(BlockNode));
				continue;
			}
			//...
			auto n = NODE_AP(ExpressionNode, true);
			if (n != nullptr) {
				nodes.push_back(n);
				continue;
			}

			throwException(t, "unmatched statement, got `" + t->value + "`");
		}
		return this;
	}

	AST::Node* AST::PrimaryExprNode::parse() {
		auto t = ast->tk();
		if (t->isLiteral()) {
			return NODE_P(LiteralNode);
		}
		if (t->isIdentifier()) {
			return NODE_P(IdentifierNode);
		}
		if (t->isOperator("(")) {
			return NODE_P(ParenExprNode);
		}
		if (t->isKeyword("fn")) {
			return NODE_AP(DefineFuncNode, DefineFuncNode::dfmValue);
		}
		if (t->isKeyword("var")) {
			return NODE_P(DeclVarNode);
		}
		return nullptr;
	}

	AST::Node* AST::LiteralNode::parse() {
		token = ast->tk();
		if (!token->isLiteral()) {
			throwException(token, "expect literal, got `" + token->value + "`");
		}
		ast->index += 1;
		return this;
	}

	AST::Node* AST::IdentifierNode::parse() {
		token = ast->tk();
		if (!token->isIdentifier()) {
			throwException(token, "expect identifier, got `" + token->value + "`");
		}
		ast->index += 1;
		return this;
	}

	AST::Node* AST::OperatorNode::parse() {
		token = ast->tk();
		if (!token->isOperator()) {
			throwException(token, "expect operator, got `" + token->value + "`");
		}
		ast->index += 1;

		if (token->isUnaryOperator()) {
			Node* node = NODE_C(UnaryOperatorNode);
			node->token = token;
			return node;
		} else if (token->isBinaryOperator()) {
			Node* node = NODE_C(BinaryOperatorNode);
			node->token = token;
			return node;
		}
		return this;
	}

	AST::Node* AST::ParenExprNode::parse() {
		int start = ast->index;
		
		ast->index += 1;//(
		auto node = NODE_P(ExpressionNode);

		if (ast->tk()->isOperator(",")) {
			ast->index = start;
			return NODE_P(TupleExprNode);
		}

		if (!ast->tk()->isOperator(")")) {
			throwException(ast->tk(), "expect `)`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//)

		return node;
	}

	AST::Node* AST::TupleExprNode::parse() {
		ast->index += 1;//(
		while (!ast->tk()->isOperator(")")) {
			values.push_back(NODE_P(ExpressionNode));
			if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator(")")) break;
			throwException(ast->tk(), "expect `,` or `)`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//)
		return this;
	}

	AST::Node* AST::FuncCallArgsNode::parse() {
		ast->index += 1;//(
		while (!ast->tk()->isOperator(")")) {
			nodes.push_back(NODE_P(ExpressionNode));
			if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator(")")) break;
			throwException(ast->tk(), "expect `,` or `)`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//)
		return this;
	}

	AST::Node* AST::SubscriptIndexNode::parse() {
		ast->index += 1;//[

		auto expr = NODE_P(ExpressionNode);

		if (!ast->tk()->isOperator("]")) {
			throwException(ast->tk(), "expect `]`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//]

		return expr;
	}

	AST::Node* AST::ExpressionNode::parse() {
		enum State {
			sOperator,
			sPrimaryExpr,
		};

		auto list = vector<Node*>();
		State state = sOperator;

		while (ast->tk() != Token::Empty) {
			auto t = ast->tk();
			if (t->isBinaryOperator() || t->isUnaryOperator()) {
				if (state == sOperator && !t->convertToUnaryOperator()) throwException(t, "expect unary operator, got `" + t->value + "`");
				list.push_back(NODE_P(OperatorNode));
				state = sOperator;
				continue;
			}
			if (t->isOperator("(") && state == sPrimaryExpr) {
				list.push_back(NODE_C(FuncCallNode));
				list.push_back(NODE_P(FuncCallArgsNode));
				state = sPrimaryExpr;
				continue;
			}
			if (t->isOperator("[") && state == sPrimaryExpr) {
				list.push_back(NODE_C(SubscriptNode));
				list.push_back(NODE_P(SubscriptIndexNode));
				state = sPrimaryExpr;
				continue;
			}
			if (state == sPrimaryExpr) break;
			auto expr = NODE_P(PrimaryExprNode);
			if (expr != nullptr) {
				list.push_back(expr);
				state = sPrimaryExpr;
				continue;
			}
			break;
		}
		if (list.empty()) {
			if (enableEmpty) return nullptr;
			throwException(ast->tk(), "expect expression, got `" + ast->tk()->value + "`");
		}
		if (state == sOperator) {
			throwException(list.back()->token, "incomplete expression, end with `" + list.back()->token->value + "`");
		}
		return buildTree(list, 0, list.size() - 1);
	}

	AST::Node* AST::ExpressionNode::buildTree(vector<Node*> list, int begin, int end) {
		int opi = -1;
		int priority = 99;
		for (auto i = begin; i <= end; i++) {
			auto n = dynamic_cast<OperatorNode*>(list[i]);
			if (n == nullptr || !n->empty()) continue;
			auto t = n->token;
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
				dynamic_cast<BinaryOperatorNode*>(op)->addNodes(buildTree(list, begin, opi - 1), buildTree(list, opi + 1, end));
			} else if (op->token->isUnaryOperator()) {
				dynamic_cast<UnaryOperatorNode*>(op)->addNode(buildTree(list, opi + 1, end));
			} else {
				//error
				throwException(op->token, "unexpected operator `" + op->token->value + "`");
			}
			return op;
		}
		return list[begin];
	}

	AST::Node* AST::DeclVarNode::parse() {
		ast->index += 1;//var

		this->name = NODE_P(IdentifierNode);

		if (!ast->tk()->isOperator(":") && !(!inLoop && ast->tk()->isOperator("=")) && !(inLoop && ast->tk()->isKeyword("in"))) {
			throwException(name->token, "cannot determine the type of variable `" + name->token->value + "`");
		}

		if (ast->tk()->isOperator(":")) {
			ast->index += 1;//:
			this->type = NODE_P(TypeNode);
		} else {
			//type=nullptr
		}

		return this;
	}

	AST::Node* AST::DeclConstNode::parse() {
		ast->index += 1;

		name = NODE_P(IdentifierNode);

		if (ast->tk()->isOperator(":")) {
			ast->index += 1;//:
			this->type = NODE_P(TypeNode);
		} else {
			//type=nullptr
		}

		if (!(!inLoop && ast->tk()->isOperator("=")) && !(inLoop && ast->tk()->isKeyword("in"))) {
			throwException(name->token, "constant value required");
		}

		return this;
	}

	AST::Node* AST::AssignNode::parse() {
		if (block->nodes.empty()) throwException(ast->tk(), "lvalue not found");

		token = ast->tk();
		ast->index += 1;//=

		//auto lv = block->nodes.back();
		//if (!lv->token->isIdentifier() &&
		//	!lv->token->isKeyword("var") &&
		//	!lv->token->isKeyword("const")) throwException(tk(), "invalid lvalue");

		node = block->nodes.back();
		block->nodes[block->nodes.size() - 1] = this;
		value = NODE_P(ExpressionNode);
		
		return nullptr;
	}

	AST::Node* AST::ReturnNode::parse() {
		token = ast->tk();
		ast->index += 1;//return

		value = NODE_AP(ExpressionNode, true);

		return this;
	}

	AST::Node* AST::TupleTypeNode::parse() {
		ast->index += 1;//(
		while (!ast->tk()->isOperator(")")) {
			types.push_back(NODE_P(TypeNode));
			if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator(")")) break;
			throwException(ast->tk(), "expect `,` or `)`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//)
		return this;
	}

	AST::Node* AST::TypeNameNode::parse() {
		if (ast->tk()->isIdentifier()) {
			name = NODE_P(IdentifierNode);
			if (ast->tk()->isOperator("<")) {
				ast->index += 1;//<
				while (!ast->tk()->isOperator(">")) {
					genericTypes.push_back(NODE_P(TypeNode));
					if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
					if (ast->tk()->isOperator(">")) break;
					throwException(ast->tk(), "expect `,` or `>`, got `" + ast->tk()->value + "`");
				}
				ast->index += 1; //>
			}
			return this;
		}
		if (ast->tk()->isOperator("(")) {
			return NODE_P(TupleTypeNode);
		}
		if (ast->tk()->isKeyword("fn")) {
			return NODE_AP(DefineFuncNode, DefineFuncNode::dfmType);
		}

		throwException(ast->tk(), "expect type name, got `" + ast->tk()->value + "`");
		return nullptr;
	}

	AST::Node* AST::TypeNode::parse() {
		if (ast->tk()->isOperator("&")) {
			isRef = true;
			ast->index += 1;//&
		}

		name = NODE_P(TypeNameNode);

		//不支持多维数组，可用泛型容器替代
		if (ast->tk()->isOperator("[")) {
			isArray = true;
			ast->index += 1;//[
			if (ast->tk()->isLiteral(Token::Type::tInteger)) {
				arrayLength = atoi(ast->tk()->value.c_str());
				if (arrayLength < 0) {
					throwException(ast->tk(), "expect positive integer, got `" + ast->tk()->value + "`");
				}
				ast->index += 1;//number
			}
			if (!ast->tk()->isOperator("]")) {
				throwException(ast->tk(), "expect `]`, got `" + ast->tk()->value + "`");
			}
			ast->index += 1;//]
		}

		return this;
	}

	AST::Node* AST::FieldNode::parse() {
		while (true) {
			if (ast->tk()->isKeyword("const")) {
				isConst = true;
				ast->index += 1;//const
				continue;
			}
			break;
		}

		name = NODE_P(IdentifierNode);

		if (!ast->tk()->isOperator(":")) {
			throwException(ast->tk(), "expect `:`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//:

		type = NODE_P(TypeNode);

		//default value
		if (ast->tk()->isOperator("=")) {
			ast->index += 1;//=
			defaultValue = NODE_P(ExpressionNode);
		}

		return this;
	}

	AST::Node* AST::IfNode::parse() {
		token = ast->tk();
		ast->index += 1;//if

		condExpr = NODE_P(ExpressionNode);

		if (!ast->tk()->isOperator("{")) {
			throwException(ast->tk(), "expect `{`, got `" + ast->tk()->value + "`");
		}

		ifBlock = NODE_P(BlockNode);

		if (ast->tk()->isKeyword("else")) {
			ast->index += 1;//else
			if (ast->tk()->isKeyword("if")) {
				elseBlock = NODE_P(IfNode);
			}
			else if (ast->tk()->isOperator("{")) {
				elseBlock = NODE_P(BlockNode);
			}
			else {
				throwException(ast->tk(), "expect `{` or `if`, got `" + ast->tk()->value + "`");
			}
		}

		return this;
	}

	AST::Node* AST::LoopNode::parse() {
		token = ast->tk();
		ast->index += 1;//loop

		if (ast->tk()->isOperator("{")) {
			block = NODE_P(BlockNode);
		}
		else if (ast->tk()->isKeyword("if")) {
			block = NODE_P(IfNode);

			Node* elseNode = block;
			do {
				elseNode = dynamic_cast<IfNode*>(elseNode)->elseBlock;
				if (elseNode == nullptr) elseNode = NODE_C(BlockNode);
			} while (elseNode->token && elseNode->token->isKeyword("if"));
			dynamic_cast<BlockNode*>(elseNode)->nodes.push_back(NODE_C(BreakNode));
		}
		else if (ast->tk()->isKeyword("each")) {
			block = NODE_P(EachNode);
		}
		else {
			throwException(ast->tk(), "expect `{` or `if` or `each`, got `" + ast->tk()->value + "`");
		}

		return this;
	}

	AST::Node* AST::EachNode::parse() {
		token = ast->tk();
		ast->index += 1;//each

		if (ast->tk()->isIdentifier()) {
			each = NODE_P(IdentifierNode);
		}
		else if (ast->tk()->isKeyword("var")) {
			each = NODE_AP(DeclVarNode, true);
		}
		else if (ast->tk()->isKeyword("const")) {
			each = NODE_AP(DeclConstNode, true);
		}
		else {
			throwException(ast->tk(), "expect `var` or `const` or identifier, got `" + ast->tk()->value + "`");
		}

		if (!ast->tk()->isKeyword("in")) {
			throwException(ast->tk(), "expect `in`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//in

		in = NODE_P(ExpressionNode);

		if (!ast->tk()->isOperator("{")) {
			throwException(ast->tk(), "expect `{`, got `" + ast->tk()->value + "`");
		}

		block = NODE_P(BlockNode);

		return this;
	}

	AST::Node* AST::DefineNameNode::parse() {
		name = NODE_P(IdentifierNode);
		if (ast->tk()->isOperator("<")) {
			ast->index += 1;//<
			while (!ast->tk()->isOperator(">")) {
				genericNames.push_back(NODE_P(IdentifierNode));
				if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
				if (ast->tk()->isOperator(">")) break;
				throwException(ast->tk(), "expect `,` or `>`, got `" + ast->tk()->value + "`");
			}
			ast->index += 1;//>
		}
		return this;
	}

	AST::Node* AST::DefineFuncNode::parse() {
		token = ast->tk();
		ast->index += 1;//fn

		//name
		if ((mode == dfmNormal || mode == dfmInterface) && ast->tk()->isIdentifier()) {
			name = NODE_P(DefineNameNode);
		}

		if (!ast->tk()->isOperator("(")) {
			throwException(ast->tk(), "expect `(`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//(

		while (!ast->tk()->isOperator(")")) {
			args.push_back(NODE_P(FieldNode));
			if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator(")")) break;
			throwException(ast->tk(), "expect `,` or `)`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//)

		if (ast->tk()->isOperator("->")) {
			ast->index += 1;//->
			returnType = NODE_P(TypeNode);
		}

		if ((mode == dfmNormal || mode == dfmInterface || mode == dfmValue) && ast->tk()->isOperator("{")) {
			block = NODE_P(BlockNode);
		} else {
			if (mode != dfmInterface && mode != dfmType) {
				throwException(ast->tk(), "expect `{`, got `" + ast->tk()->value + "`");
			}
		}

		return this;
	}

	AST::Node* AST::DefineStructNode::parse() {
		token = ast->tk();
		ast->index += 1;//struct

		name = NODE_P(DefineNameNode);

		if (!ast->tk()->isOperator("{")) {
			throwException(ast->tk(), "expect `{`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//{

		while (!ast->tk()->isOperator("}")) {
			fields.push_back(NODE_P(FieldNode));
			if (ast->tk()->isOperator(";")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator("}")) break;
			throwException(ast->tk(), "expect field name, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//}

		return this;
	}

	AST::Node* AST::DefineImplNode::parse() {
		token = ast->tk();
		ast->index += 1;//impl

		name = NODE_P(DefineNameNode);

		//interface
		if (ast->tk()->isOperator(":")) {
			ast->index += 1;//:
			iface = NODE_P(DefineNameNode);
		}

		if (!ast->tk()->isOperator("{")) {
			throwException(ast->tk(), "expect `{`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//{

		while (!ast->tk()->isOperator("}")) {
			funcs.push_back(NODE_P(DefineFuncNode));
			if (ast->tk()->isOperator(";")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator("}")) break;
			if (!ast->tk()->isKeyword("fn")) {
				throwException(ast->tk(), "expect function definition, got `" + ast->tk()->value + "`");
			}
		}
		ast->index += 1;//}

		return this;
	}

	AST::Node* AST::DefineInterfaceNode::parse() {
		token = ast->tk();
		ast->index += 1;//interface

		name = NODE_P(DefineNameNode);

		if (!ast->tk()->isOperator("{")) {
			throwException(ast->tk(), "expect `{`, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//{

		while (!ast->tk()->isOperator("}")) {
			funcs.push_back(NODE_AP(DefineFuncNode, DefineFuncNode::dfmInterface));
			if (ast->tk()->isOperator(";")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator(",")) { ast->index += 1; continue; }
			if (ast->tk()->isOperator("}")) break;
			throwException(ast->tk(), "expect function definition, got `" + ast->tk()->value + "`");
		}
		ast->index += 1;//}

		return this;
	}

	AST::Node* AST::BreakNode::parse() {
		token = ast->tk();
		ast->index += 1;//break

		return this;
	}
}