#pragma once
#include <iostream>
#include <fstream>

typedef unsigned char ubyte;
typedef signed char sbyte;
typedef unsigned short ushort;

struct intPair {
    int a;
    int b;
};

extern void Error(const char*, unsigned int);
extern void WriteHeaders(char*);
extern void CompileSource(char*);