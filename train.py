import time
from collections import defaultdict
from utils.text_preprocessing import load_arabic_letters, clean_text
from BPE_tokenizer.BPE_tokenizer import BPE_tokenizer

def int_to_string(x: int) -> str:
    return str(x)

def main():
    arabic_letters = load_arabic_letters("langauge-related-utils/arabic.json")
    if not arabic_letters:
        print("Arabic letter set empty. Exiting.")
        return

    print("arabic_letters", arabic_letters)
    
    start = time.time()

    total_lengths = 0
    number_of_words = 0
    uni = defaultdict(int)

    file_name = "datasets/arabic_1b_words/arabic_1b_words.txt"
    try:
        with open(file_name, "r", encoding="utf-8") as file:
            for line_number, line in enumerate(file):
                if line_number >= 100000: break
                if line_number % (100000 // 100) == 0:
                    print(f"{line_number // (100000 // 100)}% is done")

                try:
                    cleaned_text = clean_text(line, arabic_letters)
                    s = ""
                    num_of_words = 0

                    for c in cleaned_text:
                        if c == ' ':
                            if s:
                                uni[s] += 1
                                s = ""
                            num_of_words += 1
                        else:
                            s += c
                            total_lengths += 1

                    if s:
                        uni[s] += 1

                    number_of_words += num_of_words + 1

                except MemoryError:
                    print(f"Out of memory at file, line {line_number}")
                    raise
                except Exception as e:
                    print(f"Exception at line {line_number}: {e}")
    except FileNotFoundError:
        print(f"❌ Could not open file: {file_name}")
        return

    print(f"Total words: {number_of_words}")
    print(f"Unique words: {len(uni)}")
    print(f"Total character length: {total_lengths}")

    end = time.time()
    print(f"Reading and cleaning execution time: {int((end - start) * 1000)} ms")

    with open('temp/cleaned_data_set.txt', "w", encoding="utf-8") as f:
        for x in uni.keys():
            f.writelines(x + "\n")
        
    start = time.time()
    bpe_tokenizer = BPE_tokenizer()
    bpe_tokenizer.train(uni, 1 << 16)
    end = time.time()

    print(f"Execution time of the training process: {int((end - start) * 1000)} ms")

if __name__ == "__main__":
    main()