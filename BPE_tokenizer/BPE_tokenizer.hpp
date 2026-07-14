#ifndef BPE_TOKENIZER_HPP
#define BPE_TOKENIZER_HPP

#include <cstddef>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <list>
#include <utility>
#include "../utils/linked_list.hpp"

// Forward declaration for nlohmann::json (to avoid including json.hpp in the header)

class BPE_tokenizer
{
private:
    std::unordered_set<std::string> init_set;
    std::unordered_set<std::string> tokens;
    std::vector<std::string> tokens_added_in_order;
    std::vector<std::pair<std::string, std::string>> ordered_added_tokens_splitted;
    std::unordered_map<std::string, std::map<std::string, std::list<std::pair<linked_list<std::string>::iterator, std::pair<int, int>>>>> tokens_pair_to_nodes_mapper;
    std::unordered_map<std::string, std::map<std::string, int>> temp_tokens_pair_freq;
    std::vector<linked_list<std::string>> sentences_in_list;
    void init_data_structures(const std::unordered_map<std::string, int> &cleaned_dataset, int maximum_number_of_words);
    void sorting_data_structures_method(const int &maximum_number_of_words, const int &iters);
    void save_tokens();
    void load_tokens();
    void add_new_token(std::string first_token, std::string seocnd_token);
    bool has_tokens();

public:
    BPE_tokenizer(/* args */);
    ~BPE_tokenizer();
    void train(const std::unordered_map<std::string, int> &cleaned_dataset, int iters, int maximum_number_of_words);
    std::vector<std::string> tokenize(std::string sentences, int maximum_number_of_words);
    std::string detokenize(std::vector<std::string> sentences);
    std::unordered_set<std::string> load_init_tokens_set(const std::string &path);
    std::unordered_set<std::string> get_tokens();
    std::vector<std::string> get_tokens_added_in_order();
    std::vector<std::pair<std::string, std::string>> get_splitted_tokens_added_in_order();
};

#endif // BPE_TOKENIZER_HPP
