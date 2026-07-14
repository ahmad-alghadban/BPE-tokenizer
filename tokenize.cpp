#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <nlohmann/json.hpp>
#include "utils/text_preprocessing.hpp"
#include "utils/config.hpp"
#include "utils/utils.hpp"
#include "utils/errors.hpp"
#include "BPE_tokenizer/BPE_tokenizer.hpp"
#include <chrono>
#include <locale>
#include <codecvt>
#include "utils/text_preprocessing.hpp"

using json = nlohmann::json;
using namespace std::chrono;
using namespace std;

int main()
{

    unordered_set<string> arabic_letters = load_arabic_letters(get_config().arabic_letters_path);
    BPE_tokenizer bpe_tokenizer;
    
    std::ifstream file(get_config().tokenize_dataset_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    int i = 0;

    int total_first = 0, total_second = 0;

    const int max_lines = 100000;
    while(i < max_lines){
        if(i % (max_lines/100) == 0){
            cout << i/(max_lines/100) << "% is done" << endl;
        }
        nlohmann::json j;
        try {
            file >> j;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << "\n";
            return 1;
        }
        if(!file){ // reached end of file (or read failure): stop cleanly
            break;
        }
        i++;
        if (!j.contains("text") || !j["text"].is_string())
            continue; // skip records with no usable "text" field
        string s = j["text"];
        s = clean_text(s, arabic_letters); // This step should not be done in the testing phase, but it is used here to omit non-arabic letters
        int words = 1;
        for(char c : s){
            if(c == ' ') words++;
        }
        total_first += words;
        vector<string> tokenized = bpe_tokenizer.tokenize(s, words+10);
        total_second += tokenized.size();
        string de_tokenized = bpe_tokenizer.detokenize(tokenized);
        if (s != de_tokenized)
            throw ConsistencyError("tokenize/detokenize round-trip mismatch for input: \"" + s + "\"");
    }

    cout << (1.0*total_first)/(1.0*total_second) << "\n";

    return 0;
}