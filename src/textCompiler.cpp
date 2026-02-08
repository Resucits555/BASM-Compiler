#include <pugixml.hpp>
#include <regex>
#include <cmath>

#include "main.h"
#include "symbols.h"
#include "compilationData.h"


static ErrorData errorData;
static COFF_Relocation* currentReloc;
static COFF_Relocation* relocEnd;


static ubyte minBitsToStoreValue(uint64_t value, const bool negative) {
    if (negative)
        value = ~value << 1;
    if (value == 0)
        return 1;

    return floor(log2(value) + 1);
}





static std::optional<argument> getRegArgument(char* str) {
    const ubyte strSize = 5;
    ubyte argLen = strlen(str);
    if (argLen >= strSize)
        return std::nullopt;

    char regStr[strSize];
    for (ubyte chr = 0; str[chr] != NULL; chr++)
        regStr[chr] = std::tolower(str[chr]);

    static const char registers[][strSize] = {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "r8l", "r9l", "r10l", "r11l", "r12l", "r13l", "r14l", "r15l",
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w",
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
    /*"spl", "bpl", "sil", "dil"
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
    "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15",
    "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7",
    "mmx0", "mmx1", "mmx2", "mmx3", "mmx4", "mmx5", "mmx6", "mmx7"*/
    };
    std::optional<ubyte> reg = findStringInArray(str, *registers, std::size(registers), strSize);
    if (!reg.has_value())
        return std::nullopt;

    argument arg = { (ubyte)(*reg % 16), (ubyte)(pow(2, *reg >> 4) * 8), false, REG };
    return arg;
}





#if defined(_WIN32)
#include <Windows.h>

const char relativeReferencePath[] = "..\\data\\x86reference-master\\x86reference.xml";

static fs::path getExecutablePath() {
    char pathRaw[_MAX_PATH];
    GetModuleFileNameA(NULL, pathRaw, _MAX_PATH);
    return fs::path(pathRaw);
}

#endif

#ifdef __linux__
#include <libgen.h>
#include <unistd.h>

#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif

const char relativeReferencePath[] = "../data/x86reference-master/x86reference.xml";

static fs::path getExecutablePath() {
    char pathRaw[_MAX_PATH];
    realpath(PROC_SELF_EXE, pathRaw);
    return fs::path(pathRaw);
}

#endif




inline static pugi::xml_document OpenReference() {
    fs::path exePath = getExecutablePath();
    fs::path referencePath = exePath.parent_path().parent_path().parent_path() / relativeReferencePath;

    pugi::xml_document x86reference;
    pugi::xml_parse_result result = x86reference.load_file(referencePath.c_str());
    if (result.status)
        Error((char*)referencePath.string().c_str(), result.description());

    return x86reference;
}





inline static COFF_Relocation* CreateReloc(SymbolData* const symtab, const SymbolData* symtabEnd) {
    ErrorData errorData = {};
    srcFile.seekg(0);
    char inputLine[maxLineSize];
    ushort relocCount = 0;

    do {
        errorData.incLine();

        const fpos_t startOfLine = srcFile.tellg();
        srcFile.getline(inputLine, FPOSMAX);
        if (srcFile.fail())
            ProcessInputError(inputLine, errorData);
        if (char* comment = strchr(inputLine, ';'))
            *comment = NULL;

        for (SymbolData* symbol = symtab; symbol < symtabEnd; symbol++) {
            char symName[maxLineSize];
            std::ios::iostate state = srcFile.rdstate();
            const fpos_t ret = srcFile.tellg();
            symbol->getName(symName);
            srcFile.clear(state);
            if (state != 1)
                srcFile.seekg(ret);

            std::cmatch matchResult;
            relocCount += std::regex_search(inputLine, matchResult, std::regex("\\b" + (std::string)symName + "\\b"))
                && startOfLine + matchResult.position() != symbol->nameRef;
            symbol += symbol->isDefinedFunction();
        }
    } while (!srcFile.eof());

    currentReloc = (COFF_Relocation*)calloc(relocCount, sizeof(COFF_Relocation));
    relocEnd = currentReloc + relocCount;
    return currentReloc;
}





