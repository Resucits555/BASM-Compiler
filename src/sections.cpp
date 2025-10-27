#include "main.h"
#include "sections.h"


ushort headerSize = 0x200; //Currently used as a constant. Function for calculating header size needed
ulong codeAddress = headerSize;
ulong codeSize;
ulong rdataRVA;
ulong rdataAddress;
ulong rdataSize;

ulong sizeOfFile;





inline void WriteImports(std::ofstream& exeFile) {
    rdataAddress = roundUp(exeFile.tellp(), fileAlignment);
    rdataRVA = codeRVA + sectionAlignment;
    exeFile.seekp(rdataAddress);


    const ulong exitProcessRVA = rdataRVA + 0x38;
    exeFile.write((char*)&exitProcessRVA, 4);
    exeFile.write("\0\0\0", 4);

    constexpr char kernel32Name[] = "KERNEL32.dll";
    
    IMAGE_IMPORT_DESCRIPTOR kernel32; //Values have to be changed for including more than one dll
    kernel32.OriginalFirstThunk = 0x2030;
    kernel32.TimeDateStamp = 0;
    kernel32.ForwarderChain = 0;
    kernel32.Name = 0x2046;
    kernel32.FirstThunk = rdataRVA;

    exeFile.write((char*)&kernel32, sizeof(kernel32));
    exeFile.seekp((ulong)exeFile.tellp() + sizeof(IMAGE_IMPORT_DESCRIPTOR));

    exeFile.write((char*)&exitProcessRVA, 4);
    exeFile.write("\0\0\0", 4);

    exeFile.write("\0", 2);
    const char exitProcessFunction[] = "ExitProcess";
    exeFile.write(exitProcessFunction, sizeof(exitProcessFunction));

    exeFile.write(kernel32Name, sizeof(kernel32Name));



    rdataSize = (ulong)exeFile.tellp() - rdataAddress;

    exeFile.seekp(roundUp(exeFile.tellp(), fileAlignment) - 1);
    exeFile.put(0);
    sizeOfFile = exeFile.tellp();
}