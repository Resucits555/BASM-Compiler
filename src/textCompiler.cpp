#include <string>
#include <list>

#include "main.h"
#include "compilationData.h"

using namespace std;
ushort line = 0;


static ifstream OpenSourceFile(char* sourceFilePath) {
    ifstream sourceFile;
    sourceFile.open(sourceFilePath);
    if (!sourceFile.is_open()) {
        Error("Failed to open source file");
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
    const char alphanum[] = "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ0123456789";
    const char spaces[] = " \t\x0B";
    const char* groups[] = { alphabet, numbers, alphanum, spaces };

    const char* group = groups[groupI];
    for (int i = 0; group[i] != NULL; i++) {
        if (group[i] == strChar)
            return true;
    }
    return false;
}


//'%' escape and regex character
//COULD STILL HAVE ERRORS
static substr FindRgx(string str, string pattern, int start) {
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
            case 'w':
                group = 2;
                break;
            case 's':
                group = 3;
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
                if (InGroup(group, str[strI]) != !negate) { //basically XOR
                    patI = -1;
                    break;
                }

                while (strI < str.length() && !(InGroup(group, str[strI + 1]) != !negate))
                    ++strI;
                break;
            default:
                if (InGroup(group, str[strI]) != !negate)
                    patI = 0;
                --patI;
            }

            continue;
        }

        escape:
        magic = true;

        if (pattern[patI + 1] != '?') {
            if (!(str[strI] == pattern[patI]))
                patI = -1;
        }
        else {
            --strI;
            ++patI;
        }
            
    }

    exitfor:
    return { strIstart, ++strI - strIstart };
}




inline static ubyte DetectSection(string& inputLine, substr sectionPos) {
    inputLine = inputLine.substr(sectionPos.a, sectionPos.b);
    const char sections[][9] = { "bss", "comment", "data", "debug", "init", "text" };

    for (int obj = 0; obj < std::size(sections); obj++) {
        for (int i = 0; sections[obj][i] == inputLine[i]; i++) {
            if (sections[obj][i] == NULL) {
                return obj;
            }
        }
    }
    
    return UINT8_MAX;
}



inline static void GetArguments(string& inputLine, argument* args, substr& mnemonicPos) {
    substr argPos = FindRgx(inputLine, "%d+", mnemonicPos.end());

    for (int argI = 0; argPos.a >= 0; ++argI) {
        argument& arg = args[argI];
        
        arg.used = true;

        if (inputLine[argPos.a - 1] == '.') {

            switch (inputLine.substr(argPos.a - 2, 1)[0]) {
            case 'r':
                arg.type = reg;
                arg.val = stoi(inputLine.substr(argPos.a, argPos.b));
                break;
            case 'l':
                //TODO: lable things, no break at end
            case 'm':
                arg.type = mem;
                arg.val = stoi(inputLine.substr(argPos.a, argPos.b));
                break;
            }
        }
        else {
            arg.type = imm;
            arg.val = stoi(inputLine.substr(argPos.a, argPos.b));
        }

        argPos = FindRgx(inputLine, "%d+", argPos.end() + 2);
    }
}



inline void CompileSource(char* sourceFilePath) {
    ifstream sourceFile = OpenSourceFile(sourceFilePath);

    ubyte section = 0xFF;

    list<operation> operationList;
    operationList.resize(20);

    while (!sourceFile.eof()) {
        string inputLine;
        ++line;

        getline(sourceFile, inputLine);

        if (sourceFile.fail())
            Error("Failed to read source file");
        


        if (inputLine[FindRgx(inputLine, "%S", 0).a] == '.') {
            section = DetectSection(inputLine, FindRgx(inputLine, "%a+", 1));
            continue;
        }
        
        
        substr operationPart;
        operationPart.a = FindRgx(inputLine, "%a", 0).a;

        int end = FindRgx(inputLine, "%s+;", operationPart.a).a;
        if (end < 0) {
            operationPart.b = INT_MAX;
        }
        else {
            operationPart.b = end - operationPart.a;
        }
        inputLine = inputLine.substr(operationPart.a, operationPart.b);


        substr mnemonicPos = FindRgx(inputLine, "%a+", 0);

        argument args[3];
        GetArguments(inputLine, args, mnemonicPos);

        opcode_X86 opcode = FindOpcode(inputLine.substr(mnemonicPos.a, mnemonicPos.b));


        operationList.push_back({ {}, opcode, 0, 0, args[0], args[1], args[2] });
    }

    sourceFile.close();
}