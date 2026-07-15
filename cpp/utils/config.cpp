#include "config.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Loads config.json exactly once (function-local static) and returns a shared,
// process-wide Config. Relative paths are resolved from the current working
// directory, so run the programs from the project root.
const Config &get_config()
{
    static Config config = []() -> Config
    {
        const std::string path = "config.json";
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open config file: " + path);
        }

        json j;
        file >> j;

        Config c;
        c.arabic_letters_path = j.at("arabic_letters_path").get<std::string>();
        c.train_dataset_path = j.at("train_dataset_path").get<std::string>();
        c.tokenize_dataset_path = j.at("tokenize_dataset_path").get<std::string>();
        c.splitted_tokens_ordered_path = j.at("splitted_tokens_ordered_path").get<std::string>();
        c.tokens_ordered_path = j.at("tokens_ordered_path").get<std::string>();
        return c;
    }();

    return config;
}
