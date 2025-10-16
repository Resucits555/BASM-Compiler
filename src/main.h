#pragma once

#include <iostream>
#include <fstream>
#include <expected>

typedef std::uint_fast8_t ubyte;
typedef std::int_fast8_t sbyte;
typedef std::uint_fast16_t ushort;
typedef std::uint_fast32_t ulong;

namespace fs = std::filesystem;


const double sectionAlignment = 0x1000;

const double minHeadersSize = 0x500;
const ushort baseOfCode = std::ceil(minHeadersSize / sectionAlignment) * sectionAlignment;
extern ulong sizeOfCode;
extern ulong sizeOfImage;

//official max size for a section name is 8 chars
const char sections[][9] = { "bss", "data", "text" };


extern void Error(const char*);
extern void Error(const char*, ulong&);
extern void fillNullUntil(std::ofstream&, int);
extern void WriteHeaders(std::ofstream&);
extern void CompileSource(const fs::path&, std::ofstream&);