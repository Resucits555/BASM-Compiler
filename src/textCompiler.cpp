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





static std::optional<ubyte> findStringInArray(const char* string, const char* array, ubyte arraySize, ubyte stringSize) {
    for (ubyte stringI = 0; stringI < arraySize; stringI++) {
        for (ubyte charI = 0; string[charI] == (array + (stringI * stringSize))[charI]; charI++) {
            if ((array + (stringI * stringSize))[charI] == NULL) {
                return stringI;
            }
        }
    }
    return std::nullopt;
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





inline static void GetArguments(const char* inputLine, argument* args, ulong line) {
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


    char* tokenizedInput = (char*)malloc(strlen(inputLine) + 1);
    if (tokenizedInput == nullptr)
        Error("\"malloc\" function failed");

    strcpy(tokenizedInput, inputLine); //Visual Studio does not recognise exit() as a deadend function, aka computer dumb

    const char delimiters[] = " \t,.";
    strtok(tokenizedInput, delimiters); //don't need the mnemonic

    char* argstr = strtok(nullptr, delimiters);
    for (sbyte i = 0; argstr != nullptr; i++) {
        if (i >= 2)
            Error("Invalid arguments", line);

        argument& arg = args[i];

        if (!mrx::FindRgx(inputLine, "%D").has_value()) {
            arg.type = 'I';
            if (*argstr == '-')
                arg.negative = true;
            arg.variableSize = true;
            arg.val = std::stoull(argstr, nullptr, 0);
        }
        else {
            std::optional<ubyte> reg = findStringInArray(argstr, *registers, std::size(registers), 6);
            if (reg.has_value())
                arg = { 'r', false, (ubyte)pow(2, reg.value() >> 4), false, (ubyte)(reg.value() % 16) };
        }

        argstr = strtok(nullptr, delimiters);
    }

    free(tokenizedInput);
}





//Returns instruction data if it has fitting arguments and other criteria, otherwise returns null
static std::optional<instruction> isFittingInstruction(const pugi::xml_node& entry, argument* args) {
    const ubyte operandSizePrefix = 0x66;
    const ubyte addressSizePrefix = 0x67;
    enum REX : ubyte {
        REX = 40,
        REXB = 41,
        REXX = 42,
        REXR = 44,
        REXW = 48
    };

    instruction instr;
    ubyte& po = instr.opcode[instr.primaryOpcodeIndex];


    po = std::stoi(entry.parent().first_attribute().value(), nullptr, 16);

    ubyte textArgCounter = 0;
    for (;textArgCounter < 2 && args[textArgCounter].type != NULL; textArgCounter++);


    ubyte entryArgCounter = 0;
    for (pugi::xml_node argNode = entry.first_child().child("dst"); argNode.type() != pugi::node_null; argNode = argNode.next_sibling()) {
        argument& textArg = args[entryArgCounter];

        if (textArg.variableSize)
            textArg.size = std::ceil((double)minBitsToStore(textArg.val, textArg.negative) / 8.);

        char addressing[4] = "";
        strcpy(addressing, argNode.first_child().child_value());

        if (addressing[1] != NULL)
            return std::nullopt;

        switch (textArg.type) {
        case 'r':
            switch (addressing[0]) {
            case 'E':
            case 'H':
                instr.modrmUsed = true;
                instr.modrm |= (0b11 << 6) | textArg.val;
                break;
            case 'G':
                instr.modrmUsed = true;
                instr.modrm |= textArg.val << 3;
                break;
            case 'R': //this could be wrong but that's how I understood the description of 'R'
                instr.modrmUsed = true;
                instr.modrm |= textArg.val << 6;
                break;
            case 'Z':
                po += textArg.val;
                break;
            default:
                return std::nullopt;
            }
            break;
        case 'I':
            instr.immediate = textArg.val;
            instr.immediateSize = textArg.size;
        }


        char type[4] = "";
        strcpy(type, argNode.first_child().next_sibling().child_value());



        entryArgCounter++;
    }

    if (textArgCounter != entryArgCounter)
        return std::nullopt;

    return instr;
}




inline static instruction FindInstruction(std::string& mnemonic, argument* args, const pugi::xml_node& one_byte) {
    for (int chr = 0; chr < mnemonic.size(); chr++)
        mnemonic[chr] = std::toupper(mnemonic[chr]);

    instruction bestInstrFound;

    for (auto pri_opcd = one_byte.first_child(); pri_opcd.type() != pugi::node_null; pri_opcd = pri_opcd.next_sibling()) {
        for (auto entry = pri_opcd.first_child(); entry.type() != pugi::node_null; entry = entry.next_sibling()) {
            if (mnemonic == entry.child("syntax").first_child().child_value()) {
                auto newInstruction = isFittingInstruction(entry, args);
                if (!newInstruction.has_value())
                    continue;
                bestInstrFound = newInstruction.value();
                return bestInstrFound;
            }
        }

        if (pri_opcd.first_attribute().as_int() == 0xFF) {
            pri_opcd = one_byte.next_sibling().first_child();
            bestInstrFound.opcode[0] = 0x0F;
            bestInstrFound.primaryOpcodeIndex++;
        }
    }

    return bestInstrFound;
}





inline static void WriteToExe(std::ofstream& exeFile, instruction instr) {
    exeFile.write((char*)&instr.prefixes, instr.lastPrefixIndex + 1);

    exeFile.write((char*)&instr.opcode, instr.primaryOpcodeIndex + 1);

    if (instr.modrmUsed)
        exeFile.put(instr.modrm);
    if (instr.sibUsed)
        exeFile.put(instr.sib);
    
    exeFile.write((char*)&instr.data, instr.dataSize);
    exeFile.write((char*)&instr.immediate, instr.immediateSize);
}





inline void CompileSource(const fs::path& srcPath, std::ofstream& exeFile) {
    exeFile.seekp(baseOfCode);

    std::ifstream sourceFile;
    sourceFile.open(srcPath);

    pugi::xml_document x86reference;
    x86reference.load_file("../data/x86reference-master/x86reference.xml");
    pugi::xml_node one_byte = x86reference.first_child().first_child();

    if (!sourceFile.is_open())
        Error("Failed to open source file. Your path may be wrong. '/' and '\\' are treated the same");


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

        std::optional<std::string> mnemonic = mrx::FindRgxSubstr(inputLine, "%a+");
        if (!mnemonic.has_value())
            Error("Invalid characters", line);

        instruction instr;
        argument args[2];
        GetArguments(inputLine.c_str(), args, line);

        instr = FindInstruction(mnemonic.value(), args, one_byte);
        if (instr.dataSize + instr.immediateSize > 8)
            Error("Compiler bug: Sum of disp and imm sizes is more than 8 bytes. Please report this bug to the creators of this compiler", line);

        WriteToExe(exeFile, instr);
    }

    const ubyte leaveInstruction = 0xC9;
    exeFile.put(leaveInstruction);
    const ubyte returnInstruction = 0xC3;
    exeFile.put(returnInstruction);

    sourceFile.close();
}