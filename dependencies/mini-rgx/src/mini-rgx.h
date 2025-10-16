#include <optional>

namespace mrx {

    struct substr {
        unsigned long a;
        unsigned long b;

        inline int end() const {
            return a + b;
        }
    };

    //Check if a character is a part of a group.
    extern bool CharInGroup(const char specifier, const char strChar);

    //Find the first character in a string that is part of the specified group. a = letter, d = digit, w = previous both, s = space, p = punctuation, . = any.
    //Capitalization negates the result.
    extern std::optional<unsigned long> FindCharOfGroup(const char groupSpecifier, const std::string str);

    //Find a certain sequence of characters with a regex, returns it's starting position and lenght.
    extern std::optional<substr> FindRgx(const std::string string, const char* pattern, const unsigned short start = 0);

    //Find a certain sequence of characters with a regex, returns the first matching substring as a string.
    extern std::optional<std::string> FindRgxSubstr(const std::string string, const char* pattern, const unsigned short start = 0);

    //FindRgx, with sections or "tokens". See the documentation for more.
    extern unsigned char FindRgxSectional(substr(&out_sections)[], const std::string str, const char* pattern, const char(&invalid)[] = "");
}