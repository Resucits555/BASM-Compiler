#pragma once


const char instrDelimiters[] = " \t,.";





struct argument {
    char type = NULL;
    bool mutableSize = false;
    ubyte size = 0;
    bool negative = false;
    uint64_t val = 0;
};




class instruction {
    ubyte nextPrefixIndex = 0;
public:
    ubyte prefixes[4] = {};

    ubyte opcode[3] = {}; //an opcode can be up to 3 bytes in size
    ubyte primaryOpcodeIndex = 0;

    ubyte modrm = 0;
    bool modrmUsed = false;
    ubyte sib = 0;
    bool sibUsed = false;

    ubyte dispSize = 0;
    ubyte immediateSize = 0;
    uint64_t immediate = 0;

    void addPrefix(const ubyte& prefix) {
        prefixes[nextPrefixIndex] = prefix;
        nextPrefixIndex++;
    }

    sbyte getNextPrefixIndex() const {
        return nextPrefixIndex;
    }
};




enum requirements {
    SHRINK = 1,   //needs operand size prefix
    ADDRESS = 2,  //needs address size prefix
    REXW = 3,     //needs REXW prefix
    NOSHRINK = 4, //no size modifying allowed
    DOUBLED = 8   //see 'a' operand type
};