#pragma once

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include "token.h"
#include "lexer.h"

using namespace std;

namespace Lang {
	class AST {
	public:
		class Node;
	private:
		Lexer*       lex   = nullptr;
		int          index = 0;
		list<Node*>  nodes = list<Node*>();
	public:
		stringstream printBuf;

	public:
		AST(Lexer* l) {
			lex = l;
		}

		~AST() {
			for (auto node : nodes) delete node;
		}

		Token* tk(int offset = 0);

		Node* parse();
	public:
		template<class T, class... TS>
		T* createNode(TS&&... args) {
			auto obj = new T(forward<TS>(args)...);
			static_cast<Node*>(obj)->ast = this;
			this->nodes.push_back(obj);
			return obj;
		}

		class Node {
		public:
			AST*   ast      = nullptr;
			Token* token    = nullptr;
			string typeName = "";

			virtual ~Node() {};

			Node* setTypeName(string name) { 
				typeName = name;
				return this;
			}

			virtual Node* parse() = 0;
			virtual Node* eval() { return nullptr; };
			virtual void  print();
		};

		
		class LiteralNode : public Node {
		public:
			Node* parse() override;
			void  print() override;
		};

		class IdentifierNode : public Node {
		public:
			Node* parse() override;
			void  print() override;
		};

		class OperatorNode : public Node {
		public:
			Node* parse() override;
			void  print() override;

			virtual bool empty() {
				return false;
			};
		};

		class BinaryOperatorNode : public OperatorNode {
		public:
			Node* left  = nullptr;
			Node* right = nullptr;

			virtual void addNodes(Node* _left, Node* _right) {
				left = _left;
				right = _right;
			}

			Node* parse() override { return nullptr; };
			void  print() override;

			bool empty() override {
				return left == nullptr && right == nullptr;
			}
		};

		class UnaryOperatorNode : public OperatorNode {
		public:
			Node* node = nullptr;

			virtual void addNode(Node* _node) {
				node = _node;
			}

			Node* parse() override { return nullptr; };
			void  print() override;

			bool empty() override {
				return node == nullptr;
			}
		};

		class ScopeNode : public Node {
		public:
			Node* outerScope = nullptr;
		};

		class BlockNode : public ScopeNode {
		public:
			vector<Node*> nodes = vector<Node*>();

			Node* parse() override;
			void  print() override;
		};

		class TypeNode;
		class TypeNameNode : public Node {
		public:
			IdentifierNode*   name         = nullptr;
			vector<TypeNode*> genericTypes = vector<TypeNode*>();

			Node* parse() override;
			void  print() override;
		};

		class TypeNode : public Node {
		public:
			bool          isRef       = false;
			TypeNameNode* name        = nullptr;
			bool          isArray     = false;
			int           arrayLength = 0;

			Node* parse() override;
			void  print() override;
		};

		class ExpressionNode : public Node {
		private:
			bool enableEmpty = false;
		public:
			ExpressionNode(bool _enableEmpty = false) {
				enableEmpty = _enableEmpty;
			}

			Node* parse() override;

			static Node* buildTree(vector<Node*> list, int begin, int end);
		};

		class FuncCallArgsNode : public Node {
		public:
			vector<ExpressionNode*> nodes = vector<ExpressionNode*>();

			Node* parse() override;
			void  print() override;
		};

		class FuncCallNode : public BinaryOperatorNode {
		public:
			Node*             func = nullptr;
			FuncCallArgsNode* args = nullptr;

			FuncCallNode() {
				token = new Token(Token::Kind::kOperator, Token::Type::tFnCall, "_FNCALL_", 0, 0);
			}

			void addNodes(Node* _left, Node* _right) override {
				left = _left;
				right = _right;
				func = _left;
				args = dynamic_cast<FuncCallArgsNode*>(_right);
			}

			bool empty() override {
				return func == nullptr && args == nullptr;
			}

			void print() override;
		};

		class SubscriptNode : public BinaryOperatorNode {
		public:
			Node* node = nullptr;
			Node* index = nullptr;

			SubscriptNode() {
				token = new Token(Token::Kind::kOperator, Token::Type::tSubscript, "_SUBSCRIPT_", 0, 0);
			}

			void addNodes(Node* _left, Node* _right) override {
				left = _left;
				right = _right;
				node = _left;
				index = _right;
			}

			bool empty() override {
				return node == nullptr && index == nullptr;
			}

