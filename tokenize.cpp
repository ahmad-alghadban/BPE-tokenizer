#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <nlohmann/json.hpp>
#include "utils/text_preprocessing.hpp"
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

    unordered_set<string> arabic_letters = load_arabic_letters("langauge-related-utils/arabic.json");
    BPE_tokenizer bpe_tokenizer;
    vector<string> abc;
    
    std::ifstream file("datasets/wiki-arabic-v2-dataset/arabic-wiki-simple-dataset/wiki-v2.jsonl");

    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    int i = 0;

    int total_first = 0, total_second = 0;

    while(i < 100000){
        if(i%(61000000/100) == 0){
            cout << i/(61000000/100) << "% is done" << endl;
        }
        nlohmann::json j;
        try {
            file >> j;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << "\n";
            return 1;
        }
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
        assert(s == de_tokenized);
    }

    cout << (1.0*total_first)/(1.0*total_second) << "\n";

    return 0;
}