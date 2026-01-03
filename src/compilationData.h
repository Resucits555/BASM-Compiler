#pragma once


const char instrDelimiters[] = " \t,.";
const ubyte operandSizePrefix = 0x66;
const ubyte addressSizePrefix = 0x67;
struct REX_t {
    ubyte _ = 0x40; ubyte B = 0x41; ubyte X = 0x42; ubyte R = 0x44; ubyte W = 0x48;
};
const REX_t REX;





struct argument {
    char type = NULL;
    bool mutableSize = false;
    ubyte size = 0;
    bool negative = false;
    uint64_t val = 0;

    friend bool operator!=(const argument& arg1, const argument& arg2) {
        if (arg1.type == arg2.type && arg1.size == arg2.size && arg1.negative == arg2.negative && arg1.val == arg2.val)
            return false;
        else
            return true;
    }
};




class instruction {
    ubyte nextPrefixIndex = 0;
public:
    ubyte prefixes[4] = {};

    ubyte opcode[3] = {};
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

    ubyte size() const {
        return nextPrefixIndex + (primaryOpcodeIndex + 1) + modrmUsed + sibUsed + dispSize + immediateSize;
    }
};




enum requirements {
    SHRINK = 1,   //needs operand size prefix
    ADDRESS = 2,  //needs address size prefix
    REXW = 3,     //needs REXW prefix
    NOSHRINK = 4, //no size modifying allowed
    DOUBLED = 8   //see 'a' operand type
};