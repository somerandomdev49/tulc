#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int systemf(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    size_t len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);
    va_start(va, fmt);
    char buf[len + 1];
    vsnprintf(buf, len + 1, fmt, va);
    va_end(va);
    return system(buf);
}

const char *files[] = {
    "lexer.c",
    "parser.c",
    0
};

void command_failed(int ret, const char *fmt, ...) {
    fprintf(stderr, "Command '");
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "' failed with exit code %d\n", ret);
}

const char *flags_release = "-O3";
const char *flags_debug = "-O0 -g";

int main(int argc, char *argv[]) {
    int ret;
    ret = system("mkdir -p build");
    if(ret != 0) return command_failed(ret, "mkdir -p build"), 2;
    
    for(size_t i = 0; files[i] != NULL; ++i) {
        ret = systemf("gcc -c %s -o build/%s.o %s", files[i], files[i], flags_debug);
        if(ret != 0) return command_failed(ret, "gcc -c %s -o build/%s.o %s", files[i], files[i], flags_debug), 2;
    }

    ret = systemf("gcc build/*.o -o %s", "main");
    if(ret != 0) return command_failed(ret, "gcc build/*.o -o %s", "main"), 2;
    return 0;
}
