#include "utils.hpp"

#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

std::unordered_set<std::string> load_arabic_letters(const std::string &path)
{
    using json = nlohmann::json;
    std::unordered_set<std::string> letters;

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open file " << path << std::endl;
        return letters;
    }

    json content;
    try
    {
        file >> content;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return letters;
    }

    auto letters_it = content.find("letters");
    if (letters_it != content.end() && letters_it->is_array())
    {
        for (const auto &letter_json : *letters_it)
        {
            if (letter_json.is_string())
            {
                letters.insert(letter_json.get<std::string>()); // store the UTF-8 string directly
            }
        }
    }
    else
    {
        std::cerr << "JSON does not contain 'letters' array." << std::endl;
    }

    return letters;
}
