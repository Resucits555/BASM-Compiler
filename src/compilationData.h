#pragma once

//Abbreviations in this code come from http://ref.x86asm.net/index.html#Instruction-Operand-Codes
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
    ubyte size = 0;
    bool negative = false;
    uintmax_t val = 0;
};




struct instruction {
    std::vector<ubyte> prefixes = {};
    opcode_x86 opcode = {};
    ubyte modrm = 0;
    bool modrmUsed = false;
    ubyte sib = 0;
    bool sibUsed = false;
    ubyte dataSize = 0;
    uintmax_t data = 0; //addresses and immediate values
};