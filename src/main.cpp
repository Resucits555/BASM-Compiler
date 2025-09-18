#include "main.h"


void Error(const char* message) {
    std::cerr << "ERROR: " << message << '.';

    exit(-1);
}


void Error(const char* message, unsigned long& line) {
    std::cerr << "ERROR: " << message << ". Line: " << line;

    exit(-1);
}




const ubyte nullsSize = 0x50;
const char nulls[nullsSize] = "";

void fillNullUntil(std::ofstream& file, int until) {
    int toFill = until - file.tellp();

    for (toFill; toFill >= nullsSize; toFill -= nullsSize)
        file.write(nulls, nullsSize);
    file.write(nulls, toFill);
}




static std::string cutLastInPath(std::string path) {
    ubyte end = path.length();
    for (end; end > 0; end--) {
        if (path[end] == '/')
            break;
    }
    path = path.substr(0, end);
    return path;
}




int main(int argc, char* argv[]) {
    if (argc < 2)
        Error("Missing source path");


    std::string exePath = cutLastInPath(argv[1]).append("/a.exe");

    std::ofstream exeFile(exePath, std::ios::binary);
    if (!exeFile.is_open())
        Error("Failed to create executable file");


    CompileSource(argv[1], exeFile);
    WriteHeaders(exeFile);

    std::cout << "\nProgram successfully compiled!\n";

    return 0;
}