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

#pragma pack(push, 2)
struct COFF_Symbol {
    union {
        struct {
            uint32_t zeroes;
            uint32_t offset;
        } longName;
        char shortName[8] = "";
    } name;
    uint32_t value = 0;
    int16_t sectionNumber = NOSECTION;
    enum symbol_type type = symbol_type::IMAGE_SYM_TYPE_NULL;
    enum symbol_class storageClass = symbol_class::SYM_CLASS_NULL;
    uint8_t numberOfAuxSymbols = 0;
};




struct AuxiliaryFunctionDefinition {
    uint32_t TagIndex = 0;
    uint32_t TotalSize = 0;
    uint32_t PointerToLinenumber = 0;
    uint32_t PointerToNextFunction = 0;
    uint8_t unused[2];
};

struct AuxiliarybfAndefSymbol {
    uint8_t unused1[4];
    uint16_t Linenumber = 0;
    uint8_t unused2[6];
    uint32_t PointerToNextFunction = 0;
    uint8_t unused3[2];
};

struct AuxiliaryWeakExternal {
    uint32_t TagIndex = 0;
    uint32_t Characteristics = 0;
    uint8_t unused[10];
};

struct AuxiliarySectionDefinition {
    uint32_t Length = 0;
    uint16_t NumberOfRelocations = 0;
    uint16_t NumberOfLinenumbers = 0;
    uint32_t CheckSum = 0;
    uint32_t Number = 0;
    uint8_t Selection = 0;
    uint8_t unused;
};

struct AuxiliaryCLRToken {
    uint8_t AuxType = 0;
    uint8_t unused1;
    uint32_t SymbolTableIndex = 0;
    uint8_t unused2[12];
};
#pragma pack(pop)





extern SizeType getSymbolBytes(const char* str, ErrorData errorData);