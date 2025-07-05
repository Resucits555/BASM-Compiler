#include "main.h"


void Error(const char* message) {
    std::cerr << "ERROR: ";
    std::cerr << message << '.';
    if (line) {
        std::cerr << " Line: " << line;
    }
    exit(1);
}



int main(int argc, char* argv[]) {
    if (argc < 2)
        Error("Missing source path");
    WriteHeaders(argv[1]);
    CompileSource(argv[1]);

    std::cout << "Program successfully compiled!\n";

    return 0;
}