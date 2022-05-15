#ifndef LANG_HEADER
#define LANG_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

struct StackAllocator
{
	size_t reserved;
	size_t size;
	size_t offset;
	void *data;
};

/* Initializes a StackAllocator.
   - size - internal buffer size (can be 0) */
void StackAllocator_Init(struct StackAllocator *self, size_t size);

#define StackAllocator_ALIGNMENT 4096

/* Allocates space for a new item
   - size - item size */
void *StackAllocator_Alloc(struct StackAllocator *self, size_t size);

/* Deallocates the internal buffer of a StackAllocator. */
void StackAllocator_Free(struct StackAllocator *self);

enum TokenType
{
	TokenType_eIdentifier,
	TokenType_eInteger,
	TokenType_eDoubleEqual,
	TokenType_eNotEqual,
	TokenType_eGreaterEqual,
	TokenType_eLessEqual,
	TokenType_eArrow,
	TokenType_eDoubleAmp,
	TokenType_eDoubleBar,
	TokenType_eDoubleGreater,
	TokenType_eDoubleLess,
	TokenType_Max
};

struct Token
{
	unsigned char type;
	union
	{
		const char *value;
		size_t integer;
	};
};

struct TokenList
{
	size_t token_count;
	struct StackAllocator token_allocator;
	size_t value_count;
	struct StackAllocator value_allocator;
};

struct Lexer
{
	struct TokenList tokens;
	FILE *file_in;
	unsigned int lineno;
	unsigned int column;
};

void Lexer_Init(struct Lexer *self, FILE *file_in);
void Lexer_Read(struct Lexer *self);
void Lexer_Free(struct Lexer *self);

enum Node_Bin_Type
{
	Node_Bin_Type_eAdd,
	Node_Bin_Type_eSub,
	Node_Bin_Type_eMul,
	Node_Bin_Type_eDiv,
	Node_Bin_Type_eMod,
	Node_Bin_Type_eXor,
	Node_Bin_Type_eLsh,
	Node_Bin_Type_eRsh,
	Node_Bin_Type_eBitOr,
	Node_Bin_Type_eBitAnd,
	Node_Bin_Type_eOr,
	Node_Bin_Type_eAnd,
	Node_Bin_Type_eEq,
	Node_Bin_Type_eNeq,
	Node_Bin_Type_eGt,
	Node_Bin_Type_eLt,
	Node_Bin_Type_eGeq,
	Node_Bin_Type_eLeq,
	Node_Bin_Type_eSet,
	Node_Bin_Type_Max = 2 * Node_Bin_Type_eSet
};

enum Node_Un_Type
{
	Node_Un_Type_eNeg,
	Node_Un_Type_ePos,
	Node_Un_Type_eDeref,
	Node_Un_Type_ePtrTo,
	Node_Un_Type_eNot,
	Node_Un_Type_eBitNot,
	Node_Un_Type_Max
};

struct Node;

typedef void (*Node_ToString_FPtr)(struct Node *node, FILE *fout);
struct Node { Node_ToString_FPtr ToString; };

struct Node_Name { struct Node node; const char *name; };
struct Node_Int { struct Node node; size_t value; };
struct Node_Bin { struct Node node; enum Node_Bin_Type type; struct Node *lhs, *rhs; };
struct Node_Un { struct Node node; enum Node_Un_Type type; struct Node *op; };

struct Node_Name *Node_Name_Alloc(struct StackAllocator *alloc, const char *name);
struct Node_Int *Node_Int_Alloc(struct StackAllocator *alloc, size_t value);
struct Node_Bin *Node_Bin_Alloc(struct StackAllocator *alloc, enum Node_Bin_Type type, struct Node *lhs, struct Node *rhs);
struct Node_Un *Node_Un_Alloc(struct StackAllocator *alloc, enum Node_Un_Type type, struct Node *op);

void Node_Name_ToString(struct Node_Name *node, FILE *fout);
void Node_Int_ToString(struct Node_Int *node, FILE *fout);
void Node_Bin_ToString(struct Node_Bin *node, FILE *fout);
void Node_Un_ToString(struct Node_Un *node, FILE *fout);

struct Parser {
	struct StackAllocator node_allocator;
	struct TokenList *tokens;
	size_t index;
};

void Parser_Init(struct Parser *self, struct TokenList *tokens);
struct Node *Parser_Parse_TopLevel(struct Parser *self);
struct Node *Parser_Parse_Statement(struct Parser *self);
struct Node *Parser_Parse_Expression(struct Parser *self);
void Parser_Free(struct Parser *self);

#ifdef __cplusplus
}
#endif

#endif
