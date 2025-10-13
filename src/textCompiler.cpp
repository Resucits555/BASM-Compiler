#include <filesystem>
#include <pugixml.hpp>
#include <mini-rgx.h>

#include "main.h"
#include "compilationData.h"


//Returns the minimum amount of bits the value could be stored in
static ubyte minBitsToStore(uintmax_t value, bool negative) {
    if (negative)
        value = ~value << 1;

    return floor(log2(value) + 1);
}





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





static std::optional<argument> FindRegisterRef(const char* string) {
    const char registers[][6] = {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "r8l", "r9l", "r10l", "r11l", "r12l", "r13l", "r14l", "r15l",
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w",
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
    //"spl", "bpl", "sil", "dil"
    /*"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
    "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15",
    "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7",
    "mmx0", "mmx1", "mmx2", "mmx3", "mmx4", "mmx5", "mmx6", "mmx7"*/
    };

    for (ubyte stringI = 0; stringI < std::size(registers); stringI++) {
        for (ubyte charI = 0; string[charI] == registers[stringI][charI]; charI++) {
            if (registers[stringI][charI] == NULL) {
                return argument('r', (ubyte)pow(2, stringI >> 4), false, (ubyte)(stringI % 16));
            }
        }
    }
    return std::nullopt;
}




inline static void GetArguments(const char* inputLine, argument* args, ulong line) {
    char* tokenizedInput = (char*)malloc(strlen(inputLine) + 1);
    strcpy(tokenizedInput, inputLine);

    const char delimiters[] = " \t,.";
    strtok(tokenizedInput, delimiters);

    char* argstr = (char*)1;
    for (sbyte i = 0; argstr != nullptr; i++) {
        if (i >= 3)
            Error("Invalid arguments", line);

        argument& arg = args[i];
        argstr = strtok(nullptr, delimiters);

        if (!mrx::FindRgx(inputLine, "%D").has_value()) {
            arg.type = 'I';
            if (*argstr == '-')
                arg.negative = true;
            arg.val = std::stoull(argstr, nullptr, 0);
        }
        else {
            std::optional<argument> reg = FindRegisterRef(argstr);
            if (reg.has_value()) {
                arg = reg.value();
                return;
            }
        }
    }

    free(tokenizedInput);
}





inline static instruction FindInstruction(const char* mnemonic, argument* args) {


    return {};
}





inline static void WriteToExe(std::ofstream& exeFile, instruction instr) {
    for (ubyte prefix = 0; prefix < instr.prefixes.size(); prefix++)
        exeFile.put(instr.prefixes[prefix]);

    opcodeParts_x86& prs = instr.opcode.parts;

    exeFile.put(prs.po);
    if (prs.po == 0x0F)
        exeFile.put(prs.so);
    
    exeFile.write((char*)&instr.data, instr.dataSize);
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
            continue;
        }


        size_t commentPos = inputLine.find(';');
        if (commentPos != std::string::npos)
            inputLine = inputLine.substr(0, commentPos--);

        std::optional<const char*> mnemonic = mrx::FindRgxCSubstr(inputLine, "%a+");

        if (!mnemonic.has_value())
            Error("Invalid characters", line);

        instruction instr;
        argument args[2];
        GetArguments(inputLine.c_str(), args, line);

        instr = FindInstruction(mnemonic.value(), args);

        WriteToExe(exeFile, instr);
    }

    sourceFile.close();
}