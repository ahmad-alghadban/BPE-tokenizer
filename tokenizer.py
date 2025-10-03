import json
from utils.text_preprocessing import load_arabic_letters, clean_text
from BPE_tokenizer.BPE_tokenizer import BPE_tokenizer

def main():
    arabic_letters = load_arabic_letters("langauge-related-utils/arabic.json")
    bpe_tokenizer = BPE_tokenizer()

    total_first = 0
    total_second = 0
    i = 0

    try:
        with open("datasets/wiki-arabic-v2-dataset/arabic-wiki-simple-dataset/wiki-v2.jsonl", "r", encoding="utf-8") as file:
            while i < 10000:
                if i % (10000 // 100) == 0:
                    print(f"{i // (10000 // 100)}% is done")

                try:
                    line = file.readline()
                    if not line:
                        break  # End of file
                    
                    # print("line", line)

                    j = json.loads(line)
                    s = j["text"]
                    s = clean_text(s, arabic_letters)  # Optional in testing phase
                    
                    # print("s", "'"+s+"'")

                    words = s.count(' ') + 1
                    total_first += words

                    tokenized = bpe_tokenizer.tokenize(s)
                    total_second += len(tokenized)

                    de_tokenized = bpe_tokenizer.detokenize(tokenized)
                    # print("Got here!", "'"+de_tokenized+"'")
                    assert s == de_tokenized

                    i += 1

                except json.JSONDecodeError as e:
                    print(f"Error parsing JSON: {e}")
                    return
                except Exception as e:
                    print(f"Unexpected error at line {i}: {e}")
                    return

    except FileNotFoundError:
        print("Failed to open file.")
        return

    print(f"{total_second / total_first:.6f}")

if __name__ == "__main__":
    main()