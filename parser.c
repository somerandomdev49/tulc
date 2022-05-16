#include "lang.h"

void Parser_Init(struct Parser *self, struct TokenList *tokens)
{
	self->tokens = tokens;
	self->index = 0;
	StackAllocator_Init(&self->node_allocator, 1024);
}

void Parser_Free(struct Parser *self)
{
	self->index = 0;
	self->tokens = NULL;
	StackAllocator_Free(&self->node_allocator);
}

static inline struct Token *Get_(struct Parser *self)
{
	return &((struct Token *)self->tokens->token_allocator.data)[self->index++];
}

static inline struct Token *PeekN_(struct Parser *self, int n)
{
	return &((struct Token *)self->tokens->token_allocator.data)[self->index + n - 1];
}

static inline struct Token *Peek_(struct Parser *self)
{
	return PeekN_(self, 1);
}

#define ALLOC_NODE_GENERIC(T) (struct T*)StackAllocator_Alloc(alloc, sizeof(struct T))
#define ALLOC_NODE(T, VAR) \
	struct T *VAR = ALLOC_NODE_GENERIC(T); \
	VAR->node.ToString = (Node_ToString_FPtr)&T##_ToString

struct Node_Name *Node_Name_Alloc(struct StackAllocator *alloc, const char *name)
{
	ALLOC_NODE(Node_Name, n);
	n->name = name;
	return n;
}

struct Node_Int *Node_Int_Alloc(struct StackAllocator *alloc, size_t value)
{
	ALLOC_NODE(Node_Int, n);
	n->value = value;
	return n;
}

struct Node_Bin *Node_Bin_Alloc(struct StackAllocator *alloc, enum Node_Bin_Type type, struct Node *lhs, struct Node *rhs)
{
	ALLOC_NODE(Node_Bin, n);
	n->type = type;
	n->lhs = lhs;
	n->rhs = rhs;
	return n;
}

struct Node_Un *Node_Un_Alloc(struct StackAllocator *alloc, enum Node_Un_Type type, struct Node *op)
{
	ALLOC_NODE(Node_Un, n);
	n->type = type;
	n->op = op;
	return n;
}

struct Node *PTopLevel(struct Parser *self);
struct Node *PStmt(struct Parser *self);
struct Node *PExpr(struct Parser *self);
struct Node *PAtom(struct Parser *self);

#define DECL_POps(NAME) \
	struct Node *POps##NAME(struct Parser *self)

DECL_POps(Asn);
DECL_POps(Eql);
DECL_POps(Log);
DECL_POps(Cmp);
DECL_POps(Add);
DECL_POps(Mul);
DECL_POps(Mod);
DECL_POps(Bit);
DECL_POps(Sft);

#define CREATE_NODE(T, ...) \
	(struct Node*)T##_Alloc(&self->node_allocator, __VA_ARGS__)

#define DEF_POps(NAME, DOWN, VAR, COND, VALUE) \
	DECL_POps(NAME) { \
		struct Node *lhs = DOWN(self); \
		struct Token *VAR = Peek_(self); \
		while(COND) {\
			Get_(self); \
			struct Node *rhs = DOWN(self); \
			lhs = CREATE_NODE(Node_Bin, VALUE, lhs, rhs); \
			VAR = Peek_(self); \
		} \
		return lhs; \
	}

bool IsAssignOp_(char t)
{
	return t == '+'
		|| t == '-'
		|| t == '*'
		|| t == '/'
		|| t == '%'
		|| t == '^'
		|| t == '|'
		|| t == '&'
		|| t == TokenType_eDoubleAmp
		|| t == TokenType_eDoubleBar
		|| t == TokenType_eDoubleGreater
		|| t == TokenType_eDoubleLess;
}

enum Node_Bin_Type Node_Bin_Type_FromTokenType(char c)
{
	switch(c)
	{
	case '+': return Node_Bin_Type_eAdd;
	case '-': return Node_Bin_Type_eSub;
	case '*': return Node_Bin_Type_eMul;
	case '/': return Node_Bin_Type_eDiv;
	case '%': return Node_Bin_Type_eMod;
	case '^': return Node_Bin_Type_eXor;
	case TokenType_eDoubleLess: return Node_Bin_Type_eLsh;
	case TokenType_eDoubleGreater: return Node_Bin_Type_eRsh;
	case '|': return Node_Bin_Type_eBitOr;
	case '&': return Node_Bin_Type_eBitAnd;
	case TokenType_eDoubleBar: return Node_Bin_Type_eOr;
	case TokenType_eDoubleAmp: return Node_Bin_Type_eAnd;
	case TokenType_eDoubleEqual: return Node_Bin_Type_eEq;
	case TokenType_eNotEqual: return Node_Bin_Type_eNeq;
	case '>': return Node_Bin_Type_eGt;
	case '<': return Node_Bin_Type_eLt;
	case TokenType_eGreaterEqual: return Node_Bin_Type_eGeq;
	case TokenType_eLessEqual: return Node_Bin_Type_eLeq;
	case '=': return Node_Bin_Type_eSet;
	default: return Node_Bin_Type_Max;
	}
}

DEF_POps(
	Asn, PAtom, t,
	t->type == '=' || (PeekN_(self, 2)->type == '=' && IsAssignOp_(t->type)),
	t->type == '=' ? Node_Bin_Type_eSet : Node_Bin_Type_eSet + Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Log, PAtom, t,
	t->type == TokenType_eDoubleAmp || t->type == TokenType_eDoubleBar,
	Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Eql, POpsLog, t,
	t->type == TokenType_eDoubleEqual || t->type == TokenType_eNotEqual,
	Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Cmp, POpsEql, t,
	t->type == TokenType_eGreaterEqual || t->type == TokenType_eLessEqual || t->type == '>' || t->type == '<',
	Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Add, POpsCmp, t,
	t->type == '+' || t->type == '-',
	Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Mul, POpsAdd, t,
	t->type == '+' || t->type == '-',
	Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Mod, POpsMul, t,
	t->type == '%' || t->type == '^',
	Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Bit, POpsMod, t,
	t->type == '&' || t->type == '|',
	Node_Bin_Type_FromTokenType(t->type)
)

DEF_POps(
	Sft, POpsMod, t,
	t->type == TokenType_eDoubleGreater || t->type == TokenType_eDoubleLess,
	Node_Bin_Type_FromTokenType(t->type)
)

struct Node *PDecl(struct Parser *self)
{

}

struct Node *PFUncDef(struct Parser *self)
{
	// decl-spec declr decl-list? comp-stmt

}

struct Node *PTopLevel(struct Parser *self)
{
	// func-def | decl
}

struct Node *Parser_Parse_TopLevel(struct Parser *self)
{ return PTopLevel(self); }

struct Node *Parser_Parse_Statement(struct Parser *self)
{ return PStmt(self); }

struct Node *Parser_Parse_Expression(struct Parser *self)
{ return PExpr(self); }

