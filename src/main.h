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

#if defined(_MSC_VER)
#  define NORETURN __declspec(noreturn)
#else
#  define NORETURN [[noreturn]]
#endif





const ushort maxLineSize = 255;

const char sectionNames[][9] = { "ERROR", ".text", ".comment", ".drectve", ".bss", ".data", ".rdata" };
constexpr ubyte sectionCount = std::size(sectionNames) - 1;
enum Section : int8_t {
    ABSOLUTE = -1,
    NOSECTION,
    TEXT,
    COMMENTSEC,
    DRECTVE,
    BSS, //section BSS and above should only save variables
    DATA,
    RDATA
};

extern std::ofstream outFile;
extern std::ifstream srcFile;
extern const char* srcPathRaw;


const fpos_t FPOSMAX = std::numeric_limits<fpos_t>::max();
const ubyte terminatingNull = 1;
const ubyte requiredSymbolSpace = 2;




enum class symbol_class : uint8_t {
    SYM_CLASS_NULL = 0x0,
    AUTOMATIC = 0x1,
    EXTERNAL = 0x2,
    STATIC = 0x3,
    REGISTER = 0x4,
    EXTERNAL_DEF = 0x5,
    LABEL = 0x6,
    UNDEFINED_LABEL = 0x7,
    MEMBER_OF_STRUCT = 0x8,
    ARGUMENT = 0x9,
    STRUCT_TAG = 0xa,
    MEMBER_OF_UNION = 0xb,
    UNION_TAG = 0xc,
    TYPE_DEFINITION = 0xd,
    UNDEFINED_STATIC = 0xe,
    ENUM_TAG = 0xf,
    MEMBER_OF_ENUM = 0x10,
    REGISTER_PARAM = 0x11,
    BIT_FIELD = 0x12,
    AUTOARG = 0x13,
    LASTENT = 0x14,
    BLOCK = 0x64,
    FUNCTION = 0x65,
    END_OF_STRUCT = 0x66,
    FILE = 0x67,
    SECTION = 0x68,
    WEAK_EXTERNAL = 0x69,
    HIDDEN = 0x6a,
    CLR_TOKEN = 0x6b,
    END_OF_FUNCTION = 0xff
};




enum class amd_reloc_type : uint16_t
{
    ABSOLUTE = 0x0,
    ADDR64 = 0x1,
    ADDR32 = 0x2,
    ADDR32NB = 0x3,
    REL32 = 0x4,
    REL32_1 = 0x5,
    REL32_2 = 0x6,
    REL32_3 = 0x7,
    REL32_4 = 0x8,
    REL32_5 = 0x9,
    SECTION = 0xa,
    SECREL = 0xb,
    SECREL7 = 0xc,
    TOKEN = 0xd,
    SREL32 = 0xe,
    PAIR = 0xf,
    SSPAN32 = 0x10
};





struct SectionHeader {
    char mName[8] = "";
    char unused1[8];
    uint32_t sizeOfRawData = 0;
    uint32_t pointerToRawData = 0;
    uint32_t pointerToRelocations = 0;
    char unused2[4];
    uint16_t numberOfRelocations = 0;
    uint16_t sectionIndex = 0;
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




enum class SizeType : uint16_t {
    NONE,
    BYTE,
    WORD,
    FUNCTION,
    DWORD,
    LABEL,
    QWORD = 8
};



extern void ProcessInputError(char* inputLine, const ErrorData& errorData);

//Has to be same size as COFF_Symbol (in symbols.h)
#pragma pack(push, 2)
struct SymbolData {
    fpos_t nameRef = 0;
    uint32_t value = 0;
    int16_t sectionNumber = NOSECTION;
    enum SizeType size = SizeType::NONE;
    enum symbol_class storageClass = symbol_class::SYM_CLASS_NULL;
    uint8_t nameLen = 0;

    void getName(char* str) const {
        srcFile.clear();
        srcFile.seekg(nameRef);
        srcFile.get(str, nameLen);

        if (srcFile.bad()) {
            char errIntro[33 + _MAX_PATH] = "ERROR: Failed to read from file ";
            perror(strcat(errIntro, srcPathRaw));
            exit(-1);
        }
    }

    inline bool isDefinedFunction() const {
        return (sectionNumber && size == SizeType::FUNCTION);
    }
};




struct COFF_Relocation
{
    uint32_t virtualAddress;
    uint32_t symbolTableIndex;
    amd_reloc_type type;
};
#pragma pack(pop)




const int COFF_Symbol_Size = 18;

struct SymbolScopeCount {
    ushort auxiliary = 0;
    ushort staticSymCount = 0;
    ushort globalSymCount = 0;
    ushort externSymCount = 0;
    ubyte sectionSymCount = 0;
    ushort labelCount = 0;

    ushort getReducedSymtabSize() const {
        return staticSymCount + globalSymCount + externSymCount + auxiliary + labelCount;
    }

    inline ushort sum() const {
        return 2 * sectionSymCount + getReducedSymtabSize();
    }

    inline SymbolData* getReducedSymtabEnd(const SymbolData* const symtab) const {
        return (SymbolData*)(symtab + getReducedSymtabSize());
    }
};





//Visual studio can't detect that these functions are noreturn, so it throws warnings
extern void Warning(const ErrorData errorData, const char* message);
extern NORETURN void Error(const char* message);
extern NORETURN void Error(const char* path, const char* message);
extern NORETURN void Error(const ErrorData errorData, const char* message);
extern NORETURN void CompilerError(const char* message);
extern NORETURN void CompilerError(const ErrorData errorData, const char* message);
extern std::optional<ubyte> findStringInArray(const char* string, const char* array, ushort arraySize, const ubyte arrayStrLen);
extern SymbolData* findSymbol(const char* name, SymbolData* symtab, const SymbolData* symtabEnd, ErrorData errorData);

extern SymbolScopeCount CountSymbols(SectionHeader* sections);
extern SymbolData* FindSymbols(const SymbolScopeCount& symbolCount, SectionHeader* sections);
extern void CompileSource(SectionHeader* sections, SymbolData* const symtab, const SymbolData* symtabEnd);
extern void WriteSymbolTable(const fs::path srcPath, SectionHeader* sections, SymbolData* const symtab, const SymbolScopeCount& symbolCount);
extern void WriteCOFFHeader(const fpos_t symtabPos, const SymbolScopeCount& symbolCount);
extern void WriteSectionTable(SectionHeader* sections);