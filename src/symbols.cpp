#include "main.h"
#include "symbols.h"


inline static std::optional<symbol_class> getSymbolClass(const char* str) {
    if (strncmp(str, "extern", 6) == 0)
        return symbol_class::IMAGE_SYM_CLASS_EXTERNAL;
    if (strncmp(str, "static", 6) == 0)
        return symbol_class::IMAGE_SYM_CLASS_STATIC;

    return std::nullopt;
}





inline static ushort CountSymbols(std::ifstream& sourceFile) {
    sourceFile.seekg(0);

    std::string inputLine;
    inputLine.reserve(50);

    ushort symbolCounter = 0;
    while (!sourceFile.eof()) {
        getline(sourceFile, inputLine);

        if (getSymbolClass(inputLine.c_str()).has_value())
            symbolCounter++;
    }

    return symbolCounter;
}





inline void WriteSymbolTable(std::ofstream& outFile, std::ifstream& sourceFile, fs::path sourcePath, IMAGE_SECTION_HEADER(&sections)[]) {
    //symbol amount refers to the symtab size divided by 18, which includes aux definitions
    const ubyte baseSymbolAmount = 8;
    const ushort symbolAmount = CountSymbols(sourceFile) + baseSymbolAmount;
    const ubyte spaceForStrtabSizeVar = 4;
    fpos_t strtabEndPos = outFile.tellp() + (fpos_t)COFF_Symbol_Size * (fpos_t)symbolAmount + (fpos_t)spaceForStrtabSizeVar;


    COFF_Symbol file;
    strcpy(file.name.shortName, ".file");
    file.value = 0;
    file.sectionNumber = -2;
    file.type = symbol_type::IMAGE_SYM_TYPE_NULL;
    file.storageClass = symbol_class::IMAGE_SYM_CLASS_FILE;
    file.numberOfAuxSymbols = 1;

    outFile.write((char*)&file, COFF_Symbol_Size);


    const ubyte filenameLength = sourcePath.filename().string().length();

    if (filenameLength > COFF_Symbol_Size) {
        outFile.seekp(outFile.tellp() + (fpos_t)4);
        outFile.write((char*)&strtabEndPos, 4);

        fpos_t nextSymPos = outFile.tellp() + (fpos_t)10;
        outFile.seekp(strtabEndPos);
        outFile.write((char*)sourcePath.filename().u8string().c_str(), filenameLength + 1);

        outFile.seekp(nextSymPos);
        strtabEndPos += filenameLength + 1;
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


    const ulong strtabSize = strtabEndPos - outFile.tellp();
    outFile.write((char*)&strtabSize, 4);
}