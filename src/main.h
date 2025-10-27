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


extern ushort headerSize;
const ulong codeRVA = sectionAlignment;
extern ulong codeAddress;
extern ulong codeSize;
extern ulong rdataRVA;
extern ulong rdataAddress;
extern ulong rdataSize;

extern ulong sizeOfFile;

extern bool long64bitMode;



extern void Error(const char* message);
extern void Error(const char* message, ulong line);
extern void CompilerError(const char* message, ulong line);
extern ulong roundUp(const ulong& number, const double& roundup);

extern void CompileSource(const fs::path& srcPath, std::ofstream& exeFile);
extern void WriteImports(std::ofstream& exeFile);
extern void WriteHeaders(std::ofstream& exeFile);