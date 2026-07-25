# BPE Tokenizer

_An optimized implementation of the BPE tokenization algorithm, highly optimized to process billions of words on medium-resource machines. It comes in two versions: a **C++** version for optimizations, and a **Python** version for prototyping._

## Overview
**Byte-Pair Encoding (BPE)** is an algorithm used to *tokenize* text. That is, to split text into small pieces called **tokens** that a computer or a language model can work with. It follows a **bottom-up** approach: instead of starting from whole words and breaking them down, it starts from the smallest pieces (single characters) and builds larger tokens up from them, repeatedly merging the most common neighboring pair into a new token. After many merges you end up with a vocabulary of subword pieces, a middle ground between whole words (too many to handle) and single characters (too small to be useful). This also lets it handle words it never saw during training, by reusing familiar patterns. For example, a new past-tense verb ending in *ed*.

This project provides both halves of that process: a **trainer** that learns the merges from a body of text, and a **tokenizer/detokenizer** that uses those merges to turn text into tokens and back again.

It is **language-agnostic**. The set of allowed characters lives in a separate, swappable file (for example `arabic.json`), so pointing it at a different character set retargets the tokenizer to another language. The reference model here happens to be trained on Arabic.

There are **two versions** for two different needs: the **C++** version carries the heavy, performance-critical training, while the **Python** version mirrors the same logic in a simpler form for quick experiments.

The heavy part is the **training**, and that's where the speed of C++ matters. Once training is done, the result is just a set of saved token tables. **Tokenizing** with those tables is lightweight and simple, so it can be done in any language you like; you only need C++ for training the model, not for using it, and if your use case is for **Arabic**, the model is already trained on Arabic dataset and it is ready to use.

## Repository Layout
```
.
├── cpp/                          # C++ version (for training at scale)
│   ├── train.cpp                 # entry point: learns merges from a corpus
│   ├── tokenize.cpp              # entry point: applies a trained model to text
│   ├── config.json               # paths and settings for the C++ programs
│   ├── Makefile                  # build with `make`
│   ├── BPE_tokenizer/            # core algorithm + saved token tables
│   ├── utils/                    # text preprocessing, linked list, config, helpers
│   ├── langauge-related-utils/   # character-set files (e.g. arabic.json)
│   └── include/nlohmann/         # library used to read JSON files
│
├── python/                       # Python version (for prototyping)
│   ├── train.py                  # entry point: training
│   ├── tokenizer.py              # entry point: tokenizing
│   ├── config.py                 # paths and settings
│   ├── requirements.txt          # dependencies
│   ├── BPE_tokenizer/            # core algorithm + saved token tables
│   ├── utils/                    # text preprocessing, linked list
│   └── langauge-related-utils/   # character-set files
│
├── datasets/                     # training data (not versioned; add your own)
└── README.md
```

## Quick Start
### C++
You need a C++17 compiler (for example `g++`).

```bash
cd cpp
make            # builds two programs: train and tokenize
./train         # learns the token tables from the training dataset
./tokenize      # applies the trained tables to text
```

