#include <iostream>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <regex>
#include "text_preprocessing.hpp"
#include <chrono>
#include <cctype>

// Alias for convenience
using json = nlohmann::json;
using namespace std;

unordered_set<string> load_arabic_letters(const string &path)
{
    unordered_set<string> letters;

    ifstream file(path);
    if (!file.is_open())
    {
        cerr << "Error: Cannot open file " << path << endl;
        return letters;
    }

    json content;
    try
    {
        file >> content;
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON parse error: " << e.what() << endl;
        return letters;
    }

    if (content.contains("letters") && content["letters"].is_array())
    {
        for (const auto &letter_json : content["letters"])
        {
            if (letter_json.is_string())
            {
                string letter = letter_json.get<string>();
                letters.insert(letter); // store the UTF-8 string directly
            }
        }
    }
    else
    {
        cerr << "JSON does not contain 'letters' array." << endl;
    }

    return letters;
}

size_t utf8_char_length(unsigned char c)
{
    if ((c & 0xF8) == 0xF0)
        return 4;
    else if ((c & 0xF0) == 0xE0)
        return 3;
    else if ((c & 0xE0) == 0xC0)
        return 2;
    else
        return 1;
}

string delete_non_arabic_letters(const string &text, unordered_set<string> arabic_tokens = {})
{

    if(arabic_tokens.empty())
        arabic_tokens = load_arabic_letters("langauge-related-utils/arabic.json");

    string result;
    result.reserve(text.size());
    size_t i = 0;

    while (i < text.size())
    {
        unsigned char c = text[i];
        size_t len = utf8_char_length(c);
        string utf8_char = text.substr(i, len);
        int num_of_occs = arabic_tokens.count(utf8_char);
        if (num_of_occs || utf8_char == " " || utf8_char == "\n")
        {
            result += utf8_char;
        }

        i += len;
    }

    return result;
}

vector<int> prefix_function(string s)
{
    int n = (int)s.length();
    vector<int> pi(n);
    for (int i = 1; i < n; i++)
    {
        int j = pi[i - 1];
        while (j > 0 && s[i] != s[j])
            j = pi[j - 1];
        if (s[i] == s[j])
            j++;
        pi[i] = j;
    }
    return pi;
}

string delete_pattern_from_text(string text, string pattern)
{
    text = pattern + text;
    int m = pattern.length();
    int n = (int)text.length();
    vector<int> pi(n);
    for (int i = 1; i < n; i++)
    {
        int j = pi[i - 1];
        while (j > 0 && text[i] != text[j])
            j = pi[j - 1];
        if (text[i] == text[j] && j + 1 <= m)
            j++;
        pi[i] = j;
    }
    string result = "";
    result.reserve(n);
    for (int i = m; i + m - 1 < n; i++)
    {
        if (pi[i + m - 1] == m)
        {
            i = i + m - 1;
            continue;
        }
        result += text[i];
    }
    return result;
}

string replace_consecutive_whitespaces_with_single_space(string text)
{
    string result = "";
    result.reserve(text.size());
    for (int i = 0; i < text.length(); i++)
    {
        if (!isspace(static_cast<unsigned char>(text[i])))
        {
            result += text[i];
        }
        else
        {
            if (!result.empty() && result.back() != ' ')
            {
                result += ' ';
            }
        }
    }
    return result;
}

// Clean text function
string clean_text(const string &input, unordered_set<string> arabic_tokens = {})
{
    try
    {
        bool print = false;

        using namespace std::chrono;

        string text = input;

        // Remove markers
        auto start = high_resolution_clock::now();
        // assert(regex_replace(text, regex(R"(###Human:)"), "") == delete_pattern_from_text(text, "###Human:"));
        // assert(regex_replace(text, regex(R"(###Assistant:)"), "") == delete_pattern_from_text(text, "###Assistant:"));
        // text = regex_replace(text, regex(R"(###Human:)"), "");
        // text = regex_replace(text, regex(R"(###Assistant:)"), "");
        // text = delete_pattern_from_text(text, "###Human:");
        // text = delete_pattern_from_text(text, "###Assistant:");

        auto end = high_resolution_clock::now();

        auto duration = duration_cast<milliseconds>(end - start);
        if (print)
            std::cout << "Execution time for first regex: " << duration.count() << " ms" << std::endl;
        // Filter non-Arabic letters
        start = high_resolution_clock::now();
        text = delete_non_arabic_letters(text, arabic_tokens);
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        if (print)
            std::cout << "Execution time for letters deletion: " << duration.count() << " ms" << std::endl;
        // Replace multiple newlines and spaces

        start = high_resolution_clock::now();
        text = replace_consecutive_whitespaces_with_single_space(text);
        // text = regex_replace(text, regex(R"(\n+)"), " ");
        // text = regex_replace(text, regex(R"(\s+)"), " ");

        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        if (print)
            std::cout << "Execution time for last regex: " << duration.count() << " ms" << std::endl;

        return text;
    }
    catch (...)
    {
        cout << "The input: " << input << "\n";
        return "failed";
        exit(0);
    }
    assert(false);
    return "failed";
}

vector<string> clean_text(const vector<string> &dataset, unordered_set<string> arabic_tokens)
{
    vector<string> cleaned_dataset;
    cleaned_dataset.reserve((int)dataset.size());
    for (string x : dataset)
        cleaned_dataset.push_back(clean_text(x, arabic_tokens));
    return cleaned_dataset;
}

// int main()
// {

//     string text = "###Human:\nمرحبًا! ###Assistant: أهلا وسهلا 123";
//     cout << "text " << text << endl;
//     string cleaned = clean_text(text);

//     cout << "cleaned" << cleaned << endl;
// }