inline static void GetArguments(argument* args, SymbolData* symtab, const SymbolData* symtabEnd, const fpos_t textPos) {
    for (sbyte i = 0; i <= 2; i++) {
        char* argstr = strtok(nullptr, instrDelimiters);
        if (argstr == nullptr)
            return;

        argument& arg = args[i];
        if (isdigit(*argstr) || *argstr == '-') {
            arg.addr = IMM;
            if (*argstr == '-')
                arg.negative = true;
            arg.mutableSize = true;
            arg.val = std::stoull(argstr, nullptr, 0);
            arg.size = minBitsToStoreValue(arg.val, arg.negative);
        }
        else {
            std::optional<argument> regArg = getRegArgument(argstr);
            if (regArg.has_value()) {
                arg = regArg.value();
            }
            else {
                char* symName;
                if (char* bracket = strchr(argstr, '[')) {
                    arg.addr = MEM;
                    *bracket = NULL;
                    arg.size = (int)getSymbolBytes(argstr, errorData) * 8;
                    symName = bracket + 1;
                }
                else {
                    arg.addr = IMM;
                    symName = argstr;
                }

                if (currentReloc > relocEnd)
                    CompilerError(errorData, "Relocs don't fit into .reloc");

                if (strncmp(symName, "rel", 3) == 0) {
                    symName += 4;
                    if (char* closeBracket = strchr(symName, ']'))
                        *closeBracket = NULL;

                    SymbolData* symbol = findSymbol(symName, symtab, symtabEnd, errorData);
                    currentReloc->symbolTableIndex = symbol - symtab + requiredSymbolSpace;
                    currentReloc->type = amd_reloc_type::REL32;
                    arg.relation = RELADDR;

                    (void)strtok(nullptr, instrDelimiters);
                }
                else {
                    if (char* closeBracket = strchr(symName, ']'))
                        *closeBracket = NULL;
                    currentReloc->symbolTableIndex = findSymbol(symName, symtab, symtabEnd, errorData) - symtab + requiredSymbolSpace;
                    currentReloc->type = amd_reloc_type::ADDR64;

                    arg.relation = ABSADDR;
                }

                currentReloc++;
            }
        }
    }

    Error(errorData, "Invalid argument");
}





inline static bool IsCorrectType(argument& textArg, pugi::xml_node argNode, instruction& instr, Requirement& prevReq) {
    char type[4];
    strcpy(type, argNode.first_child().next_sibling().child_value());

    ubyte arrayI = log2(textArg.size / 8);


    static const ubyte strSize = 4;
    static const char _8[][strSize] = { "b", "bs", "bss" };
    static const ubyte _8req[std::size(_8)] = {};
    static const char _16[][strSize] = { "a", "v", "vds", "vq", "vqp", "vs", "w", "wi", "va", "wa", "wo", "ws" };
    static const ubyte _16req[std::size(_16)] = { OPERAND | DOUBLED, OPERAND, OPERAND, OPERAND, OPERAND, OPERAND, 0, 0, NOMODIF };
    static const char _32[][strSize] = { "a", "d", "di", "dqp", "ds", "p", "ptp", "sr", "v", "vds", "vqp", "vs", "va", "dqa", "da", "do" };
    static const ubyte _32req[std::size(_32)] = { NOMODIF | DOUBLED, 0, 0, 0, 0, OPERAND, OPERAND, 0, NOMODIF, NOMODIF, NOMODIF, NOMODIF, ADDRESS };
    static const char _64[][strSize] = { "dqp", "dr", "pi", "psq", "q", "qi", "qp", "vq", "vqp", "dqa", "qa", "qs"};
    static const ubyte _64req[std::size(_64)] = { REXW, 0, 0, 0, 0, 0, REXW, NOMODIF, REXW, ADDRESS };

    static const char* const operands[] = { *_8, *_16, *_32, *_64 };
    static const ubyte* const requirements[] = { _8req, _16req, _32req, _64req };
    static constexpr ubyte arraySizes[] = { std::size(_8), std::size(_16), std::size(_32), std::size(_64) };

    retry:
    const std::optional<ubyte> operandI = findStringInArray(type, operands[arrayI], arraySizes[arrayI], strSize);
    if (!operandI.has_value()) {
        if (textArg.addr == IMM && arrayI < 3) {
            arrayI++;
            goto retry;
        }
        return false;
    }

    if (textArg.addr == IMM)
        instr.immediateSize = pow(2, arrayI);


    const ubyte req = requirements[arrayI][*operandI];

    if (req & 3) {
        if ((prevReq & 7 && prevReq != req)) {
            if (textArg.addr == IMM && arrayI < 3) {
                arrayI++;
                goto retry;
            }
            else {
                return false;
            }
        }

        const ubyte prefixValues[] = { 0, operandSizePrefix, addressSizePrefix, REX.W };
        if (!memchr(instr.prefixes, req, std::size(instr.prefixes)))
            instr.addPrefix(prefixValues[req]);
    }
    else if (prevReq & 3 && req & NOMODIF) {
        if (textArg.addr == IMM && arrayI < 3) {
            arrayI++;
            goto retry;
        }
        else {
            return false;
        }
    }

    prevReq = (Requirement)req;
    return true;
}



