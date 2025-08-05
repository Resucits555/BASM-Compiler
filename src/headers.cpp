#include <time.h>

#include "main.h"


const uint16_t DOSData[] = { 0,1,0,4,0,0xFFFF };
uint32_t e_lfanew = 0x80;

inline static void writeMZHeader(std::ofstream& exeFile) {
    exeFile.write("MZ", 2);
    exeFile.write((char*)&DOSData, sizeof(DOSData));
    fillNullUntil(exeFile, 0x3C);
    exeFile.write((char*)&e_lfanew, 4);
}



const uint8_t DOSStub[] =
{ 0x0E, 0x1F, 0xBA, 0x0E, 00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68,
0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F,
0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20,
0x6D, 0x6F, 0x64, 0x65, 0x2E, 0x0D, 0x0D, 0x0A, 0x24 };

inline static void writeDOSStub(std::ofstream& exeFile) {
    exeFile.write((char*)&DOSStub, sizeof(DOSStub));
}



const uint32_t peMagic = 0x00004550;
const uint16_t machine = 3;
const uint16_t numberOfSections = 1;
//32b timeDateStamp
//32b pointerToSymbolTable = 0
//32b numberOfSymbols = 0
const uint16_t sizeOfOptionalHeader = 0; //MISSING
const uint16_t characteristics = 0x010F;


inline static void writeCOFFHeader(std::ofstream& exeFile) {
    fillNullUntil(exeFile, e_lfanew);

    const uint32_t PEData[] = { peMagic,  machine | (numberOfSections << 16), 0, 0, sizeOfOptionalHeader | (characteristics << 16) };

    exeFile.write((char*)&PEData[0], 8);

    int32_t epochTime = time(0);
    exeFile.write((char*)&epochTime, 4);

    exeFile.write((char*)&PEData[2], 12);
}



inline static void writeOptionalHeader(std::ofstream& exeFile) {
    fillNullUntil(exeFile, 0xC4);
}




void WriteHeaders(std::ofstream& exeFile) {
    exeFile.seekp(0);
    writeMZHeader(exeFile);
    writeDOSStub(exeFile);
    writeCOFFHeader(exeFile);

    //std::remove(exePath.c_str()); //temporary, don't want to delete it all the time
}