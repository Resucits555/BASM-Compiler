#include <regex>

#include "main.h"
#include "symbols.h"


static bool isValidSymbol(const char* str, Section section) {
    const char regexes[][45] = {
    "[a-zA-Z]{6} [a-zA-Z_]\\w*:?\\s*",
    "",
    "",
    "(extern )?[a-zA-Z]+ [a-zA-Z_]\\w*\\s*",
    "(global )?[a-zA-Z]+ [a-zA-Z_]\\w* = \"?.*\"?\\s*",
    "(global )?[a-zA-Z]+ [a-zA-Z_]\\w* = \"?.*\"?\\s*" };

    return std::regex_match(str, std::regex(regexes[section - 1]));
}





SizeType getSymbolBytes(const char* str, ErrorData errorData) {
    const ubyte strSize = 6;
    const char fundamentalTypes[][strSize] = { "byte", "word", "dword", "qword" };

    std::optional<ubyte> typeI = findStringInArray(str, *fundamentalTypes, std::size(fundamentalTypes), strSize);
    if (!typeI.has_value())
        Error(errorData, "Invalid variable type. Note that memory access needs a type");

    return (SizeType)pow(2, *typeI);
}





enum class Scope {
    UNDEF,
    STATIC,
    GLOBAL,
    EXTERN
};



static Scope getScope(const char* str) {
    if (str[6] != NULL && str[6] != ' ')
        return Scope::UNDEF;
    const char scopes[][7] = { "static", "global", "extern" };
    for (ubyte scopeI = 0; scopeI < std::size(scopes); scopeI++) {
        if (strncmp(str, scopes[scopeI], 6) == 0)
            return (Scope)(scopeI + 1);
    }

    return Scope::UNDEF;
}





inline SymbolScopeCount CountSymbols(SectionHeader* sections) {
    SymbolScopeCount symbolCount{};

    ErrorData errorData;

    Section currentSection = NOSECTION;
    char inputLine[maxLineSize] = "";

    do {
        errorData.incLine();

        srcFile.getline(inputLine, FPOSMAX);

        if (srcFile.fail())
            ProcessInputError(inputLine, errorData);

        if (strncmp(inputLine, "section", 7) == 0) {
            ubyte sectionI = 1;
            for (sectionI; strcmp(inputLine + 8, sectionNames[sectionI]) != 0; sectionI++) {
                if (sectionI >= sectionCount)
                    Error(errorData, "Section name does not exist");
            }

            currentSection = (Section)sectionI;
            symbolCount.sectionSymCount++;
            sections[currentSection].sectionIndex = symbolCount.sectionSymCount;
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
            if (std::regex_match(formattedLine, std::regex("\\s*\\.*[a-zA-Z_]\\w*:\\s*")))
                symbolCount.labelCount++;
            else if (currentSection != TEXT || strchr(formattedLine, ':'))
                Error(errorData, "Invalid symbol");
            continue;
        }

        Scope scope = getScope(formattedLine);

        if (scope == Scope::UNDEF) {
            symbolCount.staticSymCount++;
        }
        else {
            symbolCount.staticSymCount += (scope == Scope::STATIC);
            symbolCount.globalSymCount += (scope == Scope::GLOBAL);
            symbolCount.externSymCount += (scope == Scope::EXTERN);
            symbolCount.auxiliary += std::regex_match(formattedLine, std::regex("[a-zA-Z]{6} [a-zA-Z_]\\w*:\\s*"));
        }
    } while (!srcFile.eof());

    return symbolCount;
}





inline static ulong InitializeSymbol(SymbolData* symbol, const char* nameEnd, ErrorData errorData) {
    ulong dataSize = 0;
    const char* init = nameEnd + 2;

    do {
        if (*init == '"') {
            const char* stringEnd = strrchr(init, '"') - 1;
            outFile.write(init + 1, stringEnd - init);
            dataSize += stringEnd - init;

            init = stringEnd + 4;
        }
        else {
            long long varVal;
            size_t len;
            try { varVal = std::stoll(init, &len, 0); }
            catch (std::exception e) {
                Error(errorData, e.what());
            }
            outFile.write((char*)&varVal, (int)symbol->size);
            dataSize += (ulong)symbol->size;

            init += len + 2;
        }
    } while (*(init - 2));

    return dataSize;
}




