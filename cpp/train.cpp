#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include "nlohmann/json.hpp" // relative to include root
#include "utils/text_preprocessing.hpp"
#include "utils/config.hpp"
#include "utils/utils.hpp"
#include "BPE_tokenizer/BPE_tokenizer.hpp"
#include <chrono>

using json = nlohmann::json;
using namespace std;

int main()
{

    unordered_set<string> arabic_letters = load_arabic_letters(get_config().arabic_letters_path);
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

    string file_name = get_config().train_dataset_path;
    ifstream file(file_name);

    if (!file.is_open())
    {
        cout << "❌ Could not open file: " << file_name << endl;
        return 1;
    }

    // File size for dataset-independent progress reporting (measured by bytes read).
    file.seekg(0, ios::end);
    long long file_size = file.tellg();
    file.seekg(0, ios::beg);

    string line;
    long long line_number = 0;

    while (getline(file, line))
    {

        if (line_number % 1000000 == 0 && file_size > 0)
        {
            long long bytes_read = file.tellg();
            if (bytes_read >= 0)
                cout << (100 * bytes_read / file_size) << "% is done" << endl;
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
