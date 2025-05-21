#include "main.h"

using namespace std;


void Error(const char* message) {
    cerr << "ERROR: ";
    cerr << message << '.';
    if (line) {
        cerr << " Line: " << line;
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