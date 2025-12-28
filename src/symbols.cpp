#include <regex>

#include "main.h"
#include "symbols.h"


static bool isValidSymbol(const char* str, Section section) {
    const char regexes[][39] = {
    "[a-zA-Z]{6} [a-zA-Z]\\w*:?\\s*",
    "(global )?[a-zA-Z]+ [a-zA-Z]+ = \\d+\\s*",
    "(extern )?[a-zA-Z]+ [a-zA-Z]+\\s*" };

    return std::regex_match(str, std::regex(regexes[section - 1]));
}





enum class Scope {
    UNDEF,
    STATIC,
    GLOBAL,
    EXTERN
};



static Scope getScope(const char* str) {
    const char scopes[][7] = { "static", "global", "extern" };
    for (ubyte scopeI = 0; scopeI < std::size(scopes); scopeI++) {
        if (strncmp(str, scopes[scopeI], 6) == 0)
            return (Scope)(scopeI + 1);
    }

    return Scope::UNDEF;
}





inline SymbolScopeCount CountSymbols() {
    SymbolScopeCount symbolCount{};

    ErrorData errorData;

    Section currentSection = NOSECTION;
    char inputLine[maxLineSize] = "";

    while (!srcFile.eof()) {
        errorData.incLine();

        const fpos_t startOfLine = srcFile.tellg();
        srcFile.getline(inputLine, FPOSMAX);

        if (srcFile.fail())
            ProcessInputError(inputLine, startOfLine, errorData);

        if (strncmp(inputLine, "section", 7) == 0) {
            ubyte sectionI = 1;
            for (sectionI; strcmp(inputLine + 8, sectionNames[sectionI]) != 0; sectionI++) {
                if (sectionI >= sectionCount)
                    Error(errorData, "Section name does not exist");
            }

            currentSection = (Section)sectionI;
            continue;
        }


        const char* formattedLine = inputLine;
        while (*formattedLine == ' ' || *formattedLine == '\t')
            formattedLine++;
        if (char* comment = strchr(inputLine, ';'))
            *comment = NULL;
        if (*formattedLine == NULL)
            continue;

        if (!isValidSymbol(formattedLine, currentSection)) {
            if (currentSection == TEXT)
                continue;
            else
                Error(errorData, "Invalid symbol");
        }

        Scope scope = getScope(formattedLine);

        if (scope == Scope::UNDEF) {
            symbolCount.staticSymCount++;
        }
        else {
            symbolCount.staticSymCount += (scope == Scope::STATIC);
            symbolCount.globalSymCount += (scope == Scope::GLOBAL);
            symbolCount.externSymCount += (scope == Scope::EXTERN);
            symbolCount.auxiliary += (strrchr(formattedLine, ':') != nullptr);
        }
    }

    return symbolCount;
}





static ubyte GetSymbolSize(const char* str, ErrorData errorData) {
    const ubyte strSize = 6;
    const char fundamentalTypes[][strSize] = { "byte", "word", "dword", "qword" };

    std::optional<ubyte> typeI = findStringInArray(str, *fundamentalTypes, std::size(fundamentalTypes), strSize);
    if (!typeI.has_value())
        Error(errorData, "Invalid variable type");

    return pow(2, *typeI);
}




inline char* FindSymbols(const SymbolScopeCount& symbolCount) {
    char* symtab = (char*)calloc((size_t)symbolCount.sumWithAux(), sizeof(SymbolData));
    const char* const symtabEnd = symtab + symbolCount.sumWithAux() * sizeof(SymbolData);
    SymbolData* symbol = (SymbolData*)symtab;


    ErrorData errorData;

    srcFile.seekg(0);
    Section currentSection = NOSECTION;
    char inputLine[maxLineSize] = "";

    while (!srcFile.eof()) {
        errorData.incLine();

        const fpos_t startOfLine = srcFile.tellg();
        srcFile.getline(inputLine, FPOSMAX);

        if (srcFile.fail())
            ProcessInputError(inputLine, startOfLine, errorData);

        if (strncmp(inputLine, "section", 7) == 0) {
            ubyte sectionI = 1;
            for (sectionI; strcmp(inputLine + 8, sectionNames[sectionI]) != 0; sectionI++);
            currentSection = (Section)sectionI;
            continue;
        }

        char* formattedLine = inputLine;
        while (*formattedLine == ' ' || *formattedLine == '\t')
            formattedLine++;
        if (char* comment = strchr(inputLine, ';'))
            *comment = NULL;
        if (*formattedLine == NULL)
            continue;
        if (currentSection == NOSECTION)
            Error(errorData, "Characters found outside of any section");

        if (!isValidSymbol(formattedLine, currentSection))
            continue;

        const char* firstToken = strtok(formattedLine, " ");
        Scope scope = getScope(firstToken);

        symbol->sectionNumber = (int16_t)currentSection;

        if (scope == Scope::UNDEF) {
            symbol->storageClass = symbol_class::IMAGE_SYM_CLASS_STATIC;
            symbol->size = GetSymbolSize(firstToken, errorData);
        }
        else {
            if (scope == Scope::STATIC)
                symbol->storageClass = symbol_class::IMAGE_SYM_CLASS_STATIC;
            else
                symbol->storageClass = symbol_class::IMAGE_SYM_CLASS_EXTERNAL;

            if (currentSection != TEXT)
                symbol->size = GetSymbolSize(strtok(nullptr, " "), errorData);
            else if (scope == Scope::EXTERN)
                symbol->sectionNumber = 0;
        }

        const char* name = strtok(nullptr, " :");
        symbol->nameRef = startOfLine + (name - formattedLine) + 1;
        symbol->nameLen = strlen(name) + terminatingNull;


        symbol += hasAux(symbol);
        symbol++;
        if ((char*)symbol > symtabEnd)
            CompilerError(errorData, "Symbols don't fit into symtab. Symbols were miscounted");
    }

    return symtab;
}





