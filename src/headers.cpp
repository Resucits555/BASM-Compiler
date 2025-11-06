#include <time.h>

#include "main.h"
#include "headers.h"


inline void WriteCOFFHeader(std::ofstream& outFile, const ushort sectionNumber) {
    COFF_Header coff;
    coff.numberOfSections = sectionNumber;
    coff.timeDateStamp = time(nullptr);

    outFile.write((char*)&coff, sizeof(COFF_Header));
}