#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config
{
    std::string arabic_letters_path;
    std::string train_dataset_path;
    std::string tokenize_dataset_path;
    std::string splitted_tokens_ordered_path;
    std::string tokens_ordered_path;
};

// Returns the process-wide configuration, loaded from config.json on first call.
const Config &get_config();

#endif // CONFIG_HPP