//Returns instruction data if it has fitting arguments etc., otherwise returns nullopt
inline static std::optional<instruction> IsFittingInstruction(const pugi::xml_node pri_opcd, const pugi::xml_node syntax, argument* args) {
    instruction instr{};
    if (strcmp(pri_opcd.parent().name(), "two-byte") == 0) {
        instr.opcode[0] = 0x0F;
        instr.primaryOpcodeIndex++;
    }

    ubyte& po = instr.opcode[instr.primaryOpcodeIndex];

    po = std::stoi(pri_opcd.first_attribute().value(), nullptr, 16);
    
    ubyte textArgCounter = 0;
    for (;textArgCounter < 2 && args[textArgCounter].addr != NULL; textArgCounter++);

    instr.modrm = 0b11000000;


    if (pugi::xml_node mandatoryPref = syntax.parent().child("pref"))
        instr.addPrefix(std::stoi(mandatoryPref.child_value(), nullptr, 16));
    if (pugi::xml_node opcd_ext = syntax.parent().child("opcd_ext")) {
        instr.modrmUsed = true;
        instr.modrm |= std::stoi(opcd_ext.child_value()) << 3;
    }


    Requirement prevReq = (Requirement)0;

    ubyte entryArgCounter = 0;
    for (pugi::xml_node argNode = syntax.first_child().next_sibling();
        argNode.type() != pugi::node_null; argNode = argNode.next_sibling()) {

        argument& textArg = args[entryArgCounter];


        char addressing[4];
        if (argNode.first_child().first_child().type() == pugi::node_null) {
            if (*argNode.child_value() == 'S')
                continue;
            strcpy(addressing, argNode.child_value());
            std::optional<argument> entryArg = getRegArgument(addressing);
            if (entryArg != textArg)
                return std::nullopt;

            entryArgCounter++;
            continue;
        }

        strcpy(addressing, argNode.first_child().child_value());

        if (addressing[1] != NULL)
            continue;

        switch (textArg.addr) {
        case REG:
        {
            bool extension = 0;
            if (textArg.val >= 8)
                extension = true;

            switch (*addressing) {
            default:
                return std::nullopt;
            case 'E':
            case 'H':
            case 'R':
                instr.modrmUsed = true;
                instr.modrm |= textArg.val - (8 * extension);
                instr.rex |= REX.B * extension;
                break;
            case 'G':
                instr.modrmUsed = true;
                instr.modrm |= textArg.val - (8 * extension) << 3;
                instr.rex |= REX.R * extension;
                break;
            case 'Z':
                if (extension)
                    return std::nullopt;
                po += textArg.val;
            }
            break;
        }
        case IMM:
            if (*addressing != 'I') {
                if (*addressing != 'J' || textArg.relation != RELADDR)
                    return std::nullopt;
            }

            switch (textArg.relation) {
            case NORELATION:
                instr.immediate = textArg.val;
                break;
            case ABSADDR:
                textArg.size = 64;
                instr.reloc = (instr_reloc)((int)instr.reloc | (int)instr_reloc::IMM);
                break;
            case RELADDR:
                textArg.size = 32;
                instr.reloc = (instr_reloc)((int)instr.reloc | (int)instr_reloc::IMM);
            }
            break;
        case MEM:
            instr.modrm &= 0b00111111;
            switch (textArg.relation) {
            //case NORELATION:   TODO: Take register as address
            case ABSADDR:
                if (*addressing != 'A')
                    return std::nullopt;
                instr.dispSize = 8;
                instr.reloc = (instr_reloc)((int)instr.reloc | (int)instr_reloc::DISP);
                break;
            case RELADDR:
                if (*addressing != 'E' && *addressing != 'M')
                    return std::nullopt;
                instr.modrm |= 0b101;
                instr.dispSize = 4;
                instr.reloc = (instr_reloc)((int)instr.reloc | (int)instr_reloc::DISP);
            }
        }


        if (!IsCorrectType(textArg, argNode, instr, prevReq))
            return std::nullopt;

        entryArgCounter++;
    }

    if (textArgCounter != entryArgCounter)
        return std::nullopt;

    return instr;
}




