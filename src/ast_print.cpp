#include "ast.h"

#define PRINT ast->printBuf << 

namespace Lang {
	void AST::BlockNode::print() {
		PRINT "block" << endl;
	}
}