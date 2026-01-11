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
    for (ubyte sectionI = 1; sectionI < sectionSymCount + 1; sectionI++) {
        SectionHeader* section = sections.headers + sectionI;
        strcpy(section->mName, sectionNames[section->section]);
        section->mCharacteristics = sectionCharacteristics[section->section];
        section->section = NOSECTION;
    }

    outFile.write((char*)&sections + sizeof(SectionHeader), sectionSymCount * sizeof(SectionHeader));
}