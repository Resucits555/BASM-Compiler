#include <filesystem>

#include "main.h"


void Error(const char* message) {
    printf("ERROR: %s.\n\n", message);

    exit(-1);
}


void Error(const char* message, ulong& line) {
    std::cerr << "ERROR: " << message << ". Line: " << line << "\n\n";

    exit(-1);
}


void CompilerError(const char* message, ulong& line) {
    std::cerr << "Compiler error: " << message << ". Please report this bug to the creators of this compiler. Line: " << line << "\n\n";

    exit(-1);
}





inline ulong roundUp(const ulong& number, const double& roundup) {
    return std::ceil((double)number / roundup) * roundup;
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
ushort headerSize = 0x200; //Currently used as a constant, function for calculating header size needed

bool long64bitMode = false;

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
    sizeOfCode = (ulong)exeFile.tellp() - headerSize;
    sizeOfImage = exeFile.tellp();

    WriteHeaders(exeFile);

    exeFile.close();
    std::cout << "\nProgram successfully compiled!\n";

    return 0;
}