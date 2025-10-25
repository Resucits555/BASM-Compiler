#include <time.h>

#include "main.h"


const ulong e_lfanew = 0x80;
const ushort optionalHeaderSize = 96;


inline static void writeMZHeader(std::ofstream& exeFile) {
    exeFile.write("MZ", 2);

    exeFile.seekp(0x04);
    ulong sizeOfImagePages = roundUp(sizeOfImage, fileAlignment) / 512;
    exeFile.write((char*)&sizeOfImagePages, 2);

    exeFile.seekp(0x08);
    ulong headerSizeParagraphs = roundUp(headerSize, fileAlignment) / 16;
    exeFile.write((char*)&headerSizeParagraphs, 2);

    exeFile.seekp(0x0C);
    exeFile.write("\xFF\xFF", 2);

    exeFile.seekp(0x3C);
    exeFile.write((char*)&e_lfanew, 4);
}





inline static void writeDOSStub(std::ofstream& exeFile) {
    const uint8_t DOSStub[] =
    { 0x0E, 0x1F, 0xBA, 0x0E, 00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68,
    0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F,
    0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20,
    0x6D, 0x6F, 0x64, 0x65, 0x2E, 0x0D, 0x0D, 0x0A, 0x24 };

    exeFile.write((char*)&DOSStub, sizeof(DOSStub));
}





enum coff_characteristics : uint16_t
{
    IMAGE_FILE_RELOCS_STRIPPED = 0x1,
    IMAGE_FILE_EXECUTABLE_IMAGE = 0x2,
    IMAGE_FILE_LINE_NUMS_STRIPPED = 0x4,
    IMAGE_FILE_LOCAL_SYMS_STRIPPED = 0x8,
    IMAGE_FILE_AGGRESIVE_WS_TRIM = 0x10,
    IMAGE_FILE_LARGE_ADDRESS_AWARE = 0x20,
    IMAGE_FILE_BYTES_REVERSED_LO = 0x80,
    IMAGE_FILE_32BIT_MACHINE = 0x100,
    IMAGE_FILE_DEBUG_STRIPPED = 0x200,
    IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP = 0x400,
    IMAGE_FILE_NET_RUN_FROM_SWAP = 0x800,
    IMAGE_FILE_SYSTEM = 0x1000,
    IMAGE_FILE_DLL = 0x2000,
    IMAGE_FILE_UP_SYSTEM_ONLY = 0x4000,
    IMAGE_FILE_BYTES_REVERSED_HI = 0x8000
};

struct PeHeader {
    uint32_t mMagic = 0x00004550;
    uint16_t mMachine = 0x14C;
    uint16_t mNumberOfSections = 1;
    //uint32_t mTimeDateStamp;
    uint32_t mPointerToSymbolTable = 0;
    uint32_t mNumberOfSymbols = 0;
    uint16_t mSizeOfOptionalHeader = optionalHeaderSize;
    uint16_t mCharacteristics = IMAGE_FILE_RELOCS_STRIPPED | IMAGE_FILE_EXECUTABLE_IMAGE
        | IMAGE_FILE_LINE_NUMS_STRIPPED | IMAGE_FILE_LOCAL_SYMS_STRIPPED | IMAGE_FILE_32BIT_MACHINE;

    inline void write(std::ofstream& exeFile) const {
        exeFile.seekp(e_lfanew);

        exeFile.write((char*)&mMagic, 8);

        int32_t epochTime = time(0);
        exeFile.write((char*)&epochTime, 4);

        exeFile.write((char*)&mPointerToSymbolTable, 12);
    }
};





enum pe_subsystem : ushort
{
    IMAGE_SUBSYSTEM_UNKNOWN = 0x0,
    IMAGE_SUBSYSTEM_NATIVE = 0x1,
    IMAGE_SUBSYSTEM_WINDOWS_GUI = 0x2,
    IMAGE_SUBSYSTEM_WINDOWS_CUI = 0x3,
    IMAGE_SUBSYSTEM_OS2_CUI = 0x5,
    IMAGE_SUBSYSTEM_POSIX_CUI = 0x7,
    IMAGE_SUBSYSTEM_NATIVE_WINDOWS = 0x8,
    IMAGE_SUBSYSTEM_WINDOWS_CE_GUI = 0x9,
    IMAGE_SUBSYSTEM_EFI_APPLICATION = 0xa,
    IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER = 0xb,
    IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER = 0xc,
    IMAGE_SUBSYSTEM_EFI_ROM = 0xd,
    IMAGE_SUBSYSTEM_XBOX = 0xe,
    IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION = 0x10
};

struct Pe32OptionalHeader {
    uint16_t mMagic = 0x010b;
    uint8_t  mMajorLinkerVersion = 0;
    uint8_t  mMinorLinkerVersion = 0;
    uint32_t mSizeOfCode = roundUp(sizeOfCode, fileAlignment);
    uint32_t mSizeOfInitializedData = 0;
    uint32_t mSizeOfUninitializedData = 0;
    uint32_t mAddressOfEntryPoint = baseOfCode;
    uint32_t mBaseOfCode = baseOfCode;
    uint32_t mBaseOfData = 0;
    uint32_t mImageBase = 0x400000;
    uint32_t mSectionAlignment = sectionAlignment;
    uint32_t mFileAlignment = fileAlignment;
    uint16_t mMajorOperatingSystemVersion = 6;
    uint16_t mMinorOperatingSystemVersion = 0;
    uint16_t mMajorImageVersion = 0;
    uint16_t mMinorImageVersion = 0;
    uint16_t mMajorSubsystemVersion = 6;
    uint16_t mMinorSubsystemVersion = 0;
    uint32_t mWin32VersionValue = 0;
    uint32_t mSizeOfImage = 0x2000;
    uint32_t mSizeOfHeaders = headerSize;
    uint32_t mCheckSum = 0;
    uint16_t mSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    uint16_t mDllCharacteristics = 0;
    uint32_t mSizeOfStackReserve = 0x1000;
    uint32_t mSizeOfStackCommit = 0x1000;
    uint32_t mSizeOfHeapReserve = 0x1000;
    uint32_t mSizeOfHeapCommit = 0x1000;
    uint32_t mLoaderFlags = 0;
    uint32_t mNumberOfRvaAndSizes = 0;
};





enum pe_section_flags : ulong {
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
    char mName[8];
    uint32_t mVirtualSize;

    uint32_t mVirtualAddress;
    uint32_t mSizeOfRawData;

    uint32_t mPointerToRawData;
    uint32_t mPointerToRelocations;

    uint32_t mPointerToLinenumbers;
    uint16_t mNumberOfRelocations;

    uint16_t mNumberOfLinenumbers;
    uint32_t mCharacteristics;
};

inline static void writeSectionHeader(std::ofstream& exeFile) {
    IMAGE_SECTION_HEADER text = {
    ".text", sizeOfCode,
    baseOfCode, roundUp(sizeOfCode, fileAlignment),
    0x200, 0,
    0, 0,
    0, IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ };

    exeFile.write((char*)&text, sizeof(IMAGE_SECTION_HEADER));
}





inline void WriteHeaders(std::ofstream& exeFile) {
    exeFile.seekp(0);

    writeMZHeader(exeFile);
    writeDOSStub(exeFile);

    PeHeader COFF;
    COFF.write(exeFile);

    Pe32OptionalHeader OH;
    exeFile.write((char*)&OH, optionalHeaderSize);
    const ushort mSizeOfHeadersLocation = (ushort)exeFile.tellp() - 36; //52 for PE32+

    writeSectionHeader(exeFile);
}