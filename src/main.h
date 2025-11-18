#pragma once

#include <filesystem>
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

constexpr char sectionNames[][8] = { ".text", ".data", ".bss" };
constexpr ubyte sectionNumber = std::size(sectionNames);
const ubyte coffHeaderSize = 20;
const ubyte sectionHeaderSize = 40;

extern bool long64bitMode;





enum pe_section_flags : uint32_t {
    IMAGE_SCN_RESERVED_0001 = 0x1,
    IMAGE_SCN_RESERVED_0002 = 0x2,
    IMAGE_SCN_RESERVED_0004 = 0x4,
    IMAGE_SCN_TYPE_NO_PAD = 0x8,
    IMAGE_SCN_RESERVED_0010 = 0x10,
    IMAGE_SCN_CNT_CODE = 0x20,
    IMAGE_SCN_CNT_INITIALIZED_DATA = 0x40,
    IMAGE_SCN_CNT_UNINITIALIZED_DATA = 0x80,
    IMAGE_SCN_LNK_OTHER = 0x100,
    IMAGE_SCN_LNK_INFO = 0x200,
    IMAGE_SCN_RESERVED_0400 = 0x400,
    IMAGE_SCN_LNK_REMOVE = 0x800,
    IMAGE_SCN_LNK_COMDAT = 0x1000,
    IMAGE_SCN_GPREL = 0x8000,
    IMAGE_SCN_MEM_PURGEABLE = 0x10000,
    IMAGE_SCN_MEM_16BIT = 0x20000,
    IMAGE_SCN_MEM_LOCKED = 0x40000,
    IMAGE_SCN_MEM_PRELOAD = 0x80000,
    IMAGE_SCN_LNK_NRELOC_OVFL = 0x1000000,
    IMAGE_SCN_MEM_DISCARDABLE = 0x2000000,
    IMAGE_SCN_MEM_NOT_CACHED = 0x4000000,
    IMAGE_SCN_MEM_NOT_PAGED = 0x8000000,
    IMAGE_SCN_MEM_SHARED = 0x10000000,
    IMAGE_SCN_MEM_EXECUTE = 0x20000000,
    IMAGE_SCN_MEM_READ = 0x40000000,
    IMAGE_SCN_MEM_WRITE = 0x80000000
};

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

    friend void CompileSource(std::ofstream& outFile, std::ifstream& sourceFile, const char* srcPath, IMAGE_SECTION_HEADER(&sections)[]);

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

extern void CompileSource(std::ofstream& output, std::ifstream& sourceFile, const char* srcPath, IMAGE_SECTION_HEADER(&sections)[]);
extern void WriteSymbolTable(std::ofstream& outFile, std::ifstream& sourceFile, fs::path sourcePath, IMAGE_SECTION_HEADER(&sections)[]);
extern void WriteCOFFHeader(std::ofstream& outFile, const fpos_t symtabPos, const fpos_t strtabPos);
extern void WriteSectionTable(std::ofstream& outFile, IMAGE_SECTION_HEADER(&sections)[]);