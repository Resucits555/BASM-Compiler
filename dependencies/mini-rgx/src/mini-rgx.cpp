#include <iostream>
#include <string>

#include "mini-rgx.h"


typedef std::uint_fast8_t ubyte;
typedef std::int_fast8_t sbyte;
typedef std::uint_fast16_t ushort;
typedef std::uint_fast32_t ulong;


namespace mrx {
    bool CharInGroup(char specifier, const char strChar) {
        const char alphabet[] = "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ";
        const char numbers[] = "0123456789";
        const char alphanum[] = "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ0123456789";
        const char spaces[] = " \t\v";
        const char punctuation[] = ".,:;";
        const char groupSpecifiers[] = "adwspADWSP";

        const char* const groups[] = { alphabet, numbers, alphanum, spaces, punctuation, groupSpecifiers };

        bool negate = false;
        if (specifier >= 'A' && specifier <= 'Z') {
            specifier += 0x20;
            negate = true;
        }


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
        case '.':
            groupI = 5;
            break;
        default:
            std::cerr << "mini-rgx.cpp, " << __LINE__ << ": Invalid specifier '" << std::to_string(specifier) << "'.";
            exit(-1);
        }

        const char* group = groups[groupI];

        for (int i = 0; group[i] != 0; i++) {
            if (group[i] == strChar)
                return !negate;
        }

        return negate;
    }





    std::optional<unsigned long> FindCharOfGroup(const char group, const std::string str) {
        for (ulong i = 0; str[i] != 0; i++) {
            if (CharInGroup(group, str[i]))
                return i;
        }

        return std::nullopt;
    }





    std::optional<substr> FindRgx(const std::string str, const char* pattern, const unsigned short start) {
        ushort strI = start - 1;
        ushort strIstart = 0;

        const ushort patternLen = strlen(pattern);
        const ushort strLen = str.length();
        for (ushort patI = 0; patI < patternLen; ++patI) {
            ++strI;

            if (!patI)
                strIstart = strI;

            if (strI >= strLen)
                return std::nullopt;

            if (pattern[patI] == '%') {
                const char& specifier = pattern[++patI];
                char group = pattern[patI];


                if (specifier == '%')
                    goto charMatch;
                else if (!CharInGroup('.', pattern[patI]))
                    continue;


                switch (pattern[++patI]) {
                case '+':
                    if (!CharInGroup(group, str[strI])) {
                        patI = -1;
                        break;
                    }

                    while (strI < strLen && CharInGroup(group, str[strI + 1]))
                        ++strI;
                    break;
                case '*':
                    while (strI < strLen && CharInGroup(group, str[strI + 1]))
                        ++strI;
                    break;
                case '?':
                    if (CharInGroup(group, str[strI]))
                        ++strI;
                    break;
                default:
                    if (!CharInGroup(group, str[strI]))
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





    std::optional<std::string> FindRgxSubstr(const std::string str, const char* pattern, const unsigned short start) {
        auto sub = FindRgx(str, pattern, start);
        if (!sub.has_value())
            return std::nullopt;

        return str.substr(sub.value().a, sub.value().b);
    }





    unsigned char FindRgxSectional(substr(&out_sections)[], const std::string str, const char* pattern, const char(&invalid)[]) {
        ushort strI = -1;
        ushort strIstart = 0;

        ubyte section = 1;

        const ushort patternLen = strlen(pattern);
        const ushort strLen = str.length();
        for (ushort patI = 0; patI < patternLen; ++patI) {
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


                if (specifier == '%')
                    goto charMatch;
                else if (!CharInGroup('.', pattern[patI])) {
                    if (CharInGroup('d', specifier)) {
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
                    if (!CharInGroup(group, str[strI]))
                        return section;

                    while (strI < strLen && CharInGroup(group, str[strI + 1]))
                        ++strI;
                    break;
                case '*':
                    while (strI < strLen && CharInGroup(group, str[strI + 1]))
                        ++strI;
                    break;
                case '?':
                    if (CharInGroup(group, str[strI]))
                        ++strI;
                    break;
                default:
                    if (!CharInGroup(group, str[strI]))
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
                if (!(str[strI] == pattern[patI]) && section)
                    return section;
            }
        }

        if (section)
            out_sections[section - 1] = { strIstart, ++strI - strIstart };
        return 0;
    }
}