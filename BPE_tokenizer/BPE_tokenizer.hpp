#ifndef BPE_TOKENIZER_HPP
#define BPE_TOKENIZER_HPP

#include <vector>
#include <string>
#include <unordered_set>
#include "../utils/linked_list.hpp"
#include <string>
#include <list>

// Forward declaration for nlohmann::json (to avoid including json.hpp in the header)

class BPE_tokenizer
{
private:
    unordered_set<string> init_set;
    unordered_set<string> tokens;
    vector<string> tokens_added_in_order;
    vector<pair<string, string>> ordered_added_tokens_splitted;
    // unordered_map<string, vector<pair<linked_list<string>::iterator, int>>> token_to_nodes_mapper;
    unordered_map<string, map<string, list<pair<linked_list<string>::iterator, pair<int, int>>>>> tokens_pair_to_nodes_mapper;
    unordered_map<string, map<string, int>> temp_tokens_pair_freq;
    vector<linked_list<string>> sentences_in_list;
    void init_data_structures(const unordered_map<string, int> &cleaned_dataset, int maximum_number_of_words);
    void sorting_data_structures_method(const int &maximum_number_of_words, const int &iters);
    void save_tokens();
    void load_tokens();
    void add_new_token(string first_token, string seocnd_token);
    bool has_tokens();
    struct IteratorHash
    {
        size_t operator()(const linked_list<string>::iterator &it) const
        {
            return std::hash<linked_list<string>::Node *>()(it.current); // hash based on pointer identity
        }
    };

    struct IteratorEqual
    {
        bool operator()(const linked_list<string>::iterator &a, const linked_list<string>::iterator &b) const
        {
            return *a == *b;
        }
    };

public:
    BPE_tokenizer(/* args */);
    ~BPE_tokenizer();
    void train(const unordered_map<string, int> &cleaned_dataset, int iters, int maximum_number_of_words);
    vector<string> tokenize(string sentences, int maximum_number_of_words);
    string detokenize(vector<string> sentences);
    std::unordered_set<std::string> load_init_tokens_set(const std::string &path);
    std::unordered_set<std::string> get_tokens();
    std::vector<std::string> get_tokens_added_in_order();
    vector<pair<string, string>> get_splitted_tokens_added_in_order();
};

#endif // BPE_TOKENIZER_HPP
