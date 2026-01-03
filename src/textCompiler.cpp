#include <pugixml.hpp>
#include <regex>
#include <cmath>

#include "main.h"
#include "compilationData.h"


static ErrorData errorData;


static ubyte minBitsToStoreValue(uintmax_t value, bool negative) {
    if (negative)
        value = ~value << 1;

    return floor(log2(value) + 1);
}




static argument getArgument(char* argstr) {
    for (ubyte chr = 0; chr < strlen(argstr); chr++)
        argstr[chr] = std::tolower(argstr[chr]);

    const ubyte strSize = 5;
    const char registers[][strSize] = {
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
    std::optional<ubyte> reg = findStringInArray(argstr, *registers, std::size(registers), strSize);
    if (!reg.has_value())
        //Remove when the other registers are supported too
        Error(errorData, "Invalid argument. Note that only standard registers from al to r15 are supported in this version");

    return { 'r', false, (ubyte)(pow(2, *reg >> 4) * 8), false, (ubyte)(*reg % 16) };
}





inline static void GetArguments(char* inputLine, argument* args) {
    for (sbyte i = 0; i <= 2; i++) {
        char* argstr = strtok(nullptr, instrDelimiters);
        if (argstr == nullptr)
            return;

        argument& arg = args[i];
        if (std::regex_search(argstr, std::regex("\\D"))) {
            arg = getArgument(argstr);
        }
        else {
            arg.type = 'I';
            if (*argstr == '-')
                arg.negative = true;
            arg.mutableSize = true;
            arg.val = std::stoull(argstr, nullptr, 0);
            arg.size = ceil(minBitsToStoreValue(arg.val, arg.negative) / 8.);
        }
    }

    Error(errorData, "Invalid argument");
}





inline static bool isCorrectType(argument& textArg, pugi::xml_node argNode, instruction& instr) {
    char type[4];
    strcpy(type, argNode.first_child().next_sibling().child_value());

    ubyte arrayI = log2(textArg.size / 8);


    const ubyte strSize = 4;
    const char _8[][strSize] = { "b", "bs", "bss" };
    const ubyte _8req[3] = {};
    const char _16[][strSize] = { "a", "v", "vds", "vq", "vqp", "vs", "w", "wi", "va", "wa", "wo", "ws" };
    const ubyte _16req[12] = { SHRINK | DOUBLED, SHRINK, SHRINK, SHRINK, SHRINK, SHRINK, 0, 0, NOSHRINK };
    const char _32[][strSize] = { "a", "d", "di", "dqp", "ds", "p", "ptp", "sr", "v", "vds", "vqp", "vs", "va", "dqa", "da", "do" };
    const ubyte _32req[16] = { NOSHRINK | DOUBLED, 0, 0, 0, 0, SHRINK, SHRINK, 0, NOSHRINK, NOSHRINK, NOSHRINK, NOSHRINK, ADDRESS };
    const char _64[][strSize] = { "dqp", "dr", "pi", "psq", "q", "qi", "qp", "vqp", "dqa", "qa", "qs" };
    const ubyte _64req[11] = { REXW, 0, 0, 0, 0, 0, REXW, REXW, ADDRESS };

    const char* const operands[] = { *_8, *_16, *_32, *_64 };
    const ubyte* const requirements[] = { _8req, _16req, _32req, _64req };
    constexpr ubyte arraySizes[] = { std::size(_8), std::size(_16), std::size(_32), std::size(_64) };

    retry:
    const std::optional<ubyte> operandI = findStringInArray(type, operands[arrayI], arraySizes[arrayI], strSize);
    if (!operandI.has_value()) {
        if (textArg.mutableSize && arrayI < 3) {
            arrayI++;
            goto retry;
        }
        return false;
    }

    if (textArg.type == 'I')
        instr.immediateSize = pow(2, arrayI);


    const ubyte requirement = requirements[arrayI][*operandI];

    if (requirement) {
        const ubyte prefixValues[3] = { operandSizePrefix, addressSizePrefix, REX.W };
        for (ubyte reqI = 1; reqI < NOSHRINK; reqI++) {
            if ((requirement & 0x7) == reqI && std::find(instr.prefixes, instr.prefixes + 4, prefixValues[reqI - 1]) == instr.prefixes + 4)
                instr.addPrefix(prefixValues[reqI - 1]);
        }

        if (requirement & NOSHRINK) {
            if (std::find(instr.prefixes, instr.prefixes + 4, operandSizePrefix) != instr.prefixes + 4
                || std::find(instr.prefixes, instr.prefixes + 4, addressSizePrefix) != instr.prefixes + 4)
                return false;
        }
    }

    return true;
}



//Returns instruction data if it has fitting arguments etc., otherwise returns nullopt
static std::optional<instruction> isFittingInstruction(const pugi::xml_node pri_opcd, const pugi::xml_node syntax, argument* args) {
    instruction instr;
    if (strcmp(pri_opcd.parent().name(), "two-byte") == 0) {
        std::cout << syntax.parent().parent().parent().name();
        instr.opcode[0] = 0x0F;
        instr.primaryOpcodeIndex++;
    }

    ubyte& po = instr.opcode[instr.primaryOpcodeIndex];

    po = std::stoi(pri_opcd.first_attribute().value(), nullptr, 16);
    
    ubyte textArgCounter = 0;
    for (;textArgCounter < 2 && args[textArgCounter].type != NULL; textArgCounter++);

    instr.modrm = 0b11000000;


    ubyte entryArgCounter = 0;
    for (pugi::xml_node argNode = syntax.first_child().next_sibling();
        argNode.type() != pugi::node_null; argNode = argNode.next_sibling()) {

        argument& textArg = args[entryArgCounter];


        char addressing[4];
        if (!argNode.first_attribute().empty()) {
            if (*argNode.child_value() == 'S')
                continue;
            strcpy(addressing, argNode.child_value());
            argument entryArg = getArgument(addressing);
            if (entryArg != textArg)
                return std::nullopt;

            entryArgCounter++;
            continue;
        }

        strcpy(addressing, argNode.first_child().child_value());

        if (addressing[1] != NULL)
            Error(errorData, "Compiler does not support this type of instruction yet");

        switch (textArg.type) {
        case 'r':
            switch (addressing[0]) {
            default:
                return std::nullopt;
            case 'E':
            case 'H':
            case 'R':
                instr.modrmUsed = true;
                instr.modrm |= textArg.val;
                break;
            case 'G':
                instr.modrmUsed = true;
                instr.modrm |= textArg.val << 3;
                break;
            case 'Z':
                po += textArg.val;
            }
            break;
        case 'I':
            if (*addressing != 'I')
                return std::nullopt;
            instr.immediate = textArg.val;
        }


        if (!isCorrectType(textArg, argNode, instr))
            return std::nullopt;

        entryArgCounter++;
    }

    if (textArgCounter != entryArgCounter)
        return std::nullopt;

    return instr;
}




inline static instruction FindInstruction(char* mnemonic, argument* args, const pugi::xml_node& one_byte) {
    const ubyte mnemonicLen = strlen(mnemonic);
    for (ubyte chr = 0; chr < strlen(mnemonic); chr++)
        mnemonic[chr] = std::toupper(mnemonic[chr]);

    std::optional<instruction> bestInstrFound;

    for (auto pri_opcd = one_byte.first_child(); pri_opcd.type() != pugi::node_null; pri_opcd = pri_opcd.next_sibling()) {
        for (auto entry = pri_opcd.first_child(); entry.type() != pugi::node_null; entry = entry.next_sibling()) {
            for (auto syntax = entry.first_child(); strcmp(syntax.name(), "syntax") == 0; syntax = syntax.next_sibling()) {
                if (strcmp(mnemonic, syntax.first_child().child_value()) == 0) {
                    std::optional<instruction> newInstruction = isFittingInstruction(pri_opcd, syntax, args);
                    if (!newInstruction.has_value())
                        continue;

                    bestInstrFound = newInstruction;
                    return bestInstrFound.value();
                }
            }
        }

        if (pri_opcd.first_attribute().as_int() == 0xFF)
            pri_opcd = one_byte.next_sibling().first_child();
    }

    if (!bestInstrFound.has_value())
        Error(errorData, "No fitting opcode found. One of the arguments could be invalid for this instruction");

    return *bestInstrFound;
}





inline static void WriteToExe(std::ofstream& outFile, instruction instr) {
    outFile.write((char*)&instr.prefixes, instr.getNextPrefixIndex());

    outFile.write((char*)&instr.opcode, instr.primaryOpcodeIndex + 1);

    if (instr.modrmUsed)
        outFile.put(instr.modrm);
    if (instr.sibUsed)
        outFile.put(instr.sib);
    
    outFile.write((char*)&instr.immediate, instr.immediateSize);
}





inline void CompileSource(IMAGE_SECTION_HEADER (&sections)[]) {
    srcFile.seekg(0);

    pugi::xml_document x86reference;
    const char referencePath[] = "../data/x86reference-master/x86reference.xml";
    pugi::xml_parse_result result = x86reference.load_file(referencePath);
    if (result.status)
        Error(referencePath + 1, result.description());
    pugi::xml_node one_byte = x86reference.first_child().first_child();

    errorData = {};
    char inputLine[maxLineSize] = "";
    bool inTextSection = false;

    while (!srcFile.eof()) {
        errorData.incLine();

        const fpos_t startOfLine = srcFile.tellg();
        srcFile.getline(inputLine, FPOSMAX);

        if (srcFile.fail())
            ProcessInputError(inputLine, startOfLine, errorData);

        if (strncmp(inputLine, "section", 7) == 0) {
            if (inTextSection) {
                break;
            }
            else if (strncmp(inputLine, "section .text", 13) == 0) {
                inTextSection = true;
                continue;
            }
        }
        
        if (!inTextSection)
            continue;

        if (char* comment = strchr(inputLine, ';'))
            *comment = NULL;
        if (!std::regex_search(inputLine, std::regex("\\S")))
            continue;
        if (strchr(inputLine, ':'))
            continue;


        char* mnemonic = strtok(inputLine, instrDelimiters);
        if (strncmp(mnemonic, "extern", 7) == 0)
            continue;

        instruction instr;
        argument args[2];
        GetArguments(inputLine, args);

        instr = FindInstruction(mnemonic, args, one_byte);

        WriteToExe(outFile, instr);
    }
    
    sections[TEXT].mSizeOfRawData = (uint32_t)outFile.tellp() - sections[TEXT].mPointerToRawData;
}