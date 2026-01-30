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
extern const char* srcPathStr;


const fpos_t FPOSMAX = std::numeric_limits<fpos_t>::max();
const ubyte terminatingNull = 1;
const ubyte requiredSymbolSpace = 2;




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




enum reloc_type : uint16_t
{
    IMAGE_REL_AMD64_ABSOLUTE = 0x0,
    IMAGE_REL_AMD64_ADDR64 = 0x1,
    IMAGE_REL_AMD64_ADDR32 = 0x2,
    IMAGE_REL_AMD64_ADDR32NB = 0x3,
    IMAGE_REL_AMD64_REL32 = 0x4,
    IMAGE_REL_AMD64_REL32_1 = 0x5,
    IMAGE_REL_AMD64_REL32_2 = 0x6,
    IMAGE_REL_AMD64_REL32_3 = 0x7,
    IMAGE_REL_AMD64_REL32_4 = 0x8,
    IMAGE_REL_AMD64_REL32_5 = 0x9,
    IMAGE_REL_AMD64_SECTION = 0xa,
    IMAGE_REL_AMD64_SECREL = 0xb,
    IMAGE_REL_AMD64_SECREL7 = 0xc,
    IMAGE_REL_AMD64_TOKEN = 0xd,
    IMAGE_REL_AMD64_SREL32 = 0xe,
    IMAGE_REL_AMD64_PAIR = 0xf,
    IMAGE_REL_AMD64_SSPAN32 = 0x10
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
    enum symbol_class storageClass = symbol_class::IMAGE_SYM_CLASS_NULL;
    uint8_t nameLen = 0;

    void getName(char* str) const {
        srcFile.clear();
        srcFile.seekg(nameRef);
        srcFile.get(str, nameLen);

        if (srcFile.bad()) {
            char errIntro[33 + _MAX_PATH] = "ERROR: Failed to read from file ";
            perror(strcat(errIntro, srcPathStr));
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
    reloc_type type;
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