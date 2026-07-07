import time
from collections import Counter
from itertools import islice

import config
from utils.text_preprocessing import load_arabic_letters, clean_text
from BPE_tokenizer.BPE_tokenizer import BPE_tokenizer


def main():
    arabic_letters = load_arabic_letters(config.ARABIC_JSON)

    start = time.perf_counter()
    uni = Counter()
    report_every = max(1, config.TRAIN_MAX_LINES // 100)

    try:
        with open(config.TRAIN_INPUT, "r", encoding="utf-8") as file:
            for line_number, line in enumerate(islice(file, config.TRAIN_MAX_LINES)):
                if line_number % report_every == 0:
                    print(f"{line_number // report_every}% is done")

                try:
                    uni.update(clean_text(line, arabic_letters).split())
                except MemoryError:
                    print(f"Out of memory at file, line {line_number}")
                    raise
                except Exception as e:
                    print(f"Exception at line {line_number}: {e}")
    except FileNotFoundError:
        print(f"Could not open file: {config.TRAIN_INPUT}")
        return

    number_of_words = sum(uni.values())
    total_lengths = sum(len(word) * count for word, count in uni.items())

    print(f"Total words: {number_of_words}")
    print(f"Unique words: {len(uni)}")
    print(f"Total character length: {total_lengths}")
    print(f"Reading and cleaning execution time: {int((time.perf_counter() - start) * 1000)} ms")

    config.TEMP_DIR.mkdir(parents=True, exist_ok=True)
    with open(config.CLEANED_DATASET_OUTPUT, "w", encoding="utf-8") as f:
        f.writelines(f"{word}\n" for word in uni)

    start = time.perf_counter()
    bpe_tokenizer = BPE_tokenizer()
    bpe_tokenizer.train(uni, config.NUM_MERGES)
    print(f"Execution time of the training process: {int((time.perf_counter() - start) * 1000)} ms")


if __name__ == "__main__":
    main()
