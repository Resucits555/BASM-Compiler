#include <filesystem>

#include "main.h"


void Error(const char* message) {
    printf("ERROR: %s.", message);

    exit(-1);
}


void Error(const char* message, ulong& line) {
    std::cerr << "ERROR: " << message << ". Line: " << line;

    exit(-1);
}





void fillNullUntil(std::ofstream& file, int until) {
    const ubyte nullsSize = 0x50;
    const char nulls[nullsSize] = "";

    int toFill = until - file.tellp();

    for (toFill; toFill >= nullsSize; toFill -= nullsSize)
        file.write(nulls, nullsSize);
    file.write(nulls, toFill);
}





static void ProcessArguments(fs::path& srcPath, fs::path& exePath, const ubyte argc, char** argv) {
    for (ubyte argI = 1; argI < argc; argI++) {
        char* arg = argv[argI];

        if (arg[0] == '-') {
            if (strcmp(arg, "-help") == 0) {
                printf("Usage:\nbassemble [<options>] <src path>\n\nThe compiled executable will be created in the same directory as the source, with the same name\n\n");
                printf("Possible arguments:\nhelp - Displays this message\n\n");
                exit(0);
            }
        }
    }
    srcPath = argv[1];
    exePath = srcPath;
    exePath.replace_extension("exe");
}




ulong sizeOfCode;
ulong sizeOfImage;

int main(const ubyte argc, char* argv[]) {
    if (argc < 2)
        Error("Missing source path");

    fs::path srcPath;
    fs::path exePath;
    ProcessArguments(srcPath, exePath, argc, argv);


    std::ofstream exeFile(exePath, std::ios::binary);
    if (!exeFile.is_open())
        Error("Failed to create executable file");


    CompileSource(srcPath, exeFile);
    sizeOfCode = (ulong)exeFile.tellp() - (ulong)baseOfCode;
    fillNullUntil(exeFile, std::ceil((double)exeFile.tellp() / sectionAlignment) * sectionAlignment);
    sizeOfImage = exeFile.tellp();

    WriteHeaders(exeFile);

    exeFile.close();
    std::cout << "\nProgram successfully compiled!\n";

    return 0;
}