union SymPointer {
    char* chr;
    SymbolData* data;
    COFF_Symbol* coff;
};

inline static void ConvertSymbols(char* symtab, const SymbolScopeCount symbolCount, const fpos_t strtabPos) {
    const char* const symtabEnd = symtab + symbolCount.sumWithAux() * sizeof(COFF_Symbol);
    SymPointer sym;
    sym.chr = symtab;

    char name[maxLineSize] = "";

    while (sym.chr < symtabEnd) {
        srcFile.clear();
        srcFile.seekg(sym.data->nameRef);
        srcFile.getline(name, sym.data->nameLen);

        if (srcFile.bad())
            ProcessInputError(name, sym.data->nameRef, {});

        if (sym.data->nameLen > 8) {
            memset(sym.coff->name.shortName, 0, 8);
            sym.coff->name.longName.offset = (fpos_t)outFile.tellp() - strtabPos;
            outFile.write(name, sym.data->nameLen + terminatingNull);
        }
        else {
            strncpy(sym.coff->name.shortName, name, 8);
        }

        sym.coff->numberOfAuxSymbols = 0;

        switch (sym.data->size) {
        case 0:
            sym.coff->type = symbol_type::MS_IMAGE_SYM_TYPE_FUNCTION;

            if (sym.data->sectionNumber) {
                sym.coff->numberOfAuxSymbols = 1;
                sym.coff++;
                //TODO: Write AuxiliaryFunctionDefinition
                *sym.coff = {};
            }
            break;
        case 1:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_BYTE;
            break;
        case 2:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_WORD;
            break;
        case 4:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_DWORD;
            break;
        default:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_NULL;
        }

        sym.coff++;
    }
}




inline void WriteSymbolTable(const fs::path srcPath, IMAGE_SECTION_HEADER(&sections)[], char* symtab, const SymbolScopeCount& symbolCount) {
    const ushort baseSymbols = 2 * sectionCount + 2;
    const ushort strtabSizeVar = sizeof(uint32_t);
    const fpos_t strtabFirst = (fpos_t)outFile.tellp() + sizeof(COFF_Symbol) * (symbolCount.sumWithAux() + baseSymbols) + strtabSizeVar;


    COFF_Symbol file;
    strcpy(file.name.shortName, ".file");
    file.value = 0;
    file.sectionNumber = -2;
    file.type = symbol_type::IMAGE_SYM_TYPE_NULL;
    file.storageClass = symbol_class::IMAGE_SYM_CLASS_FILE;
    file.numberOfAuxSymbols = 1;

    outFile.write((char*)&file, sizeof(COFF_Symbol));


    const fpos_t nextSymbol = (fpos_t)outFile.tellp() + sizeof(COFF_Symbol);
    const ubyte filenameLength = srcPath.filename().string().length() + terminatingNull;

    if (filenameLength > sizeof(COFF_Symbol)) {
        outFile.write((char*)&strtabFirst, 8);
        outFile.seekp(strtabFirst);
        outFile.write((char*)srcPath.filename().u8string().c_str(), filenameLength);
    }
    else {
        outFile.write((char*)srcPath.filename().u8string().c_str(), filenameLength);
        outFile.seekp(strtabFirst);
    }


    ConvertSymbols(symtab, symbolCount, strtabFirst - strtabSizeVar);
    const ulong strtabSize = outFile.tellp() - strtabFirst + (fpos_t)strtabSizeVar;
    outFile.seekp(nextSymbol);
    outFile.write(symtab, symbolCount.sumWithAux() * sizeof(COFF_Symbol));


    for (ubyte sectionI = 1; sectionI < sectionCount + 1; sectionI++) {
        COFF_Symbol section;
        strcpy(section.name.shortName, sectionNames[sectionI]);
        section.value = 0;
        section.sectionNumber = sectionI;
        section.type = symbol_type::IMAGE_SYM_TYPE_NULL;
        section.storageClass = symbol_class::IMAGE_SYM_CLASS_STATIC;
        section.numberOfAuxSymbols = 1;
        outFile.write((char*)&section, sizeof(COFF_Symbol));

        AuxiliarySectionDefinition aux = {};
        aux.Length = sections[sectionI].mSizeOfRawData;
        outFile.write((char*)&aux, sizeof(COFF_Symbol));
    }

    outFile.write((char*)&strtabSize, 4);
}