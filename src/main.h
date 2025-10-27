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
const double fileAlignment = 0x200;


const ulong baseOfCode = sectionAlignment;
extern ulong sizeOfCode;
extern ulong sizeOfImageFile;
extern ushort headerSize;

extern bool long64bitMode;



extern void Error(const char* message);
extern void Error(const char* message, ulong line);
extern void CompilerError(const char* message, ulong line);
extern void WriteHeaders(std::ofstream&);
extern void CompileSource(const fs::path&, std::ofstream&);
extern ulong roundUp(const ulong& number, const double& roundup);