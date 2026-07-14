#include <iostream>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <regex>
#include "text_preprocessing.hpp"
#include <chrono>
#include <cctype>
#include "errors.hpp"

// Alias for convenience
using json = nlohmann::json;
using namespace std;

string delete_non_arabic_letters(const string &text, const unordered_set<string> &arabic_tokens)
{
    if (arabic_tokens.empty())
        throw ValueError(
            "delete_non_arabic_letters: arabic_tokens must not be empty "
            "(load it once via load_arabic_letters and pass it in)");

    string result;
    result.reserve(text.size());
    size_t i = 0;

    while (i < text.size())
    {
        unsigned char c = text[i];
        size_t len = utf8_char_length(c);
        string utf8_char = text.substr(i, len);
        if (arabic_tokens.count(utf8_char) || utf8_char == " " || utf8_char == "\n")
        {
            result += utf8_char;
        }

        i += len;
    }

    return result;
}

vector<int> prefix_function(const string &s)
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
    const int m = (int)pattern.size();
    if (m == 0)
        return text;

    // Reuse the pattern's KMP failure function, then run the automaton over the
    // text; each time a full occurrence completes, drop its last m characters.
    // Removes non-overlapping matches left-to-right, matching regex_replace.
    vector<int> pi = prefix_function(pattern);
    string result;
    result.reserve(text.size());

    int j = 0; // length of the pattern prefix currently matched
    for (char c : text)
    {
        while (j > 0 && c != pattern[j])
            j = pi[j - 1];
        if (c == pattern[j])
            j++;
        result += c;
        if (j == m) // a full occurrence just ended -> remove it
        {
            result.erase(result.size() - m);
            j = 0;
        }
    }
    return result;
}

string replace_consecutive_whitespaces_with_single_space(const string &text)
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
    // Drop a trailing separator so cleaned text is canonical (no leading/trailing space).
    if (!result.empty() && result.back() == ' ')
    {
        result.pop_back();
    }
    return result;
}

// Clean text function
string clean_text(const string &input, const unordered_set<string> &arabic_tokens)
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

vector<string> clean_text(const vector<string> &dataset, const unordered_set<string> &arabic_tokens)
{
    vector<string> cleaned_dataset;
    cleaned_dataset.reserve(dataset.size());
    for (const string &x : dataset)
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