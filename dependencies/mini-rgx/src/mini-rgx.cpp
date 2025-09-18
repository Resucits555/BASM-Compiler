#include <iostream>
#include <string>
#include <cstring>

#include "mini-rgx.h"


typedef unsigned char ubyte;
typedef signed char sbyte;
typedef unsigned short ushort;
typedef unsigned long ulong;


namespace mrx {

    bool InGroup(const char specifier, const char strChar) {
        const char alphabet[] = "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ";
        const char numbers[] = "0123456789";
        const char alphanum[] = "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ0123456789";
        const char spaces[] = " \t\v";
        const char punctuation[] = ".,:;";
        const char groupSpecifiers[] = "adwsp.";

        const char* const groups[] = { alphabet, numbers, alphanum, spaces, punctuation, groupSpecifiers };

        ubyte groupI;

        switch (specifier) {
        case 'a':
            groupI = 0;
            break;
        case 'd':
            groupI = 1;
            break;
        case 'w':
            groupI = 2;
            break;
        case 's':
            groupI = 3;
            break;
        case 'p':
            groupI = 4;
            break;
        case 'g':
            groupI = 5;
            break;
        case '.':
            return true;
        default:
            std::cerr << "mini-rgx.cpp, " << __LINE__ << ": Invalid specifier '" << std::to_string(specifier) << "'.";
            exit(-1);
        }

        const char* group = groups[groupI];

        for (int i = 0; group[i] != 0; i++) {
            if (group[i] == strChar)
                return true;
        }

        return false;
    }





    std::optional<substr> FindRgx(const std::string str, const char* pattern, const size_t start) {
        size_t strI = start - 1;
        size_t strIstart = 0;

        const size_t patternLen = strlen(pattern);
        const size_t strLen = str.length();
        for (size_t patI = 0; patI < patternLen; ++patI) {
            ++strI;

            if (!patI)
                strIstart = strI;

            if (strI >= strLen)
                return std::nullopt;

            if (pattern[patI] == '%') {
                const char& specifier = pattern[++patI];
                char group = pattern[patI];
                bool negate = false;

                if (specifier >= 'A' && specifier <= 'Z') {
                    negate = true;
                    group += 0x20;
                }


                if (specifier == '%')
                    goto charMatch;
                else if (!InGroup('g', pattern[patI]))
                    continue;


                switch (pattern[++patI]) {
                case '+':
                    if (!(!InGroup(group, str[strI]) != !negate)) { //basically XOR
                        patI = -1;
                        break;
                    }

                    while (strI < strLen && !(InGroup(group, str[strI + 1]) != !negate))
                        ++strI;
                    break;
                default:
                    if (!(!InGroup(group, str[strI]) != !negate))
                        patI = 0;
                    --patI;
                }

                continue;
            }

            charMatch:

            if (pattern[patI + 1] != '?') {
                if (!(str[strI] == pattern[patI]))
                    patI = -1;
            }
            else {
                if (!(str[strI] == pattern[patI]))
                    --strI;
                ++patI;
            }

        }

        return substr(strIstart, ++strI - strIstart);
    }





    std::optional<unsigned char> FindRgxSectional(substr(&out_sections)[], const std::string str, const char* pattern, const char(&invalid)[]) {
        size_t strI = -1;
        size_t strIstart = 0;

        ubyte section = 1;

        const size_t patternLen = strlen(pattern);
        const size_t strLen = str.length();
        for (size_t patI = 0; patI < patternLen; ++patI) {
            ++strI;
            
            if (strI >= strLen)
                return section;

            if (strchr(invalid, str[strI])) {
                if (section)
                    out_sections[section - 1] = { strIstart, strI - strIstart };
                section = 0;
            }


            if (pattern[patI] == '%') {
                const char& specifier = pattern[++patI];
                char group = pattern[patI];
                bool negate = false;

                if (specifier >= 'A' && specifier <= 'Z') {
                    negate = true;
                    group += 0x20;
                }


                if (specifier == '%')
                    goto charMatch;
                else if (!InGroup('g', pattern[patI])) {
                    if (InGroup('d', specifier)) {
                        if (section)
                            out_sections[section - 1] = { strIstart, strI - strIstart };

                        char s[1] = { pattern[patI] };
                        section = std::stoi(s);
                        
                        strIstart = strI;
                        --strI;
                    }

                    continue;
                }


                switch (pattern[++patI]) {
                case '+':
                    if (InGroup(group, str[strI]) == negate) // = Group XNOR negate
                        return section;

                    while (strI < strLen && InGroup(group, str[strI + 1]) != negate) //XOR
                        ++strI;
                    break;
                case '*':
                    while (strI < strLen && InGroup(group, str[strI + 1]) != negate)
                        ++strI;
                    break;
                case '?':
                    if (InGroup(group, str[strI]) == negate)
                        --strI;
                    break;
                default:
                    if (InGroup(group, str[strI]) == negate)
                        return section;
                    --patI;
                }

                continue;
            }

            charMatch:

            if (pattern[patI + 1] == '?') {
                if (!(str[strI] == pattern[patI]))
                    --strI;
                ++patI;
            }
            else {
                if (!(str[strI] == pattern[patI]))
                    return section;
            }
        }

        if (section)
            out_sections[section - 1] = { strIstart, ++strI - strIstart };
        return {};
    }
}