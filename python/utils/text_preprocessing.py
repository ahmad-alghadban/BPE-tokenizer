import json
import re

def load_arabic_letters(path: str) -> set[str]:
    with open(path, 'r', encoding='utf-8') as file:
        content = json.load(file)

    if "letters" not in content or not isinstance(content["letters"], list):
        raise ValueError(f"{path} does not contain a 'letters' array.")

    letters = {letter for letter in content["letters"] if isinstance(letter, str)}
    if not letters:
        raise ValueError(f"{path} contains no valid letters.")
    return letters

def delete_non_arabic_letters(text: str, arabic_tokens: set[str]) -> str:
    if not arabic_tokens:
        raise ValueError("arabic_tokens must be a non-empty set; load it once with load_arabic_letters().")

    allowed = arabic_tokens | {" ", "\n"}
    return "".join(ch for ch in text if ch in allowed)

def prefix_function(s: str) -> list[int]:
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
    return re.sub(r"\s+", " ", text).strip()

def clean_text(input_text: str, arabic_tokens: set[str] = None) -> str:
    # Remove markers (commented out in C++ version)
    # input_text = delete_pattern_from_text(input_text, "###Human:")
    # input_text = delete_pattern_from_text(input_text, "###Assistant:")

    text = delete_non_arabic_letters(input_text, arabic_tokens)
    text = replace_consecutive_whitespaces_with_single_space(text)
    return text

def clean_text_batch(dataset: list[str], arabic_tokens: set[str]) -> list[str]:
    return [clean_text(text, arabic_tokens) for text in dataset]