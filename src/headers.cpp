#include <time.h>

#include "main.h"
#include "headers.h"


inline static void writeMZHeader(std::ofstream& exeFile) {
    exeFile.write("MZ", 2);

    exeFile.seekp(0x04);
    ulong sizeOfImagePages = roundUp(sizeOfFile, fileAlignment) / 512;
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





inline static void writeCOFFHeader(std::ofstream& exeFile) {
    exeFile.seekp(e_lfanew);

    COFF_Header header;
    exeFile.write((char*)&header, 8);

    int32_t epochTime = time(0);
    exeFile.write((char*)&epochTime, 4);

    exeFile.write((char*)&header.mPointerToSymbolTable, 12);
}





inline static void writeOptionalHeader(std::ofstream& exeFile) {
    Pe32OptionalHeader OH;
    exeFile.write((char*)&OH, 96);


    exeFile.seekp((ubyte)exeFile.tellp() + 8);

    IMAGE_DATA_DIRECTORY importTable = { rdataRVA + 8, 0x28 };
    exeFile.write((char*)&importTable, sizeof(IMAGE_DATA_DIRECTORY));

    exeFile.seekp((ushort)exeFile.tellp() + (14 * sizeof(IMAGE_DATA_DIRECTORY)));
}





inline static void writeSectionHeader(std::ofstream& exeFile) {
    IMAGE_SECTION_HEADER text;
    strcpy(text.mName, ".text");
    text.mVirtualSize = codeSize;
    text.mVirtualAddress = codeRVA;
    text.mSizeOfRawData = roundUp(codeSize, fileAlignment);
    text.mPointerToRawData = codeAddress;
    text.mCharacteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

    exeFile.write((char*)&text, sizeof(text));

    IMAGE_SECTION_HEADER rdata;
    strcpy(rdata.mName, ".rdata");
    rdata.mVirtualSize = rdataSize;
    rdata.mVirtualAddress = rdataRVA;
    rdata.mSizeOfRawData = roundUp(rdataSize, fileAlignment);
    rdata.mPointerToRawData = rdataAddress;
    rdata.mCharacteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;

    exeFile.write((char*)&rdata, sizeof(rdata));
}





inline void WriteHeaders(std::ofstream& exeFile) {
    exeFile.seekp(0);

    writeMZHeader(exeFile);
    writeDOSStub(exeFile);

    writeCOFFHeader(exeFile);
    writeOptionalHeader(exeFile);
    writeSectionHeader(exeFile);
}