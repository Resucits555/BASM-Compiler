#include <filesystem>

#include "main.h"
#include "symbols.h"


inline void Error(const char* message) {
    std::cerr << "ERROR: " << message << ".\n\n";
    exit(-1);
}


inline void Error(const char* path, const char* message) {
    std::cerr << "ERROR: " << path << ": " << message << ".\n\n";
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





inline static void WriteSymbolTable(std::ofstream& outFile, fs::path sourcePath, IMAGE_SECTION_HEADER (&sections)[]) {
    COFF_Symbol file;
    strcpy(file.name.shortName, ".file");
    file.value = 0;
    file.sectionNumber = -2;
    file.type = symbol_type::IMAGE_SYM_TYPE_NULL;
    file.storageClass = symbol_class::IMAGE_SYM_CLASS_FILE;
    file.numberOfAuxSymbols = 1;

    outFile.write((char*)&file, COFF_Symbol_Size);


    const ubyte filenameLength = sourcePath.filename().string().length();
    
    if (filenameLength > 18) {
        Error("Filenames longer than 18 characters are not supported");
        //write filename to string table
    }
    else {
        char filename[18] = "";
        strcpy(filename, (char*)sourcePath.filename().u8string().c_str());
        outFile.write(filename, 18);
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

        COFF_AuxSection aux;
        aux.length = sections[sectionI].mSizeOfRawData;
        outFile.write((char*)&aux, COFF_Symbol_Size);
    }

    outFile.seekp((int)outFile.tellp() + 17);
    outFile.put(0);
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
                Error((char*)outPath.c_str(), "Failed to create output file");


            IMAGE_SECTION_HEADER sections[sectionNumber];
            sections[0].mPointerToRawData = coffHeaderSize + (sectionNumber * sectionHeaderSize);


            outFile.seekp(sections[0].mPointerToRawData);
            CompileSource(outFile, argv[fileI], sections);

            const ulong symbolTablePointer = outFile.tellp();
            WriteSymbolTable(outFile, (fs::path)argv[fileI], sections);
            WriteCOFFHeader(outFile, symbolTablePointer);
            WriteSectionTable(outFile, sections);
        }
    }

    std::cout << "\nSuccessfully compiled!\n";
}