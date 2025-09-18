#include <optional>

namespace mrx {

    struct substr {
        size_t a;
        size_t b;

        inline size_t end() const {
            return a + b;
        }
    };


    extern bool InGroup(const char specifier, const char strChar);
    extern std::optional<substr> FindRgx(const std::string str, const char* pattern, const size_t start = 0);
    extern std::optional<unsigned char> FindRgxSectional(substr(&out_sections)[], const std::string str, const char* pattern, const char(&invalid)[] = "");
}