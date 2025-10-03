import json
import time
from typing import Set, List

def load_arabic_letters(path: str) -> Set[str]:
    letters = set()
    try:
        with open(path, 'r', encoding='utf-8') as file:
            content = json.load(file)
    except FileNotFoundError:
        print(f"Error: Cannot open file {path}")
        return letters
    except json.JSONDecodeError as e:
        print(f"JSON parse error: {e}")
        return letters

    if "letters" in content and isinstance(content["letters"], list):
        for letter in content["letters"]:
            if isinstance(letter, str):
                letters.add(letter)
    else:
        print("JSON does not contain 'letters' array.")
    return letters

def delete_non_arabic_letters(text: str, arabic_tokens: Set[str] = None) -> str:
    if arabic_tokens is None or not arabic_tokens:
        arabic_tokens = load_arabic_letters("langauge-related-utils/arabic.json")

    result = ""
    for ch in text:
        if ch in arabic_tokens or ch in {" ", "\n"}:
            result += ch
    
    return result

def prefix_function(s: str) -> List[int]:
    n = len(s)
    pi = [0] * n
    for i in range(1, n):
        j = pi[i - 1]
        while j > 0 and s[i] != s[j]:
            j = pi[j - 1]
        if s[i] == s[j]:
            j += 1
        pi[i] = j
    return pi

def delete_pattern_from_text(text: str, pattern: str) -> str:
    combined = pattern + text
    m = len(pattern)
    n = len(combined)
    pi = [0] * n
    for i in range(1, n):
        j = pi[i - 1]
        while j > 0 and combined[i] != combined[j]:
            j = pi[j - 1]
        if combined[i] == combined[j] and j + 1 <= m:
            j += 1
        pi[i] = j

    result = []
    i = m
    while i + m - 1 < n:
        if pi[i + m - 1] == m:
            i += m
        else:
            result.append(combined[i])
            i += 1
    return ''.join(result)

def replace_consecutive_whitespaces_with_single_space(text: str) -> str:
    result = []
    for c in text:
        if not c.isspace():
            result.append(c)
        elif result and result[-1] != ' ':
            result.append(' ')
    if result[-1] == ' ': result.pop()
    return ''.join(result)

def clean_text(input_text: str, arabic_tokens: Set[str] = None) -> str:
    try:
        print_debug = False

        # Remove markers (commented out in C++ version)
        # input_text = delete_pattern_from_text(input_text, "###Human:")
        # input_text = delete_pattern_from_text(input_text, "###Assistant:")

        start = time.time()
        text = delete_non_arabic_letters(input_text, arabic_tokens)
        if print_debug:
            print(f"Execution time for letters deletion: {int((time.time() - start) * 1000)} ms")

        start = time.time()
        text = replace_consecutive_whitespaces_with_single_space(text)
        if print_debug:
            print(f"Execution time for whitespace cleanup: {int((time.time() - start) * 1000)} ms")

        return text
    except Exception as e:
        print(f"The input: {input_text}\nError: {e}")
        return "failed"

def clean_text_batch(dataset: List[str], arabic_tokens: Set[str]) -> List[str]:
    return [clean_text(text, arabic_tokens) for text in dataset]