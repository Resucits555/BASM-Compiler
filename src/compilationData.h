#pragma once


enum specialSpecifiers : ubyte {
    sixOptions = 1, //the mnemonic has 6 different opcodes and the operands are the same as i.e. ADD
    repeats = 2     //mnemonic with the same name after this one
};


enum explicitOperands : ubyte {
    reg = 1,
    mem = 2,
    r_m = 3,
    xmm = 4,
    imm = 5,
    rel = 6,
    moffs = 7,
    sreg = 8
};

enum implicitOperands : ubyte {
    ax = 16,
    cx = 17,
    dx = 18,
    bx = 19,
    fs = 20,
    gs = 21,
    rbp = 22,
    flags = 23,
    mreal = 24,
    st = 25
};


enum operandSizes : ushort {
    _8 = 32,
    _16 = 64,
    _32 = 128,
    _64 = 256,
    _128 = 512
};

struct opcodeParts_X86 {
    ubyte po; //primary opcode
    ubyte so; //secondary
    ubyte o;  //extension
};

union opcode_X86 {
    uint32_t full;
    opcodeParts_X86 parts;
};

struct mnemonicOpcode_X86 {
    char mnemonic[9];
    opcode_X86 opcode;
    uint16_t operand1;
    uint16_t operand2;
    uint16_t operand3;
    ubyte special;
};


/* -TO ADD A NEW MNEMONIC-
    http://ref.x86asm.net/coder64.html recommended as source
1.  Find the right place (alphabetically ordered)
2.  Mnemonic: lowercase, max. 8 chars
3.  Enter the first opcode you'll find associated with the mnemonic.
    If it has many (so, o), then add them from right to left into one number. Example:
    po = 0x10, so = 0x20, o = 4 -> 0x42010
4.  Add operand specifiers. Use | for multiple. Storage type + size. If one has 'sixOptions', just put 0.
    Implicit operands should be marked with 'imp'
5.  Add specialSpecifiers. Use | for multiple

Add 'repeats' to any mnemonic that has exactly the same name except when it's the last. E.g. 'call'
*/
const mnemonicOpcode_X86 opcodeTable[] = {
    { "adc", 0x10, 0, 0, sixOptions },
    { "add", 0x00, 0, 0, sixOptions },
    { "call", 0xE8, rel|_16|_32, 0, 0, repeats },
    { "call", 0x200FF, r_m|_16|_32|_64, 0, 0, 0 },
    { "cmp", 0x38, 0, 0, 0, sixOptions },
    { "dec", 0x100FE, r_m|_8, 0, 0, repeats },
    { "dec", 0x100FF, r_m|_16|_32|_64, 0, 0, 0 },
    { "div", 0x600F7, dx|_64, ax|_64, r_m|_16|_32|_64, 0 },
    { "idiv", 0x700F7, dx|_64, ax|_64, r_m|_16|_32|_64, 0 },
    { "imul", 0x500F7, dx|_64, ax|_64, r_m|_16|_32|_64, 0 },
    { "inc", 0x000FE, r_m|_8, 0, 0, repeats },
    { "inc", 0x000FF, r_m|_8, 0, 0, 0 },
    { "mov", 0xA0, ax|_8, moffs|_8, 0, repeats },
    { "mov", 0xA1, ax|_64, moffs|_16|_32|_64, 0, repeats },
    { "mov", 0xA2, moffs|_8, ax|_8, 0, repeats },
    { "mov", 0xA3, moffs|_16|_32|_64, ax|_64, 0, repeats },
    { "mul", 0x400F7, dx|_64, ax|_64, r_m|_16|_32|_64, 0 },
    { "nop", 0x90, 0, 0, 0, 0 },
    { "or", 0x08, sixOptions },
    { "pop", 0x0008F, r_m|_16|_32|_64, 0, 0, 0 },
    { "push", 0x600FF, r_m|_16|_32|_64, 0, 0, 0 },
    { "sub", 0x28, sixOptions }
};



struct argument {
    uint64_t type = UINT64_MAX;
    bool used = false;
    size_t val = SIZE_MAX;
};



struct operation {
    int8_t prefixes[4];
    opcode_X86 opcode;
    int8_t modrm;
    int8_t sib;
    argument arg1;
    argument arg2;
    argument arg3;
};