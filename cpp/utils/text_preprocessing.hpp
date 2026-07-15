#ifndef TEXT_UTILS_HPP
#define TEXT_UTILS_HPP

#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>

// Declare your function so others can use it
std::string clean_text(const std::string &text, const std::unordered_set<std::string> &arabic_letters);
std::vector<std::string> clean_text(const std::vector<std::string> &dataset, std::unordered_set<std::string> arabic_letters);

// Number of bytes in a UTF-8 character given its lead byte. `inline` + in-header
// so it can be inlined at every call site (it runs once per byte, in hot loops).
inline std::size_t utf8_char_length(unsigned char c)
{
    if ((c & 0xF8) == 0xF0)
        return 4;
    else if ((c & 0xF0) == 0xE0)
        return 3;
    else if ((c & 0xE0) == 0xC0)
        return 2;
    else
        return 1;
}

#endif // TEXT_UTILS_HPP