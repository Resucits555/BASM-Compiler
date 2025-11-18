#include <time.h>

#include "main.h"
#include "headers.h"


inline void WriteCOFFHeader(std::ofstream& outFile, const fpos_t symtabPos, const fpos_t strtabPos) {
    outFile.seekp(0);

    COFF_Header coff;
    coff.numberOfSections = sectionNumber;
    coff.timeDateStamp = time(nullptr);
    coff.pointerToSymbolTable = symtabPos;
    coff.numberOfSymbols = (strtabPos - symtabPos) / 18;

    outFile.write((char*)&coff, sizeof(COFF_Header));
}





inline void WriteSectionTable(std::ofstream& outFile, IMAGE_SECTION_HEADER (&sections)[]) {
    for (ubyte sectionI = 0; sectionI < sectionNumber; sectionI++)
        strcpy(sections[sectionI].mName, sectionNames[sectionI]);

    sections[0].mCharacteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
    sections[1].mCharacteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
    sections[2].mCharacteristics = IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    outFile.write((char*)&sections, sectionNumber * sizeof(IMAGE_SECTION_HEADER));
}