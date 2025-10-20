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





static void ProcessArguments(fs::path srcPath, fs::path& exePath, const char* argv) {
    exePath = srcPath.replace_extension("exe");
}




ulong sizeOfCode;
ulong sizeOfImage;

int main(const int argc, const char* argv[]) {
    if (argc < 2)
        Error("Missing source path");


    fs::path srcPath(argv[1]);
    fs::path exePath;
    ProcessArguments(srcPath, exePath, *argv);


    std::ofstream exeFile(exePath, std::ios::binary);
    if (!exeFile.is_open())
        Error("Failed to create executable file");


    CompileSource(srcPath, exeFile);
    sizeOfCode = (ulong)exeFile.tellp() - (ulong)baseOfCode;
    sizeOfImage = std::ceil((double)exeFile.tellp() / sectionAlignment) * sectionAlignment;

    WriteHeaders(exeFile);

    exeFile.close();
    std::cout << "\nProgram successfully compiled!\n";

    return 0;
}