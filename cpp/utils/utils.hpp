#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>

// Splits a string on runs of whitespace. `inline` so it can live in a header
// and be included by multiple translation units without ODR violations.
inline std::vector<std::string> split_by_space(const std::string &input)
{
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string word;

    while (iss >> word)
    {
        tokens.push_back(word);
    }

    return tokens;
}

// Loads a JSON file shaped like {"letters": ["ا", "ب", ...]} into a set of
// UTF-8 strings. Defined in utils.cpp (keeps the heavy nlohmann/json include
// out of this header). Returns an empty set on any failure.
std::unordered_set<std::string> load_arabic_letters(const std::string &path);

#endif // UTILS_HPP
