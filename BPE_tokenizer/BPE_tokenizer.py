import json
from collections import defaultdict
from utils.double_linked_list_implementation import LinkedList
from sortedcontainers import SortedSet

class BPE_tokenizer:
    def __init__(self):
        self.init_set = set()
        self.tokens = set()
        self.tokens_added_in_order = []
        self.ordered_added_tokens_splitted = []
        
    def get_tokens(self):
        return self.tokens

    def get_tokens_added_in_order(self):
        return self.tokens_added_in_order

    def get_splitted_tokens_added_in_order(self):
        return self.ordered_added_tokens_splitted
    
    def has_tokens(self):
        return len(self.ordered_added_tokens_splitted) != 0

    def add_new_token(self, first_token, second_token):
        new_token = first_token + second_token
        self.tokens.add(new_token)
        self.tokens_added_in_order.append(new_token)
        self.ordered_added_tokens_splitted.append((first_token, second_token))
        
    def save_tokens(self):
        try:
            with open("BPE_tokenizer/splitted_tokens_ordered.txt", "w", encoding="utf-8") as out_file:
                for first_token, second_token in self.get_splitted_tokens_added_in_order():
                    out_file.write(f"{first_token} {second_token}\n")
        except IOError:
            print("Could not open output file!")

        try:
            with open("BPE_tokenizer/tokens_ordered.txt", "w", encoding="utf-8") as out_file1:
                for token in self.get_tokens_added_in_order():
                    out_file1.write(f"{token}\n")
        except IOError:
            print("Could not open output file!")

    def load_tokens(self):
        try:
            with open("BPE_tokenizer/splitted_tokens_ordered.txt", "r", encoding="utf-8") as f:
                for line in f:
                    first, second = line.strip().split()
                    self.add_new_token(first, second)
        except Exception as e:
            print(f"Error loading tokens: {e}")

    def init_data_structures(self, cleaned_dataset):
        """
        cleaned_dataset: dict of {sentence: frequency}
        """
        self.sentences_in_list = []
        self.tokens_pair_to_nodes_mapper = defaultdict(lambda: defaultdict(list))
        self.temp_tokens_pair_freq = defaultdict(lambda: defaultdict(int))

        for sentence, freq in cleaned_dataset.items():
            word_list = LinkedList()
            # self.sentences_in_list.app
            previous = None
            for ch in sentence:
                if ch == ' ':
                    if word_list:
                        self.sentences_in_list.append(word_list)
                    word_list = LinkedList()
                    previous = None
                else:
                    node = word_list.push_back(ch)
                    if previous is not None:
                        self.temp_tokens_pair_freq[previous][ch] += freq
                        self.tokens_pair_to_nodes_mapper[previous][ch].append((node.prev, len(self.sentences_in_list), freq))
                    previous = ch
            if word_list:
                self.sentences_in_list.append(word_list)
                
    def sorting_data_structures_method(self, iters=1000):
        freq_tokens_map = defaultdict(int)
        freq_tokens_sorted = SortedSet()
        
        for first_token, second_token_map in self.tokens_pair_to_nodes_mapper.items():
            for second_token, two_tokens_list in second_token_map.items():
                pair_token = (first_token, second_token)
                freq = self.temp_tokens_pair_freq[first_token][second_token]
                freq_tokens_map[pair_token] = freq
                # if pair_token == ('ا', 'ل'):
                #     print("here:", (freq, pair_token))
                freq_tokens_sorted.add((freq, pair_token))
        
        def update_data_structures(first_node, second_node, update_value):
            pair_token = (first_node.data, second_node.data)
            old_freq = freq_tokens_map[pair_token]
            if old_freq > 0:
                # print(old_freq)
                # print(pair_token)
                # print("freq_tokens_sorted", freq_tokens_sorted)
                freq_tokens_sorted.remove((old_freq, pair_token))
            freq_tokens_map[pair_token] += update_value
            if freq_tokens_map[pair_token] > 0:
                freq_tokens_sorted.add((freq_tokens_map[pair_token], pair_token))
        
        deleted_nodes = set()
        
        for cycle in range(iters):
            if cycle % (iters / 100) == 0:
                print("Done: ", cycle / (iters / 100), "% is done")
            
            if not freq_tokens_sorted:
                print("The whole dataset was tokenized before the end of iterations\n")
                print("The total number of iterations is:", cycle)
                break
            
            last = freq_tokens_sorted[-1]  # equivalent to *rbegin()
            num_of_occ = last[0]
            first_token, second_token = last[1]
            # freq_tokens_sorted.pop()
            self.tokens.add(first_token + second_token)
            self.add_new_token(first_token, second_token)
            not_deleted = 0
            num_deleted_nodes = 0
            pair_nodes = self.tokens_pair_to_nodes_mapper[first_token][second_token]
                        
            for p in pair_nodes:
                node, idx, freqs = p
                if node in deleted_nodes:
                    num_deleted_nodes += 1
                    continue
                    
                next_node = node.next
                                
                if next_node == None or next_node in deleted_nodes:
                    assert(False)

                if next_node.data != second_token:
                    num_deleted_nodes += 1
                    continue
                
                assert(node.data == first_token and next_node.data == second_token)
                
                not_deleted += 1
                
                if node != self.sentences_in_list[idx].begin():
                    prev_node = node.prev
                    update_data_structures(prev_node, node, -freqs)
                    
                update_data_structures(node, next_node, -freqs)
                
                if next_node != self.sentences_in_list[idx].back_iter():
                    next_next_node = next_node.next
                    update_data_structures(next_node, next_next_node, -freqs)
                    
                merged = first_token + second_token
                
                next_next_node = next_node.next
                deleted_nodes.add(node)
                deleted_nodes.add(next_node)
                self.sentences_in_list[idx].erase(node)
                self.sentences_in_list[idx].erase(next_node)
                node = self.sentences_in_list[idx].insert(next_next_node, merged)
                
                if node != self.sentences_in_list[idx].begin():
                    prev_node = node.prev
                    update_data_structures(prev_node, node, +freqs)
                    self.tokens_pair_to_nodes_mapper[prev_node.data][node.data].append((prev_node, idx, freqs))
                    
                if node != self.sentences_in_list[idx].back_iter():
                    next_node = node.next
                    update_data_structures(node, next_node, +freqs)
                    self.tokens_pair_to_nodes_mapper[node.data][next_node.data].append((node, idx, freqs))
            assert(len(pair_nodes) == not_deleted + num_deleted_nodes)

    def train(self, cleaned_dataset, iters=1000):
        self.init_set = self.load_init_tokens_set("langauge-related-utils/arabic.json")
        self.tokens = self.init_set.copy()
        self.init_data_structures(cleaned_dataset)
        self.sorting_data_structures_method(iters)
        self.save_tokens()
        print("The end of the training function")
        
    import json

    def load_init_tokens_set(self, path: str) -> set[str]:
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
            for letter_json in content["letters"]:
                if isinstance(letter_json, str):
                    letters.add(letter_json)
        else:
            print("JSON does not contain 'letters' array.")

        return letters

    def tokenize(self, text):
        if not self.has_tokens():
            self.load_tokens()

        sentences_in_list = []
        tokens_pair_to_nodes_mapper = defaultdict(lambda: defaultdict(list))
        
        previous = None
        word_list = LinkedList()
        for ch in text:
            if ch == ' ':
                if not word_list.empty():
                    sentences_in_list.append(word_list)
                word_list = LinkedList()
                previous = None
            else:
                node = word_list.push_back(ch)
                if previous:
                    tokens_pair_to_nodes_mapper[previous][ch].append((node.prev, len(sentences_in_list)))
                previous = ch
        if not word_list.empty():
            sentences_in_list.append(word_list)
                
        deleted_nodes = set()
                
        for first_token, second_token in self.ordered_added_tokens_splitted:
            pair_nodes = tokens_pair_to_nodes_mapper[first_token][second_token]
            for p in pair_nodes:
                node, idx = p

                if node in deleted_nodes:
                    continue

                next_node = node.next
                if next_node == None or next_node in deleted_nodes:
                    continue

                if next_node.data != second_token:
                    continue

                assert node.data == first_token and next_node.data == second_token

                merged = first_token + second_token
                next_next_node = next_node.next
                deleted_nodes.add(node)
                deleted_nodes.add(next_node)
                sentences_in_list[idx].erase(node)
                sentences_in_list[idx].erase(next_node)
                node = sentences_in_list[idx].insert(next_next_node, merged)

                if node != sentences_in_list[idx].begin():
                    prev_node = node.prev
                    tokens_pair_to_nodes_mapper[prev_node.data][node.data].append((prev_node, idx))

                if node != sentences_in_list[idx].back_iter():
                    next_node = node.next
                    tokens_pair_to_nodes_mapper[node.data][next_node.data].append((node, idx))
                    
        result = []
        for word_list in sentences_in_list:
            result += word_list.export_as_list() + [' ']
        if result[-1] == ' ': result.pop()
        return result

    def detokenize(self, tokens):
        return ''.join(tokens)
