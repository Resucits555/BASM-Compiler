#include <filesystem>

#include "main.h"


inline void Error(const char* message) {
    printf("ERROR: %s.\n\n", message);
    exit(-1);
}


inline void ErrorNoLine(const ErrorData& errorData, const char* message) {
    std::cerr << "ERROR: " << errorData.getPath() << ": " << message << ".\n\n";
    exit(-1);
}


inline void Error(const ErrorData& errorData, const char* message) {
    std::cerr << "ERROR: " << errorData.getPath() << " at line " << errorData.getLine() << ": " << message << ".\n\n";
    exit(-1);
}


inline void CompilerError(const ErrorData& errorData, const char* message) {
    std::cerr << "Compiler error: " << errorData.getPath() << " at line " << errorData.getLine() << ": "
        << message << ". Please report this bug to the creators of this compiler.\n\n";
    exit(-1);
}





inline ulong roundUp(const ulong& number, const double& roundup) {
    return std::ceil((double)number / roundup) * roundup;
}





std::optional<ubyte> findStringInArray(const char* string, const char* array, ushort arraySize, ubyte stringSize) {
    for (ubyte stringI = 0; stringI < arraySize; stringI++) {
        for (ubyte charI = 0; string[charI] == (array + (stringI * stringSize))[charI]; charI++) {
            if ((array + (stringI * stringSize))[charI] == NULL) {
                return stringI;
            }
        }
    }
    return std::nullopt;
}





inline static ubyte GetCommand(const ubyte argc, char** argv) {
    if (argc < 2)
        Error("Command missing. Type \"basm help\" to learn how to use this program");

    char* const command = new char[8];
    strcpy(command, argv[1]);

    constexpr ubyte commandsSize = 8;
    constexpr char commands[][commandsSize] = { "help", "-help", "--help", "compile"};

    const std::optional<ubyte> _1 = findStringInArray(command, *commands, sizeof(commands), commandsSize);
    if (!_1.has_value())
        Error("Unknown command");

    return _1.value();
}





bool long64bitMode = false;

int main(const ubyte argc, char* argv[]) {
    ubyte commandI = GetCommand(argc, argv);

    switch (commandI) {
    default:
        Error("Command known but unsupported");
    case 0:
    case 1:
    case 2:
        std::cout << "Usage:\nbasm <command> [<options>] [<file>] [<file>...] [-o <outputname>]\n\n"
            << "Commands:\n1. \"help\" Displays long help message\n2. \"compile\" Compile all specified files\n\n";
        exit(0);
    case 3:
        for (ubyte fileI = 2; fileI < argc && argv[fileI][0] != '-'; fileI++) {
            fs::path outPath(argv[fileI]);
            outPath.replace_extension("o");
            std::ofstream outFile(outPath);

            if (!outFile.is_open())
                ErrorNoLine({ 0, (char*)outPath.c_str() }, "Failed to create output file");


            constexpr char sectionNames[][8] = { ".text", ".data", ".bss" };

            constexpr ubyte coffHeaderSize = 20;
            constexpr ubyte sectionHeaderSize = 40;
            constexpr ubyte sectionNumber = std::size(sectionNames);

            IMAGE_SECTION_HEADER sections[sectionNumber];
            for (ubyte sectionI = 0; sectionI < sectionNumber; sectionI++)
                strcpy(sections[sectionI].mName, sectionNames[sectionI]);


            outFile.seekp(coffHeaderSize + (sectionNumber * sectionHeaderSize));
            CompileSource(outFile, argv[fileI], sections);

            outFile.seekp(0);
            WriteCOFFHeader(outFile, sectionNumber);
            outFile.write((char*)&sections, sizeof(sections));
        }
    }

    std::cout << "\nSuccessfully compiled!\n";
}