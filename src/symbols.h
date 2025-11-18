#pragma once


enum class symbol_type : uint16_t {
    IMAGE_SYM_TYPE_NULL = 0x0,
    IMAGE_SYM_TYPE_VOID = 0x1,
    IMAGE_SYM_TYPE_CHAR = 0x2,
    IMAGE_SYM_TYPE_SHORT = 0x3,
    IMAGE_SYM_TYPE_INT = 0x4,
    IMAGE_SYM_TYPE_LONG = 0x5,
    IMAGE_SYM_TYPE_FLOAT = 0x6,
    IMAGE_SYM_TYPE_DOUBLE = 0x7,
    IMAGE_SYM_TYPE_STRUCT = 0x8,
    IMAGE_SYM_TYPE_UNION = 0x9,
    IMAGE_SYM_TYPE_ENUM = 0xa,
    IMAGE_SYM_TYPE_MOE = 0xb,
    IMAGE_SYM_TYPE_BYTE = 0xc,
    IMAGE_SYM_TYPE_WORD = 0xd,
    IMAGE_SYM_TYPE_UINT = 0xe,
    IMAGE_SYM_TYPE_DWORD = 0xf,
    MS_IMAGE_SYM_TYPE_FUNCTION = 0x20
};

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

//Do not use sizeof to determine size of COFF_Symbol struct
const ubyte COFF_Symbol_Size = 18;

struct COFF_Symbol {
    union {
        struct {
            uint32_t zeroes;
            uint32_t offset;
        } longName;
        char shortName[8] = "";
    } name;
    uint32_t value;
    int16_t sectionNumber;
    enum symbol_type type;
    enum symbol_class storageClass;
    uint8_t numberOfAuxSymbols;
};





union auxFilename {
    struct {
        uint32_t zeroes;
        uint32_t offset;
    } longName;
    char shortName[18];
};





// Auxiliary Symbol Formats
struct AuxiliaryFunctionDefinition {
    uint32_t TagIndex;
    uint32_t TotalSize;
    uint32_t PointerToLinenumber;
    uint32_t PointerToNextFunction;
    char unused[2];
};

struct AuxiliarybfAndefSymbol {
    uint8_t unused1[4];
    uint16_t Linenumber;
    uint8_t unused2[6];
    uint32_t PointerToNextFunction;
    uint8_t unused3[2];
};

struct AuxiliaryWeakExternal {
    uint32_t TagIndex;
    uint32_t Characteristics;
    uint8_t unused[10];
};

enum WeakExternalCharacteristics : unsigned int {
    IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY = 1,
    IMAGE_WEAK_EXTERN_SEARCH_LIBRARY = 2,
    IMAGE_WEAK_EXTERN_SEARCH_ALIAS = 3,
    IMAGE_WEAK_EXTERN_ANTI_DEPENDENCY = 4
};

struct AuxiliarySectionDefinition {
    uint32_t Length = 0;
    uint16_t NumberOfRelocations = 0;
    uint16_t NumberOfLinenumbers = 0;
    uint32_t CheckSum = 0;
    uint32_t Number = 0;
    uint8_t Selection = 0;
    char unused = 0;
};

struct AuxiliaryCLRToken {
    uint8_t AuxType;
    uint8_t unused1;
    uint32_t SymbolTableIndex;
    char unused2[12];
};