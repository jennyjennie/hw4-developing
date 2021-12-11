
#ifndef __JAST_H__
#define __JAST_H__

struct Location {
    int line;
    int col;
};

// AST node with a pointer "pBody" pointing to the non-terminal struct associated with the tree node.
struct AstNode {
	int  (*print)(AstNode *ptr, int);
	int  (*visit)(AstNode *ptr);
	int  (*codegen)(AstNode *ptr);
	void *pBody;
	AstNode *pNext;
	Location location;
};

#endif //__JAST_H__