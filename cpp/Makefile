# Build script for the BPE-tokenizer project.
#
#   make            # build both programs (train and tokenize)
#   make train      # build only the trainer
#   make tokenize   # build only the tokenizer
#   make clean      # remove build artifacts
#   make CXXFLAGS='-std=c++17 -g -Iinclude'   # debug build (override flags)
#
# Run the programs from this directory, since paths in config.json are relative.

CXX      := g++
# -MMD -MP auto-generate header dependency files so touching a .hpp rebuilds
# only the .o files that include it.
CXXFLAGS := -std=c++17 -O2 -Iinclude -MMD -MP

# Sources shared by both programs.
COMMON_SRCS := utils/text_preprocessing.cpp \
               utils/config.cpp \
               utils/utils.cpp \
               BPE_tokenizer/BPE_tokenizer.cpp
COMMON_OBJS := $(COMMON_SRCS:.cpp=.o)

PROGRAMS := train tokenize
OBJS     := $(COMMON_OBJS) train.o tokenize.o
DEPS     := $(OBJS:.o=.d)

.PHONY: all clean

all: $(PROGRAMS)

train: train.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

tokenize: tokenize.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Pattern rule: compile any .cpp into its .o.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(PROGRAMS) $(OBJS) $(DEPS)

# Pull in the auto-generated header dependencies (ignored if not yet present).
-include $(DEPS)