			void print() override;
		};

		class SubscriptIndexNode : public Node {
		public:
			Node* parse() override;
		};

		class PrimaryExprNode : public Node {
		public:
			Node* parse() override;
		};

		class ParenExprNode : public Node {
		public:
			Node* parse() override;
		};

		class TupleExprNode : public Node {
		public:
			vector<ExpressionNode*> values = vector<ExpressionNode*>();

			Node* parse() override;
			void  print() override;
		};

		class TupleTypeNode : public Node {
		public:
			vector<TypeNode*> types = vector<TypeNode*>();

			Node* parse() override;
			void  print() override;
		};

		class DeclVarNode : public Node {
		private:
			bool            inLoop = false;
		public:
			IdentifierNode* name   = nullptr;
			TypeNode*       type   = nullptr;

			DeclVarNode(bool _inLoop = false) {
				inLoop = _inLoop;
			}

			Node* parse() override;
			void  print() override;
		};

		class DeclConstNode : public Node {
		private:
			bool            inLoop = false;
		public:
			IdentifierNode* name   = nullptr;
			TypeNode*       type   = nullptr;

			DeclConstNode(bool _inLoop = false) {
				inLoop = _inLoop;
			}

			Node* parse() override;
			void  print() override;
		};

		class AssignNode : public Node {
		private:
			BlockNode*      block = nullptr;
		public:
			Node*           node  = nullptr;
			ExpressionNode* value = nullptr;

			AssignNode(BlockNode* _block) {
				block = _block;
			}

			Node* parse() override;
			void  print() override;
		};

		class ReturnNode : public Node {
		public:
			ExpressionNode* value = nullptr;

			Node* parse() override;
			void  print() override;
		};

		class FieldNode : public Node {
		public:
			bool            isConst      = false;
			IdentifierNode* name         = nullptr;
			TypeNode*       type         = nullptr;
			ExpressionNode* defaultValue = nullptr;

			Node* parse() override;
			void  print() override;
		};

		class IfNode : public ScopeNode {
		public:
			ExpressionNode* condExpr  = nullptr;
			BlockNode*      ifBlock   = nullptr;
			Node*           elseBlock = nullptr;

			Node* parse() override;
			void  print() override;
		};

		class LoopNode : public ScopeNode {
		public:
			Node* block = nullptr;

			Node* parse() override;
			void  print() override;
		};

		class BreakNode : public Node {
		public:
			BreakNode() {
				token = new Token(Token::Kind::kKeyword, Token::Type::tBreak, "break", 0, 0);
			}
			Node* parse() override;
			void  print() override;
		};

		class EachNode : public ScopeNode {
		public:
			Node*           each  = nullptr;
			ExpressionNode* in    = nullptr;
			BlockNode*      block = nullptr;

			Node* parse() override;
			void  print() override;
		};

		class DefineNameNode : public Node {
		public:
			IdentifierNode*         name         = nullptr;
			vector<IdentifierNode*> genericNames = vector<IdentifierNode*>();

			Node* parse() override;
			void  print() override;
		};

		class DefineFuncNode : public ScopeNode {
		public:
			enum Mode {
				dfmNormal,
				dfmInterface,
				dfmType,
				dfmValue,
			};
		private:
			Mode               mode       = dfmNormal;
		public:
			DefineNameNode*    name       = nullptr;
			vector<FieldNode*> args       = vector<FieldNode*>();
			TypeNode*          returnType = nullptr;
			BlockNode*         block      = nullptr;

			DefineFuncNode(Mode _mode = dfmNormal) {
				mode = _mode;
			}

			Node* parse() override;
			void  print() override;
		};

		class DefineStructNode : public Node {
		public:
			DefineNameNode*    name   = nullptr;
			vector<FieldNode*> fields = vector<FieldNode*>();

			Node* parse() override;
			void  print() override;
		};

		class DefineImplNode : public Node {
		public:
			DefineNameNode*         name  = nullptr;
			DefineNameNode*         iface = nullptr;
			vector<DefineFuncNode*> funcs = vector<DefineFuncNode*>();

			Node* parse() override;
			void  print() override;
		};

		class DefineInterfaceNode : public Node {
		public:
			DefineNameNode*         name  = nullptr;
			vector<DefineFuncNode*> funcs = vector<DefineFuncNode*>();

			Node* parse() override;
			void  print() override;
		};

		

	private:



		//void checkNode(Node* node);
	};
	

	
}