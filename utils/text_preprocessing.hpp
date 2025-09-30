#ifndef TEXT_UTILS_HPP
#define TEXT_UTILS_HPP

#include <string>
#include <unordered_set>

using namespace std;

// Declare your function so others can use it
unordered_set<string> load_arabic_letters(const string &path);
string clean_text(const string& text, unordered_set<string> arabic_letters);
vector<string> clean_text(const vector<string>& dataset, unordered_set<string> arabic_letters);
size_t utf8_char_length(unsigned char c);

#endif // TEXT_UTILS_HPP