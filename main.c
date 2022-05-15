#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

void StackAllocator_Init(struct StackAllocator *self, size_t size)
{
	self->reserved = size;
	self->size = 0;
	self->data = NULL;
	self->offset = 0;

	// Only allocate when size is greater than 0.
	if(size != 0) self->data = malloc(size);
}

void *StackAllocator_Alloc(struct StackAllocator *self, size_t size)
{
	// Internal buffer doesn't have enough space, reallocate.
	if(self->reserved < self->size + size)
	{
		// Align the buffer size to StackAllocator_ALIGNMENT
		self->reserved += -self->reserved & (StackAllocator_ALIGNMENT - 1);
		
		// Increase the buffer size until it is enough to fit self->size + size
		while(self->reserved < self->size + size)
			self->reserved += StackAllocator_ALIGNMENT;

		// Reallocate the buffer
		self->data = realloc(self->data, self->reserved);
	}

	// Store the pointer to the newly allocated item
	void *pointer = self->data + self->offset;

	self->offset += size;
	return pointer;
}

void StackAllocator_Free(struct StackAllocator *self)
{
	// Make sure the size and capacity are 0.
	self->size = 0;
	self->reserved = 0;
	
	// No need to deallocate the buffer.
	if(self->data == NULL) return;

	free(self->data);
}

int main(int argc, char *argv[])
{
	FILE *fin = fopen("test.tul", "r");
	struct Lexer lexer;
	Lexer_Init(&lexer, fin);
	Lexer_Read(&lexer);

	struct Token *toks = lexer.tokens.token_allocator.data;
	for(size_t i = 0; i < lexer.tokens.token_count; ++i)
	{
		struct Token *tok = &toks[i];
		if(tok->type == TokenType_eIdentifier)
			printf("[id]  %s\n", tok->value);
		else if(tok->type == TokenType_eInteger)
			printf("[int] %zu\n", tok->integer);
		else printf("[#]   '%c'\n", tok->type);
	}

	Lexer_Free(&lexer);

	fclose(fin);
	return 0;
}
