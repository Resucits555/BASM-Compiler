#include <regex>

#include "main.h"
#include "symbols.h"


static bool isValidSymbol(const char* str, Section section) {
    const char regexes[][39] = {
    "[a-zA-Z]{6} [a-zA-Z]\\w*:?\\s*",
    "(extern )?[a-zA-Z]+ [a-zA-Z]+\\s*",
    "(global )?[a-zA-Z]+ [a-zA-Z]+ = \\d+\\s*" };

    return std::regex_match(str, std::regex(regexes[std::clamp(section - 1, 0, 2)]));
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
            symbolCount.sectionSymCount++;
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




inline SymbolData* FindSymbols(const SymbolScopeCount& symbolCount, SectionTab& sections) {
    SymbolData* symtab = (SymbolData*)calloc((size_t)symbolCount.sum(), sizeof(SymbolData));
    if (symtab == nullptr)
        Error("\"calloc\" function failed");
    const SymbolData* const symtabEnd = symbolCount.getReducedSymtabEnd(symtab);
    SymbolData* symbol = (SymbolData*)symtab;


    ErrorData errorData;

    srcFile.seekg(0);
    Section currentSection = NOSECTION;
    ubyte sectionIndex = 2;
    AuxiliaryFunctionDefinition* prevAux = nullptr;
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

            if (sectionI != TEXT)
                sections.headers[sectionIndex++].section = (Section)sectionI;
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

        symbol->sectionNumber = currentSection;

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
                symbol->size = GetSymbolSize(strtok(nullptr, " :"), errorData);
            if (scope == Scope::EXTERN)
                symbol->sectionNumber = 0;
        }


        const char* name = strtok(nullptr, " :");
        symbol->nameRef = startOfLine + (name - formattedLine) + 1;
        symbol->nameLen = strlen(name) + terminatingNull;


        if (currentSection >= BSS) {
            symbol->value = sections.header(currentSection)->mSizeOfRawData;
            sections.header(currentSection)->mSizeOfRawData += symbol->size;
        }


        if (symbol->hasFunctionAux()) {
            if (prevAux != nullptr) {
                const ushort fileSymbol = 2;
                prevAux->PointerToNextFunction = symbol - symtab + fileSymbol;
            }
            prevAux = (AuxiliaryFunctionDefinition*)symbol + 1;
            symbol++;
            *prevAux = {};
        }

        symbol++;
        if (symbol > symtabEnd)
            CompilerError(errorData, "Symbols don't fit into symtab. Symbols were miscounted");
    }

    return symtab;
}





inline void WriteData(SymbolData* const symtab, const SymbolData* symtabEnd, SectionTab& sections) {
    char inputLine[maxLineSize];
    Section prevSection = NOSECTION;
    for (SymbolData* sym = symtab; sym < symtabEnd; sym++) {
        if (sym->sectionNumber <= BSS)
            continue;

        srcFile.seekg(sym->nameRef + sym->nameLen);
        srcFile.getline(inputLine, FPOSMAX);

        if (srcFile.bad())
            ProcessInputError(inputLine, sym->nameRef, {});
        if (*inputLine != '=')
            Error(("Variable in section number " + sym->sectionNumber + (std::string)" not initialised").c_str());

        if (prevSection != sym->sectionNumber) {
            sections.header((Section)sym->sectionNumber)->mPointerToRawData = outFile.tellp();
            prevSection = (Section)sym->sectionNumber;
        }

        long long value = std::stoll(inputLine + 2, nullptr, 0);
        outFile.write((char*)&value, sym->size);
    }
}





union SymPointer {
    char* chr;
    SymbolData* data;
    COFF_Symbol* coff;
};

inline static void ConvertSymbols(SymbolData* const symtab, const SymbolScopeCount symbolCount, const fpos_t strtabPos, SectionTab& sections) {
    const SymbolData* const symtabEnd = symbolCount.getReducedSymtabEnd(symtab);

    char name[maxLineSize] = "";

    SymPointer sym;
    for (sym.data = symtab; sym.data < symtabEnd; sym.data++) {
        sym.data->getName(name);

        if (sym.data->nameLen > 8) {
            memset(sym.coff->name.shortName, 0, 8);
            sym.coff->name.longName.offset = (fpos_t)outFile.tellp() - strtabPos;
            outFile.write(name, sym.data->nameLen + terminatingNull);
        }
        else {
            strncpy(sym.coff->name.shortName, name, 8);
        }

        sym.coff->numberOfAuxSymbols = 0;
        if (sym.data->sectionNumber > 0)
            sym.coff->sectionNumber = (sections.header((Section)sym.data->sectionNumber) - sections.headers);

        switch (sym.data->size) {
        case 0:
            sym.coff->type = symbol_type::MS_IMAGE_SYM_TYPE_FUNCTION;

            if (sym.data->sectionNumber) {
                sym.coff->numberOfAuxSymbols = 1;
                sym.data++;
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
    }
}




inline void WriteSymbolTable(const fs::path srcPath, SectionTab& sections, SymbolData* const symtab, const SymbolScopeCount& symbolCount) {
    const ushort strtabSizeVar = sizeof(uint32_t);
    const fpos_t strtabFirst = (fpos_t)outFile.tellp() + sizeof(COFF_Symbol) * (symbolCount.sum() + fileSymbol) + strtabSizeVar;


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


    ConvertSymbols(symtab, symbolCount, strtabFirst - strtabSizeVar, sections);
    const ulong strtabSize = outFile.tellp() - strtabFirst + (fpos_t)strtabSizeVar;
    outFile.seekp(nextSymbol);
    outFile.write((char*)symtab, (symbolCount.sum() - 2 * symbolCount.sectionSymCount) * sizeof(COFF_Symbol));


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
        aux.Length = sections.header((Section)sectionI)->mSizeOfRawData;
        outFile.write((char*)&aux, sizeof(COFF_Symbol));
    }

    outFile.write((char*)&strtabSize, 4);
}