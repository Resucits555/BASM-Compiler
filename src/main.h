#pragma once

#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <optional>


typedef std::uint_fast8_t ubyte;
typedef std::int_fast8_t sbyte;
typedef std::uint_fast16_t ushort;
typedef std::uint_fast32_t ulong;

namespace fs = std::filesystem;





const ushort maxLineSize = 255;


const double sectionAlignment = 0x1000;
const double fileAlignment = 0x200;

const char sectionNames[][8] = { "ERROR", ".text", ".data", ".bss" };
constexpr ubyte sectionCount = std::size(sectionNames) - 1;
enum Section {
    ABS = -1,
    NOSECTION,
    TEXT,
    DATA,
    BSS
};

extern std::ofstream outFile;
extern std::ifstream srcFile;
extern const char* srcPathStr;


const fpos_t FPOSMAX = std::numeric_limits<fpos_t>::max();
const ubyte terminatingNull = 1;





enum class symbol_class : uint8_t {
    IMAGE_SYM_CLASS_NULL = 0x0,
    IMAGE_SYM_CLASS_AUTOMATIC = 0x1,
    IMAGE_SYM_CLASS_EXTERNAL = 0x2,
    IMAGE_SYM_CLASS_STATIC = 0x3,
    IMAGE_SYM_CLASS_REGISTER = 0x4,
    IMAGE_SYM_CLASS_EXTERNAL_DEF = 0x5,
    IMAGE_SYM_CLASS_LABEL = 0x6,
    IMAGE_SYM_CLASS_UNDEFINED_LABEL = 0x7,
    IMAGE_SYM_CLASS_MEMBER_OF_STRUCT = 0x8,
    IMAGE_SYM_CLASS_ARGUMENT = 0x9,
    IMAGE_SYM_CLASS_STRUCT_TAG = 0xa,
    IMAGE_SYM_CLASS_MEMBER_OF_UNION = 0xb,
    IMAGE_SYM_CLASS_UNION_TAG = 0xc,
    IMAGE_SYM_CLASS_TYPE_DEFINITION = 0xd,
    IMAGE_SYM_CLASS_UNDEFINED_STATIC = 0xe,
    IMAGE_SYM_CLASS_ENUM_TAG = 0xf,
    IMAGE_SYM_CLASS_MEMBER_OF_ENUM = 0x10,
    IMAGE_SYM_CLASS_REGISTER_PARAM = 0x11,
    IMAGE_SYM_CLASS_BIT_FIELD = 0x12,
    IMAGE_SYM_CLASS_AUTOARG = 0x13,
    IMAGE_SYM_CLASS_LASTENT = 0x14,
    IMAGE_SYM_CLASS_BLOCK = 0x64,
    IMAGE_SYM_CLASS_FUNCTION = 0x65,
    IMAGE_SYM_CLASS_END_OF_STRUCT = 0x66,
    IMAGE_SYM_CLASS_FILE = 0x67,
    IMAGE_SYM_CLASS_SECTION = 0x68,
    IMAGE_SYM_CLASS_WEAK_EXTERNAL = 0x69,
    IMAGE_SYM_CLASS_HIDDEN = 0x6a,
    IMAGE_SYM_CLASS_CLR_TOKEN = 0x6b,
    IMAGE_SYM_CLASS_END_OF_FUNCTION = 0xff
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
    ulong line = 0;

public:
    inline ulong getLine() const {
        return line;
    }
    inline void incLine() {
        line++;
    }
};




extern void ProcessInputError(char* inputLine, const fpos_t startOfLine, const ErrorData& errorData);
#pragma pack(push, 2)
//Has to be same size as COFF_Symbol (in symbols.h)
struct SymbolData {
    fpos_t nameRef = 0;
    uint32_t value = 0;
    int16_t sectionNumber = NOSECTION;
    uint16_t size = 0;
    enum symbol_class storageClass = symbol_class::IMAGE_SYM_CLASS_NULL;
    uint8_t nameLen = 0;

    void getName(char* str) const {
        srcFile.seekg(nameRef);
        srcFile.getline(str, nameLen);

        if (srcFile.bad())
            ProcessInputError(str, nameRef, {});
        srcFile.clear();
    }

    inline bool hasFunctionAux() const {
        const ubyte functionSize = 0;
        return (sectionNumber && size == functionSize);
    }
};
#pragma pack(pop)




const int COFF_Symbol_Size = 18;

struct SymbolScopeCount {
    ushort auxiliary = 0;
    ushort staticSymCount = 0;
    ushort globalSymCount = 0;
    ushort externSymCount = 0;

    ushort sum() const {
        return staticSymCount + globalSymCount + externSymCount;
    }

    ushort sumWithAux() const {
        return sum() + auxiliary;
    }

    inline SymbolData* getSymtabEnd(const SymbolData* const symtab) const {
        return (SymbolData*)(symtab + sumWithAux());
    }
};





//Visual studio can't detect that these functions are noreturn, so it throws warnings
extern void Warning(const ErrorData errorData, const char* message);
extern void Error(const char* message);
extern void Error(const char* path, const char* message);
extern void Error(const ErrorData errorData, const char* message);
extern void CompilerError(const ErrorData errorData, const char* message);
extern void ProcessInputError(char* inputLine, const fpos_t startOfLine, const ErrorData& errorData);
extern std::optional<ubyte> findStringInArray(const char* string, const char* array, ushort arraySize, const ubyte arrayStrLen);

extern SymbolScopeCount CountSymbols();
extern SymbolData* FindSymbols(const SymbolScopeCount& symbolCount);
extern void CompileSource(IMAGE_SECTION_HEADER(&sections)[], SymbolData* const symtab, const SymbolData* symtabEnd);
extern void WriteSymbolTable(const fs::path srcPath, IMAGE_SECTION_HEADER(&sections)[], SymbolData* const symtab, const SymbolScopeCount& symbolCount);
extern void WriteCOFFHeader(const fpos_t symtabPos, const fpos_t strtabPos);
extern void WriteSectionTable(IMAGE_SECTION_HEADER(&sections)[]);