inline static instruction FindInstruction(char* mnemonic, argument* args, const pugi::xml_node& one_byte) {
    for (ubyte chr = 0; chr < strlen(mnemonic); chr++)
        mnemonic[chr] = std::toupper(mnemonic[chr]);

    std::optional<instruction> bestInstrFound;
    ubyte bestInstrSize = -1;

    for (auto pri_opcd = one_byte.first_child(); pri_opcd.type() != pugi::node_null; pri_opcd = pri_opcd.next_sibling()) {
        for (auto entry = pri_opcd.first_child(); entry.type() != pugi::node_null; entry = entry.next_sibling()) {
            for (auto syntax = entry.child("syntax"); strcmp(syntax.name(), "syntax") == 0; syntax = syntax.next_sibling()) {
                if (strcmp(mnemonic, syntax.first_child().child_value()) != 0
                    || (*entry.attribute("mode").value() != 'e' && *entry.next_sibling().attribute("mode").value() == 'e'))
                    continue;

                std::optional<instruction> newInstruction = IsFittingInstruction(pri_opcd, syntax, args);
                if (!newInstruction.has_value())
                    continue;

                if (bestInstrFound.has_value()) {
                    if (newInstruction.value().size() < bestInstrSize) {
                        bestInstrFound = newInstruction;
                        bestInstrSize = newInstruction.value().size();
                    }
                }
                else {
                    bestInstrFound = newInstruction;
                    bestInstrSize = newInstruction.value().size();
                }
            }
        }

        if (pri_opcd.first_attribute().as_int() == 0xFF)
            pri_opcd = one_byte.next_sibling().first_child();
    }

    if (!bestInstrFound.has_value())
        Error(errorData, "No fitting opcode found. One of the arguments could be invalid for this instruction. A \"rel\" could also be missing");

    return bestInstrFound.value();
}





inline static void WriteToExe(std::ofstream& outFile, instruction instr, const fpos_t textPos) {
    outFile.write((char*)&instr.prefixes, instr.getPrefixCount());
    if (instr.rex)
        outFile.put(instr.rex);

    outFile.write((char*)&instr.opcode, instr.primaryOpcodeIndex + 1);

    if (instr.modrmUsed)
        outFile.put(instr.modrm);
    if (instr.sibUsed)
        outFile.put(instr.sib);

    if (instr.reloc == instr_reloc::BOTH)
        currentReloc[-2].virtualAddress = outFile.tellp() - textPos;
    else if (instr.reloc == instr_reloc::DISP)
        currentReloc[-1].virtualAddress = outFile.tellp() - textPos;
    const char nullstr[16] = "";
    outFile.write(nullstr, instr.dispSize);

    if ((int)instr.reloc & (int)instr_reloc::IMM)
        currentReloc[-1].virtualAddress = outFile.tellp() - textPos;
    outFile.write((char*)&instr.immediate, instr.immediateSize);
}





inline void CompileSource(SectionHeader* sections, SymbolData* const symtab, const SymbolData* symtabEnd) {
    pugi::xml_document x86reference = OpenReference();
    pugi::xml_node one_byte = x86reference.first_child().first_child();

    const fpos_t textPos = sections[TEXT].pointerToRawData;

    COFF_Relocation* reloc = CreateReloc(symtab, symtabEnd);

    errorData = {};
    srcFile.seekg(0);
    char inputLine[maxLineSize] = "";
    SymbolData* currentFunction = nullptr;
    bool inTextSection = false;

    do {
        errorData.incLine();

        srcFile.getline(inputLine, FPOSMAX);

        if (srcFile.fail())
            ProcessInputError(inputLine, errorData);

        if (strncmp(inputLine, "section", 7) == 0) {
            if (inTextSection) {
                break;
            }
            else if (strncmp(inputLine + 8, ".text", 5) == 0) {
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


        if (char* colon = strchr(inputLine, ':')) {
            *colon = NULL;
            char* formattedLine = inputLine;
            while (*formattedLine == ' ' || *formattedLine == '\t')
                formattedLine++;

            if (strchr(formattedLine, ' ')) {
                if (currentFunction)
                    Warning(errorData, "New function declared before returning previous. This could lead to undefined behaviour");

                (void)strtok(formattedLine, instrDelimiters);
                currentFunction = findSymbol(strtok(nullptr, " :"), symtab, symtabEnd, errorData);
                currentFunction->value = (int)outFile.tellp() - textPos;
            }
            else {
                findSymbol(strtok(formattedLine, " :"), symtab, symtabEnd, errorData)->value = outFile.tellp() - textPos;
            }
            continue;
        }

        char* firstToken = strtok(inputLine, instrDelimiters);
        if (strncmp(firstToken, "extern", 7) == 0)
            continue;

        instruction instr;
        argument args[2];
        GetArguments(args, symtab, symtabEnd, textPos);

        instr = FindInstruction(firstToken, args, one_byte);

        WriteToExe(outFile, instr, textPos);

        if (strcmp(firstToken, "RETN") == 0 && currentFunction) {
            AuxiliaryFunctionDefinition* aux = (AuxiliaryFunctionDefinition*)currentFunction + 1;
            aux->TotalSize = (uint32_t)outFile.tellp() - currentFunction->value - textPos;
            currentFunction = nullptr;
        }
    } while (!srcFile.eof());
    
    sections[TEXT].sizeOfRawData = (uint32_t)outFile.tellp() - textPos;
    sections[TEXT].pointerToRelocations = outFile.tellp();
    sections[TEXT].numberOfRelocations = relocEnd - reloc;
    outFile.write((char*)reloc, sections[TEXT].numberOfRelocations * sizeof(COFF_Relocation));
    free(reloc);
}