Run the programs from inside `cpp/`, because the paths in `config.json` are relative. Open [`config.json`](cpp/config.json) to set where your datasets live and where the token tables are read from and written to. Datasets are not included, so add your own (see [Configuration & Data](#configuration--data)).

### Python
You need Python 3 and the one dependency in `requirements.txt`.

```bash
cd python
pip install -r requirements.txt
python train.py       # trains the model
python tokenizer.py   # tokenizes text with the trained model
```

Settings (dataset paths, number of merges, line limits) live in [`config.py`](python/config.py); edit that file to change them.

## How It Works
Training happens in a few clear steps:

1. **Clean the text.** Read the corpus and keep only the characters listed in the character-set file (plus spaces). Everything else is dropped, and runs of whitespace are collapsed into a single space. This leaves clean, consistent text to learn from.
2. **Count the words.** Go through the cleaned text and count how often each unique word appears. From here on, the tokenizer works with these word counts instead of the raw text, which is much less to process.
3. **Learn the merges.** Start with every word as a plain sequence of characters. Find the most common neighboring pair of pieces across all words, merge that pair into a single new token, and repeat for as many merges as you ask for. Each round adds one new token to the vocabulary.
4. **Save the token tables.** The learned tokens are written out to text files. These files are the trained model.

Using the model (tokenizing) is the reverse of the last step: take a piece of text and apply the learned merges to split it into tokens. **Detokenizing** joins those tokens back into the original text. The two are exact opposites, and the tokenizer checks this by confirming that tokenizing and then detokenizing gives back exactly what it started with.

## C++ Design & Optimizations
This is the heart of the project. Training on a billion words is only practical because the algorithm never redoes work it doesn't have to. This section explains the core algorithm first, then the specific choices that make it fast.

### The BPE Algorithm
The standard Byte-Pair Encoding training algorithm is short to describe:

1. **Start from characters.** Split every word into its individual characters, and let the starting vocabulary be the set of all characters that appear.
2. **Count pairs.** Go through the whole corpus and count how often each neighboring pair of symbols appears (for example, how many times `t` is directly followed by `h`).
3. **Merge the most frequent pair.** Take the pair with the highest count, join it into a single new symbol, add that symbol to the vocabulary, and replace every occurrence of the pair with it.
4. **Repeat.** Count pairs again on the updated corpus, merge the next most frequent pair, and keep going for a fixed number of merges (or until the vocabulary reaches a target size).

Each merge produces one new token, and later merges can join earlier tokens together, so tokens grow from single characters into common letter groups, then whole subwords, and eventually frequent full words. The list of merges, in order, is the trained model.

**Why the naive version is slow.** Written directly, steps 2 and 3 re-scan the entire corpus on every merge: recounting all pairs, then rewriting every word. That makes the total work roughly `number of merges × size of the corpus`. With tens of thousands of merges over a billion words, that product is enormous, which is exactly the cost this implementation is built to avoid. The rest of this section is about how.

### Count Words First, Then Work on Unique Words
The first and most impactful step is to **not** feed the raw text into the algorithm at all. Instead, the trainer first reduces the corpus to a table of **unique words and how often each appears**, and every later step runs over that compact table rather than the full text. A pair inside a word occurs once for every time the word appears, so each pair's count is simply weighted by the word's frequency. The totals come out identical to scanning the raw text, for a fraction of the data.

How large is the saving? It follows from **Heaps' law** (also called Herdan's law), which describes how vocabulary grows with corpus size: the number of distinct words `V` in a corpus of `N` total words is about `V ≈ K · N^β`, where the exponent `β` is empirically around `0.4`–`0.6`. Because `β` is well below 1, the vocabulary grows much more slowly than the text: doubling the corpus adds far fewer than double the new words, since most words have been seen already.

In this project's own run, a corpus of about `10^9` words reduced to roughly `3 × 10^6` unique words, a **couple-hundred-fold** shrink in the data every pass has to scan (and the number of merges is unchanged; only the corpus size each merge touches gets smaller). Note that this is well above the naive `√N ≈ 3 × 10^4` one might guess: the constant `K` is large here (on the order of 100), and two properties of this corpus push the vocabulary up:

- **Arabic is morphologically rich.** A single root spawns many surface forms: prefixes (`و`, `ال`, `ب`, `ل`), suffixes, and attached clitics all glue directly onto the word, and each combination is a distinct string. Arabic therefore produces far more unique word-forms per root than a language like English, which raises `β`.
- **Web text is noisy.** The corpus is crawled from the web, so numbers, typos, transliterations, foreign words, and stray character sequences each add fresh one-off "words" that never repeat, inflating the vocabulary further.

Even so, collapsing a billion word-tokens to a few million unique types is a massive reduction, and it is what makes everything after it affordable.

### Core Data Structures
The merge loop leans on a few purpose-built structures, each chosen so that a single merge only has to touch what actually changed. This subsection walks through them one at a time.

**Pair frequencies (`temp_tokens_pair_freq`).**
This table answers one question: for a given ordered pair of neighboring tokens, how many times does that pair occur across the whole corpus? It is a two-level map, keyed first by the left token and then by the right token, so a lookup reads as `temp_tokens_pair_freq[left][right]` and lands on an integer count. The outer level is a hash map (fast average lookup on the left token) and the inner level is an ordered map from the right token to the count.

It is filled once, during setup, while each unique word is being walked left to right. For every adjacent pair `(previous, current)` inside a word, the code does `temp_tokens_pair_freq[previous][current] += freqs`, where `freqs` is that word's frequency from the unique-words table. This is the payoff of counting unique words first: a pair that sits inside a word appearing 5,000 times is counted as 5,000, but the word itself is walked only once. The resulting totals are identical to scanning the raw corpus, at a fraction of the cost.

The `temp_` prefix reflects its role: it is the initial snapshot of every pair's count, built once and then used to seed the merge loop. Once merging begins, the live, constantly-changing counts are maintained in a separate structure (described below), so this table is really the starting point the rest of the machinery is built on.

**Words as linked lists (a custom `linked_list`).**
Each word is held as a doubly linked list of its current tokens, using a small custom class rather than a standard container. The reason is the merge step: joining a neighboring pair means unlinking two adjacent nodes and linking one merged node in their place, a constant-time pointer rewire no matter how long the word is. An array would instead have to shift every element after the merge point, on every merge.

The list carries one deliberate change from a textbook version, and it exists because other structures keep **direct pointers to individual nodes** (the position map, described next, is one). If a merged-away node were freed the instant it was merged, those pointers would dangle. So `erase()` does not free the node: it marks the node as `deleted` and parks it in a "graveyard," keeping it alive so any stale pointer can safely ask "was this deleted?" and skip it, instead of reading freed memory. All the parked nodes are then freed together, in bulk, when the list is destroyed at the end of training.

**The position map (`tokens_pair_to_nodes_mapper`).**
This is the structure that makes a merge fast, and it is the whole reason the linked-list nodes cannot be freed eagerly. It answers one precise question: *given a pair of tokens, where exactly does that pair live?* For every pair `(first, second)`, it stores a list of the occurrences of that pair, and each occurrence records three things:

- a **direct pointer to the node holding `first`** (the left half of the pair) inside some word's linked list,
- the **index of that word** in `sentences_in_list`, and
- the **frequency of that word**, so one recorded occurrence stands in for every time that word appears.

Here is the power. When the merge loop decides to merge, say, `ا` + `ل` into `ال`, it does **not** scan the corpus hunting for adjacent `ا`,`ل`. It looks up `tokens_pair_to_nodes_mapper["ا"]["ل"]` and instantly gets the exact list of every place that pair occurs, then walks that list and splices each one in O(1). So applying a merge costs only about **as much as the number of times that one pair occurs**, never the size of the corpus. Selecting the pair answers "*what* to merge next"; this map answers "*where* it is," and it answers it by direct lookup rather than by searching. That is what collapses each round from a full sweep of a billion words into a short list of surgical edits.

The map is kept alive as merging proceeds: when a newly merged token forms fresh pairs with its left and right neighbors, those new positions are appended, so later merges can find them too.

And this is where the linked list's deferred deletion pays off. Occurrences are **never eagerly removed** from these lists as words change, because that book-keeping would cost more than it saves. Stale entries are simply left in place and skipped on sight: before using an occurrence, the loop checks whether its node was deleted and whether the pair sitting there is still the pair it expects. A stored pointer therefore has to stay safe to inspect long after its node was spliced out, which is exactly why nodes are marked `deleted` and kept in the graveyard instead of being freed.

**The frequency-ordered set (`std::set` of `(count, pair)`).**
The position map says *where* a pair is; this set says *which* pair to merge next. It holds every pair as a `(count, pair)` entry, kept sorted by count. Because it stays ordered, the most frequent pair is always the last element and can be read instantly. When a merge changes a pair's count, the entry is removed and reinserted at its new position in `O(log n)`, so the set stays correct without ever recounting from scratch. That is the whole job: always know the top pair, cheaply.

### One Merge Round, End to End
Here is how the four structures cooperate in a single round. Suppose the most frequent pair right now is `ا` + `ل`, and one of the words that contains it is `ع ا ل م` ("world").

1. **Pick the pair.** The frequency-ordered set has `(count, ا,ل)` sitting at its end, so the round grabs the top pair instantly, removes it from the set, and records `ال` as a new token in the vocabulary.
2. **Jump to every occurrence.** The position map hands back the exact list of spots where `ا`,`ل` occurs, with no searching. The round walks that list; our word `ع ا ل م` is one entry.
3. **Retire the pairs that are about to vanish.** At this spot, the merge will destroy three neighboring pairs: `(ع, ا)`, `(ا, ل)`, and `(ل, م)`. Each of their counts is decremented by this word's frequency, and each change is a quick remove-and-reinsert in the set. (If the pair sat at a word boundary, there would simply be one fewer neighbor to update.)
4. **Splice.** On the linked list, the `ا` and `ل` nodes are erased (marked `deleted` and parked in the graveyard) and a single `ال` node is linked in their place, in O(1). The word is now `ع ال م`.
5. **Register the pairs that just appeared.** The merge created two new neighboring pairs, `(ع, ال)` and `(ال, م)`. Their counts are incremented, and their new positions are appended to the position map so later rounds can find them.

Notice what the round did **not** do. It never looked at a single word that didn't contain `ا ل`, and even inside `ع ا ل م` it only touched the three characters around the merge. Repeat steps 3 to 5 for every occurrence of the pair, then start the next round, where the set once again has the current most-frequent pair waiting at its end. The cost of the whole round is proportional to how often that one pair occurred, while the millions of untouched words simply sit still. That is the entire point of the design.

### Copy Avoidance & Hot-Path Tuning
The structures above are large and deeply nested (hash maps of ordered maps of lists), so a single accidental copy can be very expensive. A handful of habits keep the hot paths lean:

- **Pass big structures by `const&`.** The word-frequency table and the per-pair maps are handed between functions by reference, never copied. Reading loops use `const auto&` structured bindings (for example `for (const auto &[first, seconds] : ...)`) so iterating never duplicates an entry.
- **`reserve()` before filling.** Containers whose rough final size is known ahead of time are reserved up front, so they fill without repeated reallocation and rehashing: the list vector and the frequency map are sized to the word count, and the cleaned-text buffers to the length of the input.
- **Move, don't copy.** Where a container's contents can be transferred instead of duplicated, they are moved, and the `swap`-with-an-empty-container trick is used to hand large allocations back to the allocator all at once.
- **Inline the tightest helpers.** The per-byte UTF-8 length helper runs once for every byte of the corpus, so it is defined `inline` in a header and folded straight into each call site rather than paying a function call every time.

None of these change what the algorithm computes. They only keep the constant factor small, which matters enormously when the inner loops run billions of times.

## Configuration & Data
### Settings
Paths and settings live in one place per version:

- **C++:** [`cpp/config.json`](cpp/config.json) holds the file paths (training data, tokenizing data, the character-set file, and where the token tables are read from and written to).
- **Python:** [`python/config.py`](python/config.py) holds the same paths plus a few defaults, such as the number of merges and how many lines to read.

### The character-set file
The list of allowed characters lives in a small JSON file, for example [`arabic.json`](cpp/langauge-related-utils/arabic.json). This is what makes the tokenizer language-agnostic: to work with another language, point the config at a different character-set file. Any character not in this file is dropped while cleaning the text.

### Datasets
Datasets are **not included** in the repository (they are large and git-ignored), so you provide your own. Two are used:

- **Training data** (a plain text file, one line at a time, words separated by spaces). The reference model was trained on Arabic text from [ClusterlabAi/101_billion_arabic_words_dataset](https://huggingface.co/datasets/ClusterlabAi/101_billion_arabic_words_dataset). That corpus is huge (101 billion words), so rather than use all of it, about **1 billion words** were streamed out of it and the model was trained on that slice only.
- **Tokenizing/testing data** (a JSONL file where each line is a JSON record with a `"text"` field). This project uses [abdullahbl/arabic-wiki-simple-dataset](https://huggingface.co/datasets/abdullahbl/arabic-wiki-simple-dataset), a set of Arabic Wikipedia text.

## Outputs
Training produces two text files, and together they are the trained model:

- **`tokens_ordered.txt`** — the learned tokens (the vocabulary), one per line, in the order they were learned. The most useful merges come first. For example, the top lines of the Arabic model are common prefixes and short words like `ال`, `ات`, `في`, `من`.
- **`splitted_tokens_ordered.txt`** — the same tokens, but each line shows the two pieces that were merged to create it (separated by a space), such as `ا ل` or `ال م`. This is the "recipe" for rebuilding each token, and it is what lets the tokenizer apply the merges to new text.

Both files have one line per merge, so their length equals the number of merges you trained for (65,536 in the reference model).

When you run `tokenize`, it also prints a **compression ratio**: the total number of words divided by the total number of tokens across the text it processed. The higher this number, the fewer tokens each word needs on average, which means better compression.

## Acknowledgements
This project builds on the work of others:

**Datasets**
- [ClusterlabAi/101_billion_arabic_words_dataset](https://huggingface.co/datasets/ClusterlabAi/101_billion_arabic_words_dataset) (Apache 2.0), the source of the training text (a ~1B-word slice was used).
- [abdullahbl/arabic-wiki-simple-dataset](https://huggingface.co/datasets/abdullahbl/arabic-wiki-simple-dataset) (MIT), used for tokenizing and testing.
