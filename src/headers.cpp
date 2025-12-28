#include <time.h>

#include "main.h"
#include "headers.h"


inline void WriteCOFFHeader(const fpos_t symtabPos, const fpos_t strtabPos) {
    outFile.seekp(0);

    COFF_Header coff;
    coff.numberOfSections = sectionCount;
    coff.timeDateStamp = time(nullptr);
    coff.pointerToSymbolTable = symtabPos;
    coff.numberOfSymbols = (strtabPos - symtabPos) / 18;

    outFile.write((char*)&coff, sizeof(COFF_Header));
}





inline void WriteSectionTable(IMAGE_SECTION_HEADER (&sections)[]) {
    for (ubyte sectionI = 1; sectionI < sectionCount + 1; sectionI++)
        strcpy(sections[sectionI].mName, sectionNames[sectionI]);

    sections[TEXT].mCharacteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
    sections[DATA].mCharacteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
    sections[BSS].mCharacteristics = IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    outFile.write((char*)&sections + sizeof(IMAGE_SECTION_HEADER), sectionCount * sizeof(IMAGE_SECTION_HEADER));
}