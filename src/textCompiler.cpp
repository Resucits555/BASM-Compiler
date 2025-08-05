#include <string>
#include <mini-rgx.h>

#include "main.h"
#include "compilationData.h"


static std::ifstream OpenSourceFile(char* sourceFilePath) {
    std::ifstream sourceFile;
    sourceFile.open(sourceFilePath);
    if (!sourceFile.is_open()) {
        Error("Failed to open source file");
    }

    return sourceFile;
}





static opcode_X86 FindOpcode(std::string& str) {
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





inline static std::expected<ubyte, bool> DetectSection(std::string& inputLine, mrx::substr sectionPos) {
    inputLine = inputLine.substr(sectionPos.a, sectionPos.b);

    for (int obj = 0; obj < std::size(sections); obj++) {
        for (int i = 0; sections[obj][i] == inputLine[i]; i++) {
            if (sections[obj][i] == NULL) {
                return obj;
            }
        }
    }
    
    return std::unexpected(false);
}



inline static void GetArguments(std::string& inputLine, argument* args, mrx::substr& mnemonicPos) {
    mrx::substr argPos = mrx::FindRgx(inputLine, "%d+", mnemonicPos.end());

    for (int argI = 0; argPos.a >= 0; ++argI) {
        argument& arg = args[argI];

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

        argPos = mrx::FindRgx(inputLine, "%d+", argPos.end() + 2);
    }
}




static ubyte GetArgumentSize(uintmax_t value) {
    return _64
        | ((value & 0xFFFFFFFF00000000 == 0) * _32)
        | ((value & 0xFFFFFFFFFFFF0000 == 0) * _16)
        | ((value & 0xFFFFFFFFFFFFFF00 == 0) * _8);
}




inline static void WriteToExe(std::ofstream& exeFile, instruction op, argument* args) {
    ubyte towrite = 0;
    for (towrite; op.prefixes[towrite]; towrite++);
    exeFile.write((char*)&op.prefixes, towrite);


    opcodeParts_X86& prs = op.opcode.parts;

    ubyte oppart[4];
    towrite = 1;

    if (prs.OF) {
        oppart[0] = prs.OF;
        oppart[1] = prs.po;
        towrite++;
    }
    else
        oppart[0] = prs.po;

    if (prs.so)
        oppart[towrite++] = prs.so;
    if (prs.o >= 0)
        oppart[towrite++] = prs.o;

    exeFile.write((char*)&oppart, towrite);


    //Missing: modr/m, sib, etc.

    

    for (ubyte arg = 0; args[arg].type; arg++) {
        ubyte argumentSize = GetArgumentSize(args[arg].val);
        exeFile.write((char*)&args[arg].val, argumentSize); //TODO: Include possible var sizes
    }
}




inline void CompileSource(char* sourceFilePath, std::ofstream& exeFile) {
    std::ifstream sourceFile = OpenSourceFile(sourceFilePath);
    exeFile.seekp(baseOfCode);

    ubyte section = 0xFF;
    unsigned long line = 0;


    while (!sourceFile.eof()) {
        std::string inputLine;
        ++line;
        getline(sourceFile, inputLine);

        if (sourceFile.fail())
            Error("Failed to read source file");
        
        sbyte nonSpace = mrx::FindRgx(inputLine, "%S", 0).a;

        if (nonSpace < 0)
            continue;

        if (inputLine[nonSpace] == '.') {
            auto newSection = DetectSection(inputLine, mrx::FindRgx(inputLine, "%a+", 1));
            if (newSection)
                section = newSection.value();
            else
                Error("Unknown section name", line);
            continue;
        }
        
        
        mrx::substr operationPart;
        operationPart.a = mrx::FindRgx(inputLine, "%a", 0).a;

        ubyte end = mrx::FindRgx(inputLine, "%s+;", operationPart.a).a;
        if (end < 0) {
            operationPart.b = UINT8_MAX;
        }
        else {
            operationPart.b = end - operationPart.a;
        }

        if (operationPart.a > operationPart.b || (operationPart.a < 0 && operationPart.b < 0))
            continue;

        inputLine = inputLine.substr(operationPart.a, operationPart.b);


        mrx::substr mnemonicPos = mrx::FindRgx(inputLine, "%a+", 0);

        argument args[3];
        GetArguments(inputLine, args, mnemonicPos);

        std::string mnemonic = inputLine.substr(mnemonicPos.a, mnemonicPos.b);
        opcode_X86 opcode = FindOpcode(mnemonic);
        opcode.parts.o = -1;

        WriteToExe(exeFile, { {}, opcode, 0, 0 }, args);
    }

    sourceFile.close();
}