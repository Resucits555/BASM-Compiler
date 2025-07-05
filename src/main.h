#pragma once
#include <iostream>
#include <fstream>

typedef unsigned char ubyte;
typedef signed char sbyte;
typedef unsigned short ushort;

struct substr {
    int a;
    int b;

    inline int end() {
        return a + b;
    }
};

extern ushort line;

extern void Error(const char*);
extern void WriteHeaders(char*);
extern void CompileSource(char*);