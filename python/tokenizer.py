import json
from itertools import islice

import config
from utils.text_preprocessing import load_arabic_letters, clean_text
from BPE_tokenizer.BPE_tokenizer import BPE_tokenizer


def main():
    arabic_letters = load_arabic_letters(config.ARABIC_JSON)
    bpe_tokenizer = BPE_tokenizer()

    total_words = 0
    total_tokens = 0
    report_every = max(1, config.EVAL_MAX_LINES // 100)

    try:
        with open(config.EVAL_INPUT, "r", encoding="utf-8") as file:
            for i, line in enumerate(islice(file, config.EVAL_MAX_LINES)):
                if i % report_every == 0:
                    print(f"{i // report_every}% is done")

                try:
                    text = clean_text(json.loads(line)["text"], arabic_letters)
                    tokens = bpe_tokenizer.tokenize(text)
                    assert bpe_tokenizer.detokenize(tokens) == text

                    total_words += len(text.split())
                    total_tokens += len(tokens)
                except json.JSONDecodeError as e:
                    print(f"Error parsing JSON: {e}")
                    return
                except Exception as e:
                    print(f"Unexpected error at line {i}: {e}")
                    return
    except FileNotFoundError:
        print("Failed to open file.")
        return

    if total_words:
        print(f"{total_tokens / total_words:.6f}")
    else:
        print("No words to evaluate.")


if __name__ == "__main__":
    main()
