#include <string>

#include "main.h"
#include "compilationData.h"

using namespace std;
static ushort line = 0;


static ifstream OpenSourceFile(char* sourceFilePath) {
    ifstream sourceFile;
    sourceFile.open(sourceFilePath);
    if (!sourceFile.is_open()) {
        cerr << "Unable to open file";
        exit(1);
    }

    return sourceFile;
}



static opcode_X86 FindOpcode(string& str) {
    opcode_X86 opcode = { 0xFFFFFFFF };
    
    for (int obj = 0; obj < std::size(opcodeTable); obj++) {
        for (int i = 0; opcodeTable[obj].mnemonic[i] == str[i]; i++) {
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



static bool InGroup(int groupI, char strChar) {
    const char alphabet[] = "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ";
    const char numbers[] = "0123456789";
    const char spaces[] = " \t\x0B";
    const char* groups[] = { alphabet, numbers, spaces };

    const char* group = groups[groupI];
    for (int i = 0; group[i] != NULL; i++) {
        if (group[i] == strChar)
            return true;
    }
    return false;
}


//'%' escape and regex character
//COULD STILL HAVE ERRORS
static intPair FindRgx(string str, string pattern, int start) {
    int& strI = start;
    --strI;
    ushort strIstart = 0;
    bool magic = true;
    for (ushort patI = 0; patI < pattern.length(); ++patI) {
        ++strI;
        
        if (!patI)
            strIstart = strI;

        if (strI >= str.length())
            return { -1, -2 };

        if (pattern[patI] == '%' && magic) {
            bool negate = false;
            ++patI;
            if (pattern[patI] >= 'A' && pattern[patI] <= 'Z') {
                negate = true;
                pattern[patI] += 0x20;
            }

            ubyte group;
            
            switch (pattern[patI]) {
            case 'a':
                group = 0;
                break;
            case 'd':
                group = 1;
                break;
            case 's':
                group = 2;
                break;
            case '%':
                magic = false;
                goto escape;
            default:
                ++strI;
                goto escape;
            }

            switch (pattern[++patI]) {
            case '+':
                if (InGroup(group, str[strI]) && negate) { //basically XOR
                    patI = -1;
                    break;
                }

                while (strI < str.length() && InGroup(group, str[strI + 1]))
                    ++strI;
                break;
            default:
                if (InGroup(group, str[strI]) && negate)
                    patI = 0;
                --patI;
            }

            continue;
        }

        escape:
        magic = true;

        if (!str[strI] == pattern[patI] && str[strI + 1] != '?')
            patI = -1;
    }

    exitfor:
    return { strIstart, ++strI - strIstart };
}



inline void CompileSource(char* sourceFilePath) {
    ifstream sourceFile = OpenSourceFile(sourceFilePath);

    while (true) {
        string inputLine;
        ++line;

        getline(sourceFile, inputLine);

        if (sourceFile.fail())
            Error("Failed to read source file", sourceFile.tellg());



        switch (inputLine[FindRgx(inputLine, "%S", 0).a]) {
        case '.': {
            intPair sectionPos = FindRgx(inputLine, "%a+", 1);
            inputLine = inputLine.substr(sectionPos.a, sectionPos.b);
            const char sections[][9] = { "bss", "comment", "data", "debug", "init", "text" };

            ubyte section = 0xFF;
            for (int obj = 0; obj < std::size(sections); obj++) {
                for (int i = 0; sections[obj][i] == inputLine[i]; i++) {
                    const char* chr = &opcodeTable[obj].mnemonic[i];
                    if (*chr == NULL) {
                        section = obj;
                        goto fullBreak;
                    }
                }
            }
            fullBreak:
            cout << to_string(section);

            break;
        }
        default:
            intPair mnemonicPos = FindRgx(inputLine, "%a+", 0);
            if (mnemonicPos.a < 0)
                break;

            string mnemonic = inputLine.substr(mnemonicPos.a, mnemonicPos.b);
            inputLine = inputLine.substr(mnemonicPos.a + mnemonicPos.b, SIZE_MAX);
            opcode_X86 opcode = FindOpcode(mnemonic);
        }


        if (sourceFile.eof())
            break;
    }

    sourceFile.close();
}