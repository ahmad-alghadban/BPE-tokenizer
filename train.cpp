#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include "nlohmann/json.hpp" // relative to include root
#include "utils/text_preprocessing.hpp"
#include "BPE_tokenizer/BPE_tokenizer.hpp"
#include <chrono>
#include <locale>
#include <codecvt>

using json = nlohmann::json;
using namespace std;

string int_to_string(int x)
{
    if (x == 0)
        return "0";
    string result = "";
    while (x)
    {
        result += '0' + x % 10;
        x /= 10;
    }
    reverse(result.begin(), result.end());
    return result;
}

int main()
{

    unordered_set<string> arabic_letters = load_arabic_letters("langauge-related-utils/arabic.json");
    if (arabic_letters.empty())
    {
        cerr << "Arabic letter set empty. Exiting.\n";
        return 1;
    }

    using namespace std::chrono;

    auto start = high_resolution_clock::now();

    long long total_lengths = 0;
    long long number_of_words = 0;
    unordered_map<string, int> uni;

    string file_name = "datasets/arabic_1b_words/arabic_1b_words.txt";
    ifstream file(file_name);

    if (!file.is_open())
    {
        cout << "❌ Could not open file: " << file_name << endl;
        return 1;
    }

    string line;
    int line_number = 0;

    while (getline(file, line))
    {

        if(line_number%(61000000/100) == 0){
            cout << line_number/(61000000/100) << "% is done" << endl;
        }

        try
        {
            string cleaned_text = clean_text(line, arabic_letters);
            string s = "";
            int num_of_words = 0;

            for (char c : cleaned_text)
            {
                if (c == ' ')
                {
                    if (!s.empty())
                    {
                        uni[s]++;
                        s.clear();
                    }
                    num_of_words++;
                }
                else
                {
                    s += c;
                    total_lengths++;
                }
            }

            if (!s.empty())
            {
                uni[s]++;
            }

            number_of_words += num_of_words + 1;
            line_number++;
        }
        catch (const std::bad_alloc &e)
        {
            cout << "Out of memory at file, line " << line_number << endl;
            throw;
        }
        catch (const std::exception &e)
        {
            cout << "Exception at line " << line_number << ": " << e.what() << endl;
        }
        catch (...)
        {
            cout << "Unknown exception at line " << line_number << endl;
        }
    }

    file.close();
    cout << "Total words: " << number_of_words << endl;
    cout << "Unique words: " << uni.size() << endl;
    cout << "Total character length: " << total_lengths << endl;

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Reading and cleaning execution time: " << duration.count() << " ms" << endl;

    start = high_resolution_clock::now();
    BPE_tokenizer bpe_tokenizer;
    bpe_tokenizer.train(uni, 1<<16, number_of_words + 100);

    end = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(end - start);
    std::cout << "Execution time of the training process: " << duration.count() << " ms" << std::endl;

    return 0;
}