inline SymbolData* FindSymbols(const SymbolScopeCount& symbolCount, SectionHeader* sections) {
    SymbolData* symtab = (SymbolData*)calloc((size_t)symbolCount.sum(), sizeof(SymbolData));
    if (symtab == nullptr)
        Error("\"calloc\" function failed");
    const SymbolData* const symtabEnd = symbolCount.getReducedSymtabEnd(symtab);
    SymbolData* symbol = (SymbolData*)symtab;


    ErrorData errorData;

    srcFile.seekg(0);
    ubyte currentSection = 0;
    AuxiliaryFunctionDefinition* prevAux = nullptr;
    char inputLine[maxLineSize] = "";

    const char* name;
    do {
        errorData.incLine();

        srcFile.getline(inputLine, FPOSMAX);

        if (srcFile.fail())
            ProcessInputError(inputLine, errorData);

        if (strncmp(inputLine, "section", 7) == 0) {
            ubyte sectionI = 0;
            for (;strcmp(inputLine + 8, sectionNames[sectionI + 1]) != 0; sectionI++) {
                if (sectionI > std::size(sectionNames))
                    Error(errorData, "Unknown section name");
            }
            currentSection = sectionI + 1;
            sections[currentSection].pointerToRawData = outFile.tellp();
            continue;
        }

        char* formattedLine = inputLine;
        while (*formattedLine == ' ' || *formattedLine == '\t')
            formattedLine++;
        if (char* comment = strchr(inputLine, ';'))
            *comment = NULL;
        if (*formattedLine == NULL)
            continue;
        if (currentSection == 0)
            Error(errorData, "Characters found outside of any section");

        bool isLabel = false;
        if (!isValidSymbol(formattedLine, (Section)currentSection)) {
            if (std::regex_match(formattedLine, std::regex("\\s*\\.*[a-zA-Z_]\\w*:\\s*")))
                isLabel = true;
            else continue;
        }

        const char* firstToken = strtok(formattedLine, " :");
        Scope scope = getScope(firstToken);

        symbol->sectionNumber = sections[currentSection].sectionIndex;

        if (scope == Scope::UNDEF) {
            symbol->storageClass = symbol_class::IMAGE_SYM_CLASS_STATIC;
            if (!isLabel) {
                symbol->size = getSymbolBytes(firstToken, errorData);
            }
            else {
                //TODO: Add parent names
                symbol->size = SizeType::LABEL;
                symbol->nameRef = (srcFile.tellg() - srcFile.gcount()) + (firstToken - formattedLine);
                symbol->nameLen = strlen(firstToken) + terminatingNull;
                goto nextSymbol;
            }
        }
        else {
            if (scope == Scope::STATIC)
                symbol->storageClass = symbol_class::IMAGE_SYM_CLASS_STATIC;
            else
                symbol->storageClass = symbol_class::IMAGE_SYM_CLASS_EXTERNAL;

            if (currentSection != TEXT)
                symbol->size = getSymbolBytes(strtok(nullptr, " :"), errorData);
            else
                symbol->size = SizeType::FUNCTION;
            if (scope == Scope::EXTERN)
                symbol->sectionNumber = 0;
        }


        name = strtok(nullptr, " :");
        symbol->nameRef = (srcFile.tellg() - srcFile.gcount()) + (name - formattedLine);
        symbol->nameLen = strlen(name) + terminatingNull;


        if (currentSection >= BSS) {
            ulong dataSize = (ulong)symbol->size;
            if (currentSection > BSS)
                dataSize = InitializeSymbol(symbol, name + symbol->nameLen, errorData);

            symbol->value = sections[currentSection].sizeOfRawData;
            sections[currentSection].sizeOfRawData += dataSize;
        }


        if (symbol->isDefinedFunction()) {
            if (prevAux != nullptr) {
                const ushort fileSymbol = 2;
                prevAux->PointerToNextFunction = symbol - symtab + fileSymbol;
            }
            prevAux = (AuxiliaryFunctionDefinition*)symbol + 1;
            symbol++;
        }

        nextSymbol:
        symbol++;
        if (symbol > symtabEnd)
            CompilerError(errorData, "Symbols don't fit into symtab");
    } while (!srcFile.eof());

    return symtab;
}





union SymPointer {
    char* chr;
    SymbolData* data;
    COFF_Symbol* coff;
};

inline static void ConvertSymbols(SymbolData* const symtab, const SymbolScopeCount symbolCount, const fpos_t strtabPos, SectionHeader* sections) {
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

        switch (sym.data->size) {
        case SizeType::NONE:
            CompilerError("Symbol type not set");
        case SizeType::BYTE:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_BYTE;
            break;
        case SizeType::WORD:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_WORD;
            break;
        case SizeType::FUNCTION:
            sym.coff->type = symbol_type::MS_IMAGE_SYM_TYPE_FUNCTION;

            if (sym.data->sectionNumber) {
                sym.coff->numberOfAuxSymbols = 1;
                sym.data++;
            }
            break;
        case SizeType::DWORD:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_DWORD;
            break;
        default:
            sym.coff->type = symbol_type::IMAGE_SYM_TYPE_NULL;
        }
    }
}




inline void WriteSymbolTable(const fs::path srcPath, SectionHeader* sections, SymbolData* const symtab, const SymbolScopeCount& symbolCount) {
    const ushort strtabSizeVar = sizeof(uint32_t);
    const fpos_t strtabFirst = (fpos_t)outFile.tellp() + sizeof(COFF_Symbol) * (symbolCount.sum() + requiredSymbolSpace) + strtabSizeVar;


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


    for (ubyte sectionI = 1; sectionI < std::size(sectionNames); sectionI++) {
        if (sections[sectionI].sizeOfRawData == 0)
            continue;
        COFF_Symbol sectionSymbol;
        strcpy(sectionSymbol.name.shortName, sectionNames[sectionI]);
        sectionSymbol.value = 0;
        sectionSymbol.sectionNumber = sections[sectionI].sectionIndex;
        sectionSymbol.type = symbol_type::IMAGE_SYM_TYPE_NULL;
        sectionSymbol.storageClass = symbol_class::IMAGE_SYM_CLASS_STATIC;
        sectionSymbol.numberOfAuxSymbols = 1;
        outFile.write((char*)&sectionSymbol, sizeof(COFF_Symbol));

        AuxiliarySectionDefinition aux = {};
        aux.Length = sections[sectionI].sizeOfRawData;
        outFile.write((char*)&aux, sizeof(COFF_Symbol));
    }

    outFile.write((char*)&strtabSize, 4);
}