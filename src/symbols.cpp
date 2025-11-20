#include "main.h"
#include "symbols.h"


inline static std::optional<symbol_class> getSymbolClass(const char* str) {
    while (*str == ' ' || *str == '\t')
        str++;

    if (strncmp(str, "extern", 6) == 0)
        return symbol_class::IMAGE_SYM_CLASS_EXTERNAL;
    if (strncmp(str, "static", 6) == 0)
        return symbol_class::IMAGE_SYM_CLASS_STATIC;

    return std::nullopt;
}





inline static ushort CountSymbols(std::ifstream& srcFile) {
    char inputLine[maxLineSize] = "";

    ushort symbolCounter = 0;
    while (true) {
        srcFile.getline(inputLine, maxLineSize);
        if (srcFile.eof())
            break;

        if (srcFile.fail() && inputLine[maxLineSize - 2] != NULL) {
            *strchr(inputLine, ';') = NULL;
            srcFile.clear();
        }
            

        if (getSymbolClass(inputLine).has_value())
            symbolCounter++;
    }

    return symbolCounter;
}





inline void WriteSymbolTable(std::ofstream& outFile, std::ifstream& srcFile, fs::path sourcePath, IMAGE_SECTION_HEADER(&sections)[]) {
    //symbol amount refers to amount of symbols AND their aux definition
    const ubyte baseSymbolAmount = 8;
    const ushort symbolAmount = CountSymbols(srcFile) + baseSymbolAmount;
    const fpos_t strtabPos = outFile.tellp() + (fpos_t)COFF_Symbol_Size * (fpos_t)symbolAmount;

    const ubyte spaceForStrtabSizeVar = 4;
    fpos_t strtabEndPos = strtabPos + spaceForStrtabSizeVar;


    COFF_Symbol file;
    strcpy(file.name.shortName, ".file");
    file.value = 0;
    file.sectionNumber = -2;
    file.type = symbol_type::IMAGE_SYM_TYPE_NULL;
    file.storageClass = symbol_class::IMAGE_SYM_CLASS_FILE;
    file.numberOfAuxSymbols = 1;

    outFile.write((char*)&file, COFF_Symbol_Size);


    const ubyte filenameLength = sourcePath.filename().string().length() + 1;

    if (filenameLength > COFF_Symbol_Size) {
        outFile.seekp(outFile.tellp() + (fpos_t)4);
        outFile.write((char*)&strtabEndPos, 4);

        fpos_t nextSymPos = outFile.tellp() + (fpos_t)10;
        outFile.seekp(strtabEndPos);
        outFile.write((char*)sourcePath.filename().u8string().c_str(), filenameLength);

        outFile.seekp(nextSymPos);
        strtabEndPos += filenameLength;
    }
    else {
        outFile.write((char*)sourcePath.filename().u8string().c_str(), filenameLength);
        outFile.seekp(outFile.tellp() + (fpos_t)COFF_Symbol_Size - (fpos_t)filenameLength);
    }



    for (ubyte sectionI = 0; sectionI < sectionNumber; sectionI++) {
        COFF_Symbol section;
        strcpy(section.name.shortName, sectionNames[sectionI]);
        section.value = 0;
        section.sectionNumber = sectionI + 1;
        section.type = symbol_type::IMAGE_SYM_TYPE_NULL;
        section.storageClass = symbol_class::IMAGE_SYM_CLASS_STATIC;
        section.numberOfAuxSymbols = 1;
        outFile.write((char*)&section, COFF_Symbol_Size);

        AuxiliarySectionDefinition aux;
        aux.Length = sections[sectionI].mSizeOfRawData;
        outFile.write((char*)&aux, COFF_Symbol_Size);
    }


    const ulong strtabSize = strtabEndPos - strtabPos;
    outFile.write((char*)&strtabSize, 4);
}