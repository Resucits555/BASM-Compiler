#pragma once

#include <iostream>
#include <fstream>
#include <expected>
#include <optional>


typedef std::uint_fast8_t ubyte;
typedef std::int_fast8_t sbyte;
typedef std::uint_fast16_t ushort;
typedef std::uint_fast32_t ulong;

namespace fs = std::filesystem;





const double sectionAlignment = 0x1000;
const double fileAlignment = 0x200;

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

    friend void CompileSource(std::ofstream&, const char*, IMAGE_SECTION_HEADER*);

public:
    ErrorData(const ulong inLine = 0, const char* inPath = nullptr) {
        line = inLine;
        path = inPath;
    }

    inline ulong getLine() const {
        return line;
    }

    inline char* getPath() const {
        return (char*)&path;
    }
};





extern void Error(const char* message);
extern void ErrorNoLine(const ErrorData& errorData, const char* message);
extern void Error(const ErrorData& errorData, const char* message);
extern void CompilerError(const ErrorData& errorData, const char* message);
extern ulong roundUp(const ulong& number, const double& roundup);
extern std::optional<ubyte> findStringInArray(const char* string, const char* array, ushort arraySize, ubyte stringSize);

extern void CompileSource(std::ofstream& output, const char* srcPath, IMAGE_SECTION_HEADER* sections);
extern void WriteCOFFHeader(std::ofstream& exeFile, const ushort sectionNumber);