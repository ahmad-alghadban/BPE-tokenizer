#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "../utils/text_preprocessing.hpp"
#include "../utils/utils.hpp"
#include "../utils/config.hpp"
#include "../utils/errors.hpp"
#include <set>
#include "BPE_tokenizer.hpp"

using namespace std;
using json = nlohmann::json;

BPE_tokenizer::BPE_tokenizer()
{
    this->init_set = unordered_set<string>();
}

BPE_tokenizer::~BPE_tokenizer()
{
}

unordered_set<string> BPE_tokenizer::get_tokens()
{
    return tokens;
}

vector<string> BPE_tokenizer::get_tokens_added_in_order()
{
    return tokens_added_in_order;
}

vector<pair<string, string>> BPE_tokenizer::get_splitted_tokens_added_in_order()
{
    return ordered_added_tokens_splitted;
}

bool BPE_tokenizer::has_tokens()
{
    return !ordered_added_tokens_splitted.empty();
}

void BPE_tokenizer::add_new_token(string first_token, string seocnd_token)
{
    string new_token = first_token + seocnd_token;
    tokens.insert(new_token);
    tokens_added_in_order.push_back(new_token);
    ordered_added_tokens_splitted.push_back({first_token, seocnd_token});
}

void BPE_tokenizer::save_tokens()
{
    ofstream out_file(get_config().splitted_tokens_ordered_path);
    if (!out_file.is_open())
    {
        cerr << "Could not open output file!" << endl;
        return;
    }

    for (const auto &[first_tokem, second_token] : get_splitted_tokens_added_in_order())
    {
        out_file << first_tokem << " " << second_token << endl;
    }

    out_file.close();

    ofstream out_file1(get_config().tokens_ordered_path);
    if (!out_file1.is_open())
    {
        cerr << "Could not open output file!" << endl;
        return;
    }

    for (const auto &token : get_tokens_added_in_order())
    {
        out_file1 << token << endl;
    }

    out_file1.close();
}

void BPE_tokenizer::load_tokens()
{
    if (has_tokens())
        return; // tokens already loaded; don't append them a second time
    ifstream in_file(get_config().splitted_tokens_ordered_path);
    if (!in_file.is_open())
    {
        cerr << "Could not open input file!" << endl;
        return;
    }

    string token;
    while (getline(in_file, token))
    {
        vector<string> token_splitted = split_by_space(token);
        if ((int)token_splitted.size() != 2)
            throw ParseError("load_tokens: expected 2 space-separated tokens per line, got " +
                             std::to_string(token_splitted.size()) + " in line: \"" + token + "\"");
        add_new_token(token_splitted[0], token_splitted[1]);
    }

    in_file.close();
}

void BPE_tokenizer::init_data_structures(const unordered_map<string, int> &cleaned_dataset, int maximum_number_of_words)
{

    // unordered_map<string, vector<pair<linked_list<string>::iterator, int>>>().swap(token_to_nodes_mapper);
    unordered_map<string, map<string, list<pair<linked_list<string>::iterator, pair<int, int>>>>>().swap(tokens_pair_to_nodes_mapper);
    unordered_map<string, map<string, int>>().swap(temp_tokens_pair_freq);
    vector<linked_list<string>>().swap(sentences_in_list);

    sentences_in_list.reserve(maximum_number_of_words);

    for (const auto &[line, freqs] : cleaned_dataset)
    {
        // list<string> temp_list;
        string previous = "-1";
        sentences_in_list.push_back(linked_list<string>());
        for (int i = 0; i < line.size();)
        {
            unsigned char c = line[i];
            size_t len = utf8_char_length(c);
            string utf8_char = line.substr(i, len);
            if (utf8_char == " ")
            {
                sentences_in_list.emplace_back();
            }
            else
            {
                if (i == 0 || previous == " ")
                {
                    linked_list<string>::iterator added_node = sentences_in_list.back().insert(sentences_in_list.back().end(), utf8_char);
                    assert(utf8_char == *added_node);
                    // token_to_nodes_mapper[utf8_char].push_back({added_node, (int)sentences_in_list.size() - 1});
                }
                else
                {
                    linked_list<string>::iterator added_node = sentences_in_list.back().insert(sentences_in_list.back().end(), utf8_char);
                    // token_to_nodes_mapper[utf8_char].push_back({added_node, (int)sentences_in_list.size() - 1});
                    assert(utf8_char == *added_node);
                    --added_node;
                    assert(previous == *added_node);
                    temp_tokens_pair_freq[previous][utf8_char] += freqs;
                    tokens_pair_to_nodes_mapper[previous][utf8_char].push_back({added_node, {(int)sentences_in_list.size() - 1, freqs}});
                }
            }
            i += len;
            previous = utf8_char;
        }
        if (!sentences_in_list.back().empty())
        {
            sentences_in_list.push_back(linked_list<string>());
        }
    }

    if (sentences_in_list.back().empty())
        sentences_in_list.pop_back();
}

// void BPE_tokenizer::iterate_over_frequencies_method(const int &maximum_number_of_words, const int &iters, const int &maximum_word_freq){
//     assert(false, 'The function is not ready yet');
//     vector<list<pair<string, string>>> freqs_map(maximum_word_freq+1);
//     // unordered_map<string, map<string, vector<pair<linked_list<string>::iterator, int>>>> tokens_pair_to_nodes_mapper;
//     for(auto [first_token, second_token_map] : tokens_pair_to_nodes_mapper){
//         for(auto [second_token, two_tokens_vector] : second_token_map){
//             assert((int)two_tokens_vector.size() <= maximum_word_freq);
//             freqs_map[(int)two_tokens_vector.size()].push_back({first_token, second_token});
//         }
//     }

//     for(int i = maximum_word_freq, cycle = 0; i > 0 && cycle < iters; i--){
//         for(auto [first_token, second_token] : freqs_map[i]){
//             if(cycle >= iters) break;
//             cycle++;

//         }
//     }

// }

void BPE_tokenizer::sorting_data_structures_method(const int &maximum_number_of_words, const int &iters)
{

    struct PairHash
    {
        size_t operator()(const pair<string, string> &p) const
        {
            // Hash both strings and combine
            size_t h1 = std::hash<string>{}(p.first);
            size_t h2 = std::hash<string>{}(p.second);

            // A common way to combine hashes (boost::hash_combine)
            return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };

    struct PairEqual
    {
        bool operator()(const pair<string, string> &a, const pair<string, string> &b) const
        {
            return a.first == b.first && a.second == b.second;
        }
    };

    unordered_map<pair<string, string>, int, PairHash, PairEqual> freq_tokens_map;
    // map<pair<string, string>, int> freq_tokens_map;
    freq_tokens_map.reserve(maximum_number_of_words);
    set<pair<int, pair<string, string>>> freq_tokens_sorted; // sorted by frequency ascending

    for (const auto &[first_token, second_token_map] : tokens_pair_to_nodes_mapper)
    {
        for (const auto &[second_token, two_tokens_vector] : second_token_map)
        {
            pair<string, string> pair_token(first_token, second_token);
            int freq = temp_tokens_pair_freq[first_token][second_token];
            freq_tokens_map[pair_token] = freq;
            freq_tokens_sorted.insert({freq, pair_token});
        }
    }

    auto update_data_structures = [&](const linked_list<string>::iterator &first_node, const linked_list<string>::iterator &second_node, const int update_value)
    {
        pair<string, string> pair_token = {*first_node, *second_node};
        int old_freq = freq_tokens_map[pair_token];
        if (old_freq > 0)
        {
            freq_tokens_sorted.erase({old_freq, pair_token});
        }
        freq_tokens_map[pair_token] += update_value;
        if (freq_tokens_map[pair_token] > 0)
        {
            freq_tokens_sorted.insert({freq_tokens_map[pair_token], pair_token});
        }
    };

    for (int cycle = 0; cycle < iters; cycle++)
    {

        if (cycle % (iters / 100) == 0)
        {
            cout << "Done: " << cycle / (iters / 100) << "\n";
        }

        if (freq_tokens_sorted.empty())
        {
            cout << "The whole dataset was tokenized before the end of iterations\n";
            cout << "The total number of iterations is: " << cycle << endl;
            break;
        }

        auto last = *freq_tokens_sorted.rbegin(); // max freq
        int num_of_occ = last.first;
        string first_token = last.second.first, second_token = last.second.second;
        freq_tokens_sorted.erase(--freq_tokens_sorted.end());
        tokens.insert(first_token + second_token);
        add_new_token(first_token, second_token);
        int not_deleted = 0;
        auto &pair_nodes = tokens_pair_to_nodes_mapper[first_token][second_token];
        int num_deleted_nodes = 0;

        for (auto &p : pair_nodes)
        {
            auto node = p.first;
            int idx = p.second.first;
            int freqs = p.second.second;

            if (node.is_deleted())
            {
                num_deleted_nodes++;
                continue;
            }

            auto next_node = node;
            ++next_node;
            if (next_node == sentences_in_list[idx].end() || next_node.is_deleted())
            {
                assert(false);
                continue;
            }

            if (*next_node != second_token)
            {
                num_deleted_nodes++;
                continue;
            }

            assert(*node == first_token && *next_node == second_token);

            not_deleted++;
            if (node != sentences_in_list[idx].begin())
            {
                auto prev_node = node;
                prev_node--;
                update_data_structures(prev_node, node, -freqs);
            }

            update_data_structures(node, next_node, -freqs);

            if (next_node != sentences_in_list[idx].back_iter())
            {
                auto next_next_node = next_node;
                next_next_node++;
                update_data_structures(next_node, next_next_node, -freqs);
            }

            string merged = first_token + second_token;
            auto next_next_node = next_node;
            next_next_node++;
            sentences_in_list[idx].erase(node);     // marks node->deleted
            sentences_in_list[idx].erase(next_node); // marks next_node->deleted
            node = sentences_in_list[idx].insert(next_next_node, merged);

            if (node != sentences_in_list[idx].begin())
            {
                auto prev_node = node;
                prev_node--;
                update_data_structures(prev_node, node, +freqs);
                // token_to_nodes_mapper[*prev_node].push_back({prev_node, idx});
                tokens_pair_to_nodes_mapper[*prev_node][*node].push_back({prev_node, {idx, freqs}});
            }

            if (node != sentences_in_list[idx].back_iter())
            {
                auto next_node = node;
                next_node++;
                update_data_structures(node, next_node, +freqs);
                // token_to_nodes_mapper[*node].push_back({node, idx});
                tokens_pair_to_nodes_mapper[*node][*next_node].push_back({node, {idx, freqs}});
            }
        }
        assert((int)pair_nodes.size() == not_deleted + num_deleted_nodes);
        // vector<std::pair<linked_list<std::string>::iterator, int>>().swap(pair_nodes);
    }
}

void BPE_tokenizer::train(const unordered_map<string, int> &cleaned_dataset, int iters, int maximum_number_of_words)
{

    // vector<string> cleaned_dataset = clean_text(dataset);

    this->init_set = this->load_init_tokens_set(get_config().arabic_letters_path);
    tokens = this->init_set;

    init_data_structures(cleaned_dataset, maximum_number_of_words);
    sorting_data_structures_method(maximum_number_of_words, iters);
    save_tokens();
    cout << "The end of the training function\n";
}

unordered_set<string> BPE_tokenizer::load_init_tokens_set(const string &path)
{
    // Delegates to the shared loader so the JSON-parsing logic lives in one place.
    return load_arabic_letters(path);
}

vector<string> BPE_tokenizer::tokenize(string sentences, int maximum_number_of_words)
{
    if (!has_tokens())
        load_tokens();
    vector<linked_list<string>> sentences_in_list;
    sentences_in_list.reserve(maximum_number_of_words);
    unordered_map<string, map<string, list<pair<linked_list<string>::iterator, int>>>> tokens_pair_to_nodes_mapper;

    int estimated_number_of_resulting_tokens = sentences.size();

    // for (string line : sentences)
    // {
        sentences_in_list.emplace_back();
        string previous = "-1";
        for (int i = 0; i < sentences.size();)
        {
            unsigned char c = sentences[i];
            size_t len = utf8_char_length(c);
            string utf8_char = sentences.substr(i, len);
            if (utf8_char == " ")
            {
                sentences_in_list.emplace_back();
            }
            else
            {
                if (i == 0 || previous == " ")
                {
                    linked_list<string>::iterator added_node = sentences_in_list.back().insert(sentences_in_list.back().end(), utf8_char);
                    estimated_number_of_resulting_tokens++;
                }
                else
                {
                    linked_list<string>::iterator added_node = sentences_in_list.back().insert(sentences_in_list.back().end(), utf8_char);
                    estimated_number_of_resulting_tokens++;
                    --added_node;
                    tokens_pair_to_nodes_mapper[previous][utf8_char].push_back({added_node, (int)sentences_in_list.size() - 1});
                }
            }
            i += len;
            previous = utf8_char;
        }
        if (!sentences_in_list.back().empty())
        {
            // sentences_in_list.push_back(linked_list<string>());
            sentences_in_list.emplace_back();
        }
    // }

    if (sentences_in_list.back().empty())
        sentences_in_list.pop_back();

    for (const auto &[first_token, second_token] : ordered_added_tokens_splitted){
        auto &pair_nodes = tokens_pair_to_nodes_mapper[first_token][second_token];
        for (auto &p : pair_nodes)
        {
            auto node = p.first;
            int idx = p.second;

            if (node.is_deleted())
            {
                continue;
            }

            auto next_node = node;
            ++next_node;
            if (next_node == sentences_in_list[idx].end() || next_node.is_deleted())
            {
                continue;
            }

            if (*next_node != second_token)
            {
                continue;
            }

            assert(*node == first_token && *next_node == second_token);

            string merged = first_token + second_token;
            auto next_next_node = next_node;
            next_next_node++;
            sentences_in_list[idx].erase(node);     // marks node->deleted
            sentences_in_list[idx].erase(next_node); // marks next_node->deleted
            node = sentences_in_list[idx].insert(next_next_node, merged);
            estimated_number_of_resulting_tokens--;

            if (node != sentences_in_list[idx].begin())
            {
                auto prev_node = node;
                prev_node--;
                tokens_pair_to_nodes_mapper[*prev_node][*node].push_back({prev_node, idx});
            }

            if (node != sentences_in_list[idx].back_iter())
            {
                auto next_node = node;
                next_node++;
                tokens_pair_to_nodes_mapper[*node][*next_node].push_back({node, idx});
            }
        }
    }

    vector<string> result;
    result.reserve(estimated_number_of_resulting_tokens);
    for (size_t w = 0; w < sentences_in_list.size(); w++)
    {
        if (w > 0)
            result.push_back(" "); // single separator between words, not after the last
        for (const auto &token : sentences_in_list[w].export_as_vector())
        {
            result.push_back(token);
        }
    }

    return result;
    
}

string BPE_tokenizer::detokenize(vector<string> sentences)
{
    string result = "";
    for(const string &token : sentences){
        result += token;
    }

    return result;
}