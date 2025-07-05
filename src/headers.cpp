#include <chrono>

#include "main.h"


static std::string cutLastInPath(std::string path) {
    ubyte end = path.length();
    for (end; end > 0; end--) {
        if (path[end] == '/')
            break;
    }
    path = path.substr(0, end);
    return path;
}



const char nulls[50] = "";
constexpr ubyte nullsSize = sizeof(nulls);

static void fillNullUntil(std::ofstream& file, int until) {
    int toFill = until - file.tellp();

    for (toFill; toFill >= nullsSize; toFill -= nullsSize)
        file.write(nulls, nullsSize);
    file.write(nulls, toFill);
}




static void writeDOSHeader();
static void writeDOSStub();
static void writeCOFFHeader();


std::ofstream executableFile;

void WriteHeaders(char* sourceFilePath) {
    std::string executablePath = cutLastInPath(sourceFilePath).append("/a.exe");

    executableFile.open(executablePath, std::ios::binary);
    if (!executableFile.is_open()) {
        Error("Failed to create executable file");
    }

    writeDOSHeader();
    writeDOSStub();
    writeCOFFHeader();

    executableFile.close();
    std::remove(executablePath.c_str()); //temporary, don't want to delete it all the time
}



const uint16_t DOSData[] = { 0,1,0,4,0,0xFFFF };
uint32_t e_Ifanew = 0x90;

static void writeDOSHeader() {
    executableFile.write("MZ", 2);
    executableFile.write((char*)&DOSData, sizeof(DOSData));
    fillNullUntil(executableFile, 0x3C);
    executableFile.write((char*)&e_Ifanew, 4);
}



const uint8_t DOSStub[] =
{ 0x0E, 0x1F, 0xBA, 0x0E, 00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68,
0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F,
0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20,
0x6D, 0x6F, 0x64, 0x65, 0x2E, 0x0D, 0x0D, 0x0A, 0x24 };

static void writeDOSStub() {
    executableFile.write((char*)&DOSStub, sizeof(DOSStub));
    fillNullUntil(executableFile, 0x90);
}



const uint16_t COFFData[] = { 0x014C, 3, 0,0, 0,0, 0,0, 0x00E0, 0x010F };

static void writeCOFFHeader() {
    executableFile.write("PE\x00", 4);
    executableFile.write((char*)&COFFData, sizeof(COFFData));

    int ret = executableFile.tellp();
    executableFile.seekp(0x98);
    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
    uint32_t epochTime = std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
    executableFile.write((char*)&epochTime, 4);
    executableFile.seekp(ret);
}