#pragma once


const char instrDelimiters[] = " \t,.";
const ubyte operandSizePrefix = 0x66;
const ubyte addressSizePrefix = 0x67;
struct REX_t {
    ubyte _ = 0x40; ubyte B = 0x41; ubyte X = 0x42; ubyte R = 0x44; ubyte W = 0x48;
};
const REX_t REX;





enum addressing : ubyte {
    NOARG,
    REG,
    MEM,
    IMM
};



enum addressRelation : ubyte {
    NORELATION,
    ABSADDR,
    RELADDR
};




struct argument {
    uint64_t val = 0;
    //Size of final variable. Like in [rax], this would represent the size of the referenced address
    ushort size = 0;
    bool mutableSize = false;
    enum addressing addr = NOARG;
    enum addressRelation relation = NORELATION;
    bool negative = false;

    friend bool operator!=(const argument& arg1, const argument& arg2) {
        if (arg1.addr == arg2.addr && arg1.size == arg2.size && arg1.negative == arg2.negative && arg1.val == arg2.val)
            return false;
        else
            return true;
    }
};




struct Modrm {
    ubyte mod = 0b11;
    ubyte reg = 0;
    ubyte rm = 0;

    void write(std::ofstream& outFile) const {
        ubyte modrm;
        modrm = (mod << 6) | (reg << 3) | rm;
        outFile.put(modrm);
    }
};



struct SIB {
    ubyte scale = 0;
    ubyte index = 0;
    ubyte base = 0;

    void write(std::ofstream& outFile) const {
        ubyte sib;
        sib = (scale << 6) | (index << 3) | base;
        outFile.put(sib);
    }
};



enum class instr_reloc : ubyte {
    NONE,
    DISP,
    IMM,
    BOTH
};



class instruction {
public:
    ubyte prefixes[4] = {};
    ubyte rex = 0;

    ubyte opcode[3] = {};
    ubyte primaryOpcodeIndex = 0;

    Modrm modrm = {};
    bool modrmUsed = false;
    SIB sib = {};
    bool sibUsed = false;

    instr_reloc reloc = instr_reloc::NONE;
    ubyte dispSize = 0;
    ubyte immediateSize = 0;
    uint64_t immediate = 0;

    ubyte getPrefixCount() const {
        ubyte prefixI = 0;
        while(prefixes[prefixI] != 0)
            prefixI++;
        return prefixI;
    }

    void addPrefix(const ubyte prefix) {
        if (prefix >= 0x40 && prefix < 0x50)
            rex |= prefix;
        else
            prefixes[getPrefixCount()] = prefix;

        if (getPrefixCount() + (bool)rex > 4)
            CompilerError("Prefix overflow");
    }

    ubyte size() const {
        return getPrefixCount() + (primaryOpcodeIndex + 1) + modrmUsed + sibUsed + dispSize + immediateSize;
    }
};




enum Requirement : ubyte {
    OPERAND = 1,  //needs operand size prefix
    ADDRESS = 2,  //needs address size prefix
    REXW = 3,     //needs REX.W prefix
    NOMODIF = 4,  //no size modifying allowed
    DOUBLED = 8   //see 'a' operand type
};