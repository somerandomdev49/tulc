#include "lang.h"

static inline bool IsIdenBegin_(char c)
{ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

static bool IsIden_(char c)
{ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; }

static bool IsIntBegin_(char c)
{ return (c >= '0' && c <= '9'); }

static bool IsInt_(char c)
{ return (c >= '0' && c <= '9') || c == '_'; }

static bool IsSpace_(char c)
{ return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }

void Lexer__ReportError(struct Lexer *self, const char *error)
{
	fprintf(stderr, "\033[0;31merror:\033[0;0m %s\n", error);
}

char Lexer_Next(struct Lexer *self)
{
	char c = fgetc(self->file_in);
	self->column += (c != EOF); // using this removes the branch.
	return c;
}

void Lexer_Read(struct Lexer *self)
{
	char c = Lexer_Next(self);
	while(1)
	{
		if(IsIdenBegin_(c))
		{
			size_t count = 1;
			char *buffer = StackAllocator_Alloc(&self->tokens.value_allocator, 32);
			buffer[0] = c;
			buffer[1] = '\0';

			for(;;)
			{
				c = Lexer_Next(self);
				if(!IsIden_(c)) break;
				++count;
				if(count >= 32)
				{
					Lexer__ReportError(self, "Identifiers cannot be longer than 31 characters.");
					break;
				}
				buffer[count - 1] = c;
				buffer[count] = '\0';
			}

			// Remove unnecesary bytes.
			self->tokens.value_allocator.offset -= 32 - count - 1;

			struct Token *t = StackAllocator_Alloc(&self->tokens.token_allocator, sizeof(struct Token));
			t->type = TokenType_eIdentifier;
			t->value = buffer;
			++self->tokens.value_count;
			++self->tokens.token_count;
		}
		else if(IsIntBegin_(c))
		{
			size_t value = c - '0';
			while(IsInt_(c = Lexer_Next(self)))
				value += value * 10 + c - '0';

			struct Token *t = StackAllocator_Alloc(&self->tokens.token_allocator, sizeof(struct Token));
			t->type = TokenType_eInteger;
			t->integer = value;
			++self->tokens.token_count;
		}
		else if(IsSpace_(c))
		{
			if(c == '\n') ++self->lineno;
			c = Lexer_Next(self);
		}
		else if(c == EOF) break;
		else
		{
			struct Token *t = StackAllocator_Alloc(&self->tokens.token_allocator, sizeof(struct Token));
			char d = Lexer_Next(self);
#define O(S, T) if(c == #S[0] && d == #S[1]) t->type = T
			O(->, TokenType_eArrow);
			O(==, TokenType_eDoubleEqual);
			O(>>, TokenType_eDoubleGreater);
			O(<<, TokenType_eDoubleLess);
			O(!=, TokenType_eNotEqual);
			O(>=, TokenType_eGreaterEqual);
			O(<=, TokenType_eLessEqual);
			O(||, TokenType_eDoubleBar);
			O(&&, TokenType_eDoubleAmp);
#undef O
			t->type = c;
			++self->tokens.token_count;
			c = Lexer_Next(self);
		} // Lexer__ReportError(self, "Unknown character.");
	}
}

void Lexer_Init(struct Lexer *self, FILE *file_in)
{
	self->column = 0;
	self->lineno = 0;
	self->file_in = file_in;
	self->tokens.token_count = 0;
	self->tokens.value_count = 0;
	StackAllocator_Init(&self->tokens.token_allocator, 1024);
	StackAllocator_Init(&self->tokens.value_allocator, 1024);
}

void Lexer_Free(struct Lexer *self)
{
	self->column = 0;
	self->lineno = 0;
	self->file_in = NULL;
	self->tokens.token_count = 0;
	self->tokens.value_count = 0;
	StackAllocator_Free(&self->tokens.token_allocator);
	StackAllocator_Free(&self->tokens.value_allocator);
}
