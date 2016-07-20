#include "ast.h"

#define PRINT ast->printBuf << 

namespace Lang {
	void AST::Node::print() {
		PRINT "<div><span style=\"color:red\">" + typeName + "</span></div>";
	}

	void AST::BlockNode::print() {
		PRINT "<div>";
		PRINT "<span>BLOCK</span>";
		PRINT "<table><tr>";
		for (auto node : nodes) {
			PRINT "<td>";
			node->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::LiteralNode::print() {
		PRINT "<div><span>" + token->value + "</span></div>";
	}

	void AST::IdentifierNode::print() {
		PRINT "<div><span>" + token->value + "</span></div>";
	}

	void AST::OperatorNode::print() {
		PRINT "<div><span>" + token->value + "</span></div>";
	}

	void AST::UnaryOperatorNode::print() {
		PRINT "<div>";
		PRINT "<span>" + token->value + "</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		node->print();
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::BinaryOperatorNode::print() {
		PRINT "<div>";
		PRINT "<span>" + token->value + "</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		left->print();
		PRINT "</td>";
		PRINT "<td>";
		right->print();
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::TypeNode::print() {
		PRINT "<div>";
		PRINT "<span>TYPE</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		if (isRef) PRINT "&";
		PRINT "</td>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		PRINT "<td>";
		if (isArray) {
			PRINT "[";
			if (arrayLength > 0) PRINT arrayLength;
			PRINT "]";
		}
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::TypeNameNode::print() {
		PRINT "<div>";
		PRINT "<span></span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		for (auto type : genericTypes) {
			PRINT "<td>";
			type->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::FuncCallNode::print() {
		PRINT "<div>";
		PRINT "<span>FnCall</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		func->print();
		PRINT "</td>";
		PRINT "<td>";
		args->print();
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::FuncCallArgsNode::print() {
		PRINT "<div>";
		PRINT "<span>FnCallArgs</span>";
		PRINT "<table><tr>";
		for (auto node : nodes) {
			PRINT "<td>";
			node->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::SubscriptNode::print() {
		PRINT "<div>";
		PRINT "<span>Subscript</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		node->print();
		PRINT "</td>";
		PRINT "<td>";
		index->print();
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::TupleExprNode::print() {
		PRINT "<div>";
		PRINT "<span>TupleExpr</span>";
		PRINT "<table><tr>";
		for (auto node : values) {
			PRINT "<td>";
			node->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::TupleTypeNode::print() {
		PRINT "<div>";
		PRINT "<span>TupleType</span>";
		PRINT "<table><tr>";
		for (auto node : types) {
			PRINT "<td>";
			node->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::DeclVarNode::print() {
		PRINT "<div>";
		PRINT "<span>var</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		if (type != nullptr) {
			PRINT "<td>";
			type->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::DeclConstNode::print() {
		PRINT "<div>";
		PRINT "<span>const</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		if (type != nullptr) {
			PRINT "<td>";
			type->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::AssignNode::print() {
		PRINT "<div>";
		PRINT "<span>=</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		node->print();
		PRINT "</td>";
		PRINT "<td>";
		value->print();
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::ReturnNode::print() {
		PRINT "<div>";
		PRINT "<span>Return</span>";
		if (value != nullptr) {
			PRINT "<table><tr>";
			PRINT "<td>";
			value->print();
			PRINT "</td>";
			PRINT "</tr></table>";
		}
		PRINT "</div>";
	}

	void AST::FieldNode::print() {
		PRINT "<div>";
		PRINT "<span>Field</span>";
		PRINT "<table><tr>";
		if (isConst) {
			PRINT "<td>const</td>";
		}
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		if (type != nullptr) {
			PRINT "<td>";
			type->print();
			PRINT "</td>";
		}
		if (defaultValue != nullptr) {
			PRINT "<td>";
			defaultValue->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::IfNode::print() {
		PRINT "<div>";
		PRINT "<span>if</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		condExpr->print();
		PRINT "</td>";
		PRINT "<td>";
		ifBlock->print();
		PRINT "</td>";
		if (elseBlock != nullptr) {
			PRINT "<td>";
			elseBlock->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::LoopNode::print() {
		PRINT "<div>";
		PRINT "<span>loop</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		block->print();
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::BreakNode::print() {
		PRINT "<div><span>break</span></div>";
	}

	void AST::EachNode::print() {
		PRINT "<div>";
		PRINT "<span>each</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		each->print();
		PRINT "</td>";
		PRINT "<td>";
		in->print();
		PRINT "</td>";
		PRINT "<td>";
		block->print();
		PRINT "</td>";
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::DefineNameNode::print() {
		PRINT "<div>";
		PRINT "<span></span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		for (auto n : genericNames) {
			PRINT "<td>";
			n->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::DefineFuncNode::print() {
		PRINT "<div>";
		PRINT "<span>DefineFunc</span>";
		PRINT "<table><tr>";
		if (name != nullptr) {
			PRINT "<td>";
			name->print();
			PRINT "</td>";
		}
		if (!args.empty()) {
			PRINT "<td><div><table><tr>";
			for (auto n : args) {
				PRINT "<td>";
				n->print();
				PRINT "</td>";
			}
			PRINT "</tr></table></div></td>";
		}
		if (returnType != nullptr) {
			PRINT "<td>";
			returnType->print();
			PRINT "</td>";
		}
		if (block != nullptr) {
			PRINT "<td>";
			block->print();
			PRINT "</td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::DefineStructNode::print() {
		PRINT "<div>";
		PRINT "<span>DefineStruct</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		if (!fields.empty()) {
			PRINT "<td><div><table><tr>";
			for (auto n : fields) {
				PRINT "<td>";
				n->print();
				PRINT "</td>";
			}
			PRINT "</tr></table></div></td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::DefineImplNode::print() {
		PRINT "<div>";
		PRINT "<span>DefineImpl</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		if (iface != nullptr) {
			PRINT "<td>";
			iface->print();
			PRINT "</td>";
		}
		if (!funcs.empty()) {
			PRINT "<td><div><table><tr>";
			for (auto n : funcs) {
				PRINT "<td>";
				n->print();
				PRINT "</td>";
			}
			PRINT "</tr></table></div></td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}

	void AST::DefineInterfaceNode::print() {
		PRINT "<div>";
		PRINT "<span>DefineInterface</span>";
		PRINT "<table><tr>";
		PRINT "<td>";
		name->print();
		PRINT "</td>";
		if (!funcs.empty()) {
			PRINT "<td><div><table><tr>";
			for (auto n : funcs) {
				PRINT "<td>";
				n->print();
				PRINT "</td>";
			}
			PRINT "</tr></table></div></td>";
		}
		PRINT "</tr></table>";
		PRINT "</div>";
	}
}