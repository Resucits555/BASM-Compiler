#include <time.h>

#include "main.h"
#include "headers.h"


inline void WriteCOFFHeader(const fpos_t symtabPos, const SymbolScopeCount& symbolCount) {
    outFile.seekp(0);

    COFF_Header coff;
    coff.numberOfSections = symbolCount.sectionSymCount;
    coff.timeDateStamp = time(nullptr);
    coff.pointerToSymbolTable = symtabPos;
    coff.numberOfSymbols = symbolCount.sum() + requiredSymbolSpace;

    outFile.write((char*)&coff, sizeof(COFF_Header));
}





inline void WriteSectionTable(SectionHeader* sections) {
    for (ubyte sectionI = 1; sectionI < std::size(sectionNames); sectionI++) {
        SectionHeader* section = sections;
        for (; section->sectionIndex != sectionI; section++)
            if (section - sections > std::size(sectionNames))
                return;

        strcpy(section->mName, sectionNames[section - sections]);
        section->mCharacteristics = sectionCharacteristics[section - sections];
        section->sectionIndex = 0;
        outFile.write((char*)section, sizeof(SectionHeader));
    }
}