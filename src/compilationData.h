#pragma once

//Abbreviations for type, addr and other in this code come from http://ref.x86asm.net/index.html#Instruction-Operand-Codes
//The table found at that website was copied into an xml file, which is used here as an opcode reference.

struct opcodeParts_x86 {
    ubyte po = 0;   //primary opcode
    ubyte so = 0;   //secondary
    sbyte o = -1;   //extension
};

union opcode_x86 {
    uint32_t full;
    opcodeParts_x86 parts;
};




struct argument {
    char type = 0;
    char addr = 0;
    bool negative = false;
    uintmax_t val = 0;
};




struct instruction {
    int8_t prefixes[4] = {};
    opcode_x86 opcode = {};
    int8_t modrm = 0;
    bool modrmUsed = false;
    int8_t sib = 0;
    bool sibUsed = false;
    argument arguments[2] = {};
};