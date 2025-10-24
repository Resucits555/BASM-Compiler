#pragma once

//Abbreviations in this code come from http://ref.x86asm.net/index.html#Instruction-Operand-Codes
//The table found at that website was copied into an xml file, which is used here as an opcode reference.
//https://wiki.osdev.org/X86-64_Instruction_Encoding is the primary source of information for this project.


struct argument {
    char type = NULL;
    bool variableSize = false;
    ubyte size = 0;
    bool negative = false;
    uintmax_t val = 0;
};




struct instruction {
    ubyte prefixes[4] = {};
    ubyte lastPrefixIndex = 0;

    ubyte opcode[3] = {}; //an opcode can be up to 3 bytes in size
    ubyte primaryOpcodeIndex = 0;

    ubyte modrm = 0;
    bool modrmUsed = false;
    ubyte sib = 0;
    bool sibUsed = false;

    ubyte dataSize = 0;
    ubyte immediateSize = 0;
    uint64_t data = 0; //used for displacement values and direct addresses
    uint64_t immediate = 0;
};




enum requirements {
    SHRINK = 1,
    ADDRESS, //shrink with the address-size prefix instead of the operand-size prefix
    REXW,
    NOSHRINK, //both address and operand prefixes
    DOUBLED = 8 //only 'a' type
};



constexpr char _8[][4] = { "b", "bs", "bss" };
constexpr ubyte _8req[3] = {};
constexpr char _16[][4] = { "a", "v", "vds", "vq", "vqp", "vs", "w", "wi", "va", "wa", "wo", "ws" };
constexpr ubyte _16req[12] = { SHRINK | DOUBLED, SHRINK, SHRINK, SHRINK, SHRINK, SHRINK, 0, 0, NOSHRINK };
constexpr char _32[][4] = { "a", "d", "di", "dqp", "ds", "p", "ptp", "sr", "v", "vds", "vqp", "vs", "va", "dqa", "da", "do" };
constexpr ubyte _32req[16] = { NOSHRINK | DOUBLED, 0, 0, 0, 0, SHRINK, SHRINK, 0, NOSHRINK, NOSHRINK, NOSHRINK, NOSHRINK, ADDRESS };
constexpr char _64[][4] = { "dqp", "dr", "pi", "psq", "q", "qi", "qp", "vqp", "dqa", "qa", "qs" };
constexpr ubyte _64req[11] = { REXW, 0, 0, 0, 0, 0, REXW, REXW, ADDRESS };

const char* const operands[] = { *_8, *_16, *_32, *_64 };
const ubyte* const reqArrays[] = {_8req, _16req, _32req, _64req};
constexpr ubyte arraySizes[] = { 3, 12, 16, 11 };