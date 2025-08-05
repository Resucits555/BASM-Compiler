#pragma once

#include <iostream>
#include <fstream>
#include <expected>

typedef unsigned char ubyte;
typedef signed char sbyte;
typedef unsigned short ushort;
typedef unsigned long ulong;


const ushort sectionAlignment = 0x1000;

const double minHeadersSize = 0x500;
const ulong baseOfCode = std::ceil(minHeadersSize / (double)sectionAlignment) * sectionAlignment;

const char sections[][8] = { "bss", "data", "text" };


extern void Error(const char*);
extern void Error(const char*, unsigned long&);
extern void fillNullUntil(std::ofstream&, int);
extern void WriteHeaders(std::ofstream&);
extern void CompileSource(char*, std::ofstream&);