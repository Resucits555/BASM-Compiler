#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <optional>


typedef std::uint_fast8_t ubyte;
typedef std::int_fast8_t sbyte;
typedef std::uint_fast16_t ushort;
typedef std::uint_fast32_t ulong;

namespace fs = std::filesystem;





const ubyte maxLineSize = 255;


const double sectionAlignment = 0x1000;
const double fileAlignment = 0x200;

const char sectionNames[][8] = { ".text", ".data", ".bss" };
constexpr ubyte sectionNumber = std::size(sectionNames);
const ubyte coffHeaderSize = 20;
const ubyte sectionHeaderSize = 40;

extern bool long64bitMode;





struct IMAGE_SECTION_HEADER {
    char mName[8] = "";
    uint32_t mVirtualSize = 0;
    uint32_t mVirtualAddress = 0;
    uint32_t mSizeOfRawData = 0;
    uint32_t mPointerToRawData = 0;
    uint32_t mPointerToRelocations = 0;
    uint32_t mPointerToLinenumbers = 0;
    uint16_t mNumberOfRelocations = 0;
    uint16_t mNumberOfLinenumbers = 0;
    uint32_t mCharacteristics = 0;
};




class ErrorData {
    ulong line;
    const char* path;

    friend void CompileSource(std::ofstream& outFile, std::ifstream& srcFile, const char* srcPath, IMAGE_SECTION_HEADER(&sections)[]);

public:
    ErrorData(const ulong inLine = 0, const char* inPath = nullptr) {
        line = inLine;
        path = inPath;
    }

    inline ulong getLine() const {
        return line;
    }

    inline const char* getPath() const {
        return path;
    }
};





extern void Error(const char* message);
extern void Error(const char* path, const char* message);
extern void Error(const ErrorData& errorData, const char* message);
extern void CompilerError(const ErrorData& errorData, const char* message);
extern std::optional<ubyte> findStringInArray(const char* string, const char* array, ushort arraySize, ubyte stringSize);

extern void CompileSource(std::ofstream& output, std::ifstream& srcFile, const char* srcPath, IMAGE_SECTION_HEADER(&sections)[]);
extern void WriteSymbolTable(std::ofstream& outFile, std::ifstream& srcFile, fs::path sourcePath, IMAGE_SECTION_HEADER(&sections)[]);
extern void WriteCOFFHeader(std::ofstream& outFile, const fpos_t symtabPos, const fpos_t strtabPos);
extern void WriteSectionTable(std::ofstream& outFile, IMAGE_SECTION_HEADER(&sections)[]);