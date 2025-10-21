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
    sbyte lastPrefixIndex = -1;

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