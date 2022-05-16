# tulc

> C Language Superset (not strict), middle ground between C and C++

## Future Example

> See [future.tul](/future.tul)

```c++
// A class is like an interface
class AnyAllocator;
// This class has the same semantics as void*
class Pointer = void*;

// Returns some type that has the same semantics as void*
// Takes a pointer to a type that has the AnyAllocator class.
class Pointer T Allocate(class AnyAllocator T *this);

// Takes an allocator and a pointer
void DeAllocate(class AnyAllocator T *this, class Pointer T ptr);

// Same as C
struct Person
{
    const char *name;
    unsigned int age;
};

// Takes a const pointer
void Greet(const struct Person *this, FILE *output)
{
    fprintf(output, "Hello, %s!\n", this->name);
}

// Takes any allocator
char *ToString(const struct Person *this, class AnyAllocator T *allocator)
{
    const char *fmt = "%s, %d";

    size_t size = snprintf(NULL, 0, fmt, this->name, this->age);

    // Calls the Allocate function for T, which is valid because T is AnyAllocator
    char *c = allocator:Allocate(size + 1);
    
    snprintf(c, size + 1, fmt, this->name, this->age);
    return c;
}
```

## Building

```bash
# Compile the build system, one time
gcc build.c -o cbuild
# Compile the project into 'main'
./cbuild
# Run
./main ...
```

## Resources

> Resources that might be needed for developming the compiler

- [ANSI C Grammar (yacc)](https://www.lysator.liu.se/c/ANSI-C-grammar-y.html#type-specifier)
- [C Standard Draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf#page=476)
