#include "main.h"


std::ofstream outFile;
std::ifstream srcFile;
static fs::path outPath;
const char* srcPathRaw;



void Warning(const ErrorData errorData, const char* message) {
    std::cout << "Warning: " << srcPathRaw << " at line " << errorData.getLine() << ": " << message << ".\n\n";
}


NORETURN static void ErrorCleanup() {
    if (outFile.is_open())
        outFile.close();
    fs::remove(outPath);
    exit(-1);
}


NORETURN void Error(const char* message) {
    if (srcPathRaw == nullptr)
        std::cerr << "ERROR: " << message << ".\n\n";
    else
        std::cerr << "ERROR: " << srcPathRaw << ": " << message << ".\n\n";
    ErrorCleanup();
}


NORETURN void Error(const char* path, const char* message) {
    std::cerr << "ERROR: " << path << ": " << message << ".\n\n";
    ErrorCleanup();
}


NORETURN void Error(const ErrorData errorData, const char* message) {
    std::cerr << "ERROR: " << srcPathRaw << " at line " << errorData.getLine() << ": " << message << ".\n\n";
    ErrorCleanup();
}


NORETURN void CompilerError(const char* message) {
    std::cerr << "Compiler error: " << srcPathRaw << ": "
        << message << ". Please report the bug to the creators of this compiler.\n\n";
    ErrorCleanup();
}


NORETURN void CompilerError(const ErrorData errorData, const char* message) {
    std::cerr << "Compiler error: " << srcPathRaw << " at line " << errorData.getLine() << ": "
        << message << ". Please report the bug to the creators of this compiler.\n\n";
    ErrorCleanup();
}





void ProcessInputError(char* inputLine, const ErrorData& errorData) {
    if (srcFile.bad()) {
        std::string errIntro = "ERROR: Failed to read from file " + (std::string)srcPathRaw
            + " at line " + std::to_string(errorData.getLine());
        perror(errIntro.c_str());
        exit(-1);
    }

    //In this case the line is probably too long to fit
    if (inputLine[maxLineSize - 1] != NULL) {
        srcFile.clear();
        srcFile.seekg(srcFile.tellg() - srcFile.gcount());
        srcFile.getline(inputLine, FPOSMAX, ';');
        if (srcFile.fail()) {
            Error(errorData, ("Exceeding line length limit of " + std::to_string(maxLineSize)
                + " characters. Comments not included").c_str());
        }
        else {
            srcFile.ignore(FPOSMAX, '\n');
            return;
        }
    }

    Error(errorData, "Failed to read file. Exact error is unknown");
}





std::optional<ubyte> findStringInArray(const char* string, const char* array, const ushort arrayElementCount, const ubyte arrayStrLen) {
    for (ubyte stringI = 0; stringI < arrayElementCount; stringI++) {
        if (strcmp(string, array + (stringI * arrayStrLen)) == NULL)
            return stringI;
    }
    return std::nullopt;
}





SymbolData* findSymbol(const char* name, SymbolData* symtab, const SymbolData* symtabEnd, ErrorData errorData) {
    char symName[maxLineSize] = "";

    for (SymbolData* symbol = symtab; symbol < symtabEnd; symbol++) {
        std::ios::iostate state = srcFile.rdstate();
        fpos_t ret = srcFile.tellg();
        symbol->getName(symName);
        srcFile.clear(state);
        if (state != 1)
            srcFile.seekg(ret);
        if (strcmp(name, symName) == 0)
            return symbol;
        if (symbol->isDefinedFunction())
            symbol++;
    }

    Error(errorData, "Symbol could not be found");
}





inline static ubyte GetCommand(const ubyte argc, char** argv) {
    if (argc < 2)
        Error("Command missing. Type \"basm help\" to learn how to use this program");

    if (strlen(argv[1]) >= 8)
        Error("Unknown command. Type \"basm help\" to learn how to use this program");

    char command[8];
    strcpy(command, argv[1]);

    constexpr ubyte strSize = 8;
    constexpr char commands[][strSize] = { "help", "-help", "--help", "compile" };

    const std::optional<ubyte> commandI = findStringInArray(command, *commands, std::size(commands), strSize);
    if (!commandI.has_value())
        Error("Unknown command. Type \"basm help\" to learn how to use this program");

    return *commandI;
}





int main(const ubyte argc, char* argv[]) {
    ubyte commandI = GetCommand(argc, argv);

    switch (commandI) {
    case 0:
    case 1:
    case 2:
        std::cout << "Usage:\nbasm <command> [<options>] [<file> <file>...]\n\nCommands: \n1. \"help\" Displays long help message\n2. \"compile\" Compile all specified files\n\nLearn more in the README file.\n\n";
        exit(0);
    case 3:
        for (ubyte fileI = 2; fileI < argc && argv[fileI][0] != '-'; fileI++) {
            outPath = fs::path(argv[fileI]);
            fs::path srcPath = outPath;
            outPath.replace_extension("o");

            const std::string srcPathString = srcPath.string();
            srcPathRaw = srcPathString.c_str();

            outFile.open(outPath, std::ios::binary);
            if (!outFile.is_open())
                Error((char*)outPath.c_str(), "Failed to create output file");
            outFile.seekp(0);

            srcFile.open(srcPath);
            if (!srcFile.is_open() || srcFile.fail()) {
                char errIntro[] = "ERROR: Failed to open file";
                perror(strcat(errIntro, (char*)srcPath.c_str()));
                exit(-1);
            }

            SectionHeader sections[std::size(sectionNames)] = {};

            SymbolScopeCount symbolCount = CountSymbols(sections);

            const ushort coffHeaderSize = 20;
            const ushort sectionHeaderSize = 40;
            outFile.seekp(coffHeaderSize + (symbolCount.sectionSymCount * sectionHeaderSize));

            SymbolData* const symtab = FindSymbols(symbolCount, sections);
            SymbolData* symtabEnd = symbolCount.getReducedSymtabEnd(symtab);

            CompileSource(sections, (SymbolData*)symtab, symtabEnd);

            const fpos_t symtabPos = outFile.tellp();
            WriteSymbolTable(srcPath, sections, symtab, symbolCount);
            srcFile.close();
            free(symtab);

            WriteCOFFHeader(symtabPos, symbolCount);
            WriteSectionTable(sections);
            outFile.close();
        }
    }

    std::cout << "\nSuccessfully compiled!\n\n";
}