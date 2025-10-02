#include <filesystem>
#include <pugixml.hpp>
#include <mini-rgx.h>

#include "main.h"
#include "compilationData.h"


inline static sbyte NewSection(std::string& inputLine) {
    inputLine = inputLine.substr(1, SIZE_MAX);

    for (int obj = 0; obj < std::size(sections); obj++) {
        for (int i = 0; sections[obj][i] == inputLine[i]; i++) {
            if (sections[obj][i] == NULL) {
                return obj;
            }
        }
    }

    return -1;
}





inline static void GetArguments(std::string& inputLine, argument* args) {
    mrx::substr instructionParts[4] = {};
                         // mnem type1  arg1  type2  arg2
    const char pat[] = "%0%s*%a+ %1%a?.?%2%w, %3%a?.?%4%w";
    mrx::FindRgxSectional(instructionParts, inputLine, pat);


}





//Returns the minimum amount of bits the value could be stored in
static ubyte minBitsToStore (uintmax_t value, bool negative) {
    if (value == 0)
        return 1;
    if (negative)
        value = value << 1;

    return floor(log2(value) + 1);
}



inline static instruction FindInstruction(const char* mnemonic) {
    return {};
}





inline static void WriteToExe(std::ofstream& exeFile, instruction op, argument* args) {
    ubyte towrite = 0;
    for (towrite; op.prefixes[towrite]; towrite++);
    exeFile.write((char*)&op.prefixes, towrite);


    opcodeParts_x86& prs = op.opcode.parts;

    exeFile.write((char*)&prs.po, 1);
    if (prs.po == 0x0F)
        exeFile.write((char*)&prs.so, 1);
    

    for (ubyte arg = 0; args[arg].type; arg++) {
        exeFile.write((char*)&args[arg].val, 4);
    }
}





inline void CompileSource(const fs::path& srcPath, std::ofstream& exeFile) {
    std::ifstream sourceFile;
    sourceFile.open(srcPath);

    if (!sourceFile.is_open())
        Error("Failed to open source file. Your path may be wrong. '/' and '\\' are treated the same");


    exeFile.seekp(baseOfCode);

    ubyte section = UINT8_MAX;
    ulong line = 0;


    while (!sourceFile.eof()) {
        std::string inputLine;
        ++line;
        getline(sourceFile, inputLine);

        if (sourceFile.fail())
            Error("Failed to read source file. Your path may be wrong. '/' and '\\' are treated the same");

        if (!mrx::FindCharOfGroup('S', inputLine).has_value())
            continue;


        if (inputLine[0] == '.') {
            sbyte newSection = NewSection(inputLine);
            if (newSection < 0)
                Error("Unknown section name", line);
            else
                section = newSection;
        }
        

        std::optional<const char*> mnemonic = mrx::FindRgxCSubstr(inputLine, "%a+");

        if (!mnemonic.has_value())
            Error("Invalid characters", line);

        instruction instr = FindInstruction(mnemonic.value());


        argument args[2];
        GetArguments(inputLine, args);

        WriteToExe(exeFile, instr, args);
    }

    sourceFile.close();
}