#include <string>

#include "main.h"
#include "compilationData.h"

using namespace std;

static ifstream OpenSourceFile(char* sourceFilePath) {
    ifstream sourceFile;
    sourceFile.open(sourceFilePath);
    if (!sourceFile.is_open()) {
        cerr << "Unable to open file";
        exit(1);
    }

    return sourceFile;
}



static opcode_X86 findOpcode(const char* string) {
    opcode_X86 opcode = { 0xFFFFFFFF };
    
    for (int obj = 0; obj < std::size(opcodeTable); obj++) {
        for (int i = 0; opcodeTable[obj].mnemonic[i] == string[i]; i++) {
            const char* chr = &opcodeTable[obj].mnemonic[i];
            if (*chr == NULL) {
                opcode = opcodeTable[obj].opcode;
                goto fullBreak;
            }
        }
    }

    fullBreak:
    return opcode;
}



//'%' escape and regex character
static intPair findRgx(string str, string pattern, int start) {
    struct border {
        char l;
        char h;

        bool inside(int var) const {
            if (var >= l && var <= h)
                return true;
            return false;
        }
    };

    const border alphabet = { 'a', 'Z' };
    const border numbers = { '0', '9' };
    const border* borders[] = { &alphabet, &numbers };

    ushort patI = 0;
    bool magic = true;
    for (ushort strI = start; strI < str.length(); strI++) {
        const char strChar = str[strI];

        if (patI == pattern.length())
            return { strI - patI, strI };

        if (strChar == '%' && magic) {
            ubyte group;
            switch (str[strI + 1]) {
            case 'a':
                group = 0;
                break;
            case 'd':
                group = 1;
                break;
            case '%':
                ++strI;
                magic = false;
                goto escape;
            default:
                ++strI;
                goto escape;
            }
            ++strI;

            if (!borders[group]->inside(strChar)) {
                patI = 0;
                continue;
            }

            escape:
            ++patI;
            continue;
        }

        magic = true;

        if (!strChar == pattern[patI]) {
            patI = 0;
            continue;
        }

        ++patI;
    }

    return { -1, -1 };
}



void CompileSource(char* sourceFilePath) {
    ifstream sourceFile = OpenSourceFile(sourceFilePath);

    while (true) {
        string inputLine;
        getline(sourceFile, inputLine);

        if (sourceFile.fail())
            Error("Failed to read source file", sourceFile.tellg());
        
        cout << inputLine << '\n';

        
        if (sourceFile.eof())
            break;
    }

    sourceFile.close();
}