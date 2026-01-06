#include "main.h"


void Warning(const ErrorData errorData, const char* message) {
    std::cout << "Warning: " << srcPathStr << " at line " << errorData.getLine() << ": " << message << ".\n\n";
}


[[noreturn]] void Error(const char* message) {
    std::cerr << "ERROR: " << srcPathStr << ": " << message << ".\n\n";
    exit(-1);
}


[[noreturn]] void Error(const char* path, const char* message) {
    std::cerr << "ERROR: " << path << ": " << message << ".\n\n";
    exit(-1);
}


[[noreturn]] void Error(const ErrorData errorData, const char* message) {
    if (errorData.getLine() == 0)
        Error(message);
    std::cerr << "ERROR: " << srcPathStr << " at line " << errorData.getLine() << ": " << message << ".\n\n";
    exit(-1);
}


[[noreturn]] void CompilerError(const ErrorData errorData, const char* message) {
    std::cerr << "Compiler error: " << srcPathStr << " at line " << errorData.getLine() << ": "
        << message << ". Please report the bug to the creators of this compiler.\n\n";
    exit(-1);
}





void ProcessInputError(char* inputLine, const fpos_t startOfLine, const ErrorData& errorData) {
    if (srcFile.bad()) {
        std::string errIntro = "ERROR: Failed to read from file " + (std::string)srcPathStr
            + " at line " + std::to_string(errorData.getLine());
        perror(errIntro.c_str());
        exit(-1);
    }

    //In this case the line is probably too long to fit
    if (inputLine[maxLineSize - 1] != NULL) {
        srcFile.clear();
        srcFile.seekg(startOfLine);
        srcFile.getline(inputLine, FPOSMAX, ';');
        if (srcFile.fail()) {
            Error(errorData, ("Exceeding line length limit of " + std::to_string(maxLineSize)
                + " characters. Comments not included").c_str());
        }
        else {
            srcFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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





inline static ubyte GetCommand(const ubyte argc, char** argv) {
    if (argc < 2)
        Error("Command missing. Type \"basm help\" to learn how to use this program");

    char* const command = new char[8];
    strcpy(command, argv[1]);

    constexpr ubyte strSize = 8;
    constexpr char commands[][strSize] = { "help", "-help", "--help", "compile" };

    const std::optional<ubyte> commandI = findStringInArray(command, *commands, std::size(commands), strSize);
    if (!commandI.has_value())
        Error("Unknown command");

    return *commandI;
}





std::ofstream outFile;
std::ifstream srcFile;
const char* srcPathStr;

int main(const ubyte argc, char* argv[]) {
    ubyte commandI = GetCommand(argc, argv);

    switch (commandI) {
    default:
        Error("Command known but unsupported");
    case 0:
    case 1:
    case 2:
        std::cout << "Usage:\nbasm <command> [<options>] [<file> <file>...]\n\n"
            << "Commands:\n1. \"help\" Displays long help message\n2. \"compile\" Compile all specified files\n\n";
        exit(0);
    case 3:
        for (ubyte fileI = 2; fileI < argc && argv[fileI][0] != '-'; fileI++) {
            fs::path outPath(argv[fileI]);
            fs::path srcPath = outPath;
            outPath.replace_extension("o");

            const std::string srcPathString = srcPath.string();
            srcPathStr = srcPathString.c_str();

            outFile.open(outPath);
            if (!outFile.is_open())
                Error((char*)outPath.c_str(), "Failed to create output file");
            outFile.seekp(0);

            srcFile.open(srcPath);
            if (!srcFile.is_open()) {
                char errIntro[] = "ERROR: Failed to open file";
                perror(strcat(errIntro, (char*)srcPath.c_str()));
                exit(-1);
            }


            IMAGE_SECTION_HEADER sections[sectionCount + 1];
            const ubyte coffHeaderSize = 20;
            const ubyte sectionHeaderSize = 40;
            sections[TEXT].mPointerToRawData = coffHeaderSize + (sectionCount * sectionHeaderSize);


            SymbolScopeCount symbolCount = CountSymbols();
            SymbolData* const symtab = FindSymbols(symbolCount);
            SymbolData* symtabEnd = symbolCount.getSymtabEnd(symtab);

            outFile.seekp(sections[TEXT].mPointerToRawData);
            CompileSource(sections, (SymbolData*)symtab, symtabEnd);

            const fpos_t symtabPos = outFile.tellp();
            WriteSymbolTable(srcPath, sections, symtab, symbolCount);
            srcFile.close();
            free(symtab);

            WriteCOFFHeader(symtabPos, outFile.tellp());
            WriteSectionTable(sections);

            outFile.close();
        }
    }

    std::cout << "\nSuccessfully compiled!\n";
}