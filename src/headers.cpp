#include <time.h>

#include "main.h"
#include "headers.h"


inline void WriteCOFFHeader(const fpos_t symtabPos, const SymbolScopeCount& symbolCount) {
    outFile.seekp(0);

    COFF_Header coff;
    coff.numberOfSections = symbolCount.sectionSymCount;
    coff.timeDateStamp = time(nullptr);
    coff.pointerToSymbolTable = symtabPos;
    coff.numberOfSymbols = symbolCount.sum() + fileSymbol;

    outFile.write((char*)&coff, sizeof(COFF_Header));
}





inline void WriteSectionTable(SectionTab& sections, const ubyte sectionSymCount) {
    for (ubyte sectionI = 1; sectionI < sectionCount + 1; sectionI++) {
        strcpy(sections.header((Section)sectionI)->mName, sectionNames[sectionI]);
        SectionHeader* section = sections.header((Section)sectionI);
        section->section = NOSECTION;
        section->mCharacteristics = sectionCharacteristics[sectionI];
    }

    outFile.write((char*)&sections + sizeof(SectionHeader), sectionSymCount * sizeof(SectionHeader));
}