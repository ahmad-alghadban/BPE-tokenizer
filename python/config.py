"""Central configuration: single source of truth for paths and defaults.

Package resources (the Arabic letter set and the trained token files) ship with
the code, so they are resolved relative to THIS file rather than the current
working directory — the project therefore runs correctly from any directory.

Run-specific values (dataset input, output location, line caps, merge count)
are defaults only; the CLI scripts expose them as overridable arguments.
"""

from pathlib import Path

# Repo root (directory containing this file).
PACKAGE_ROOT = Path(__file__).resolve().parent

# --- Package-internal resources (not meant to be configured) ---
ARABIC_JSON = PACKAGE_ROOT / "langauge-related-utils" / "arabic.json"

TOKENS_DIR = PACKAGE_ROOT / "BPE_tokenizer"
SPLITTED_TOKENS_FILE = TOKENS_DIR / "splitted_tokens_ordered.txt"
TOKENS_FILE = TOKENS_DIR / "tokens_ordered.txt"

# --- Run-specific defaults (overridable via CLI) ---
DATASETS_DIR = PACKAGE_ROOT / "datasets"
TRAIN_INPUT = DATASETS_DIR / "arabic_1b_words" / "arabic_1b_words.txt"
EVAL_INPUT = (
    DATASETS_DIR
    / "wiki-arabic-v2-dataset"
    / "arabic-wiki-simple-dataset"
    / "wiki-v2.jsonl"
)

TEMP_DIR = PACKAGE_ROOT / "temp"
CLEANED_DATASET_OUTPUT = TEMP_DIR / "cleaned_data_set.txt"

TRAIN_MAX_LINES = 100_000
EVAL_MAX_LINES = 10_000
NUM_MERGES = 1 << 16
