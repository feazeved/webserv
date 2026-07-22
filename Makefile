# Configuration ------------------------------- #
NAME = webserv
VPATH = src test
MAIN_SRC = main.cpp
CORE_SRC = parseConfig.cpp
TEST_SRC = test_main.cpp test_parseConfig.cpp
LDLIBS =
ARG = config/default.conf

# Defaults ------------------------------------ #
RM := rm -f
BUILD_PATH = build
INC_PATH = includes src test src/http
OBJ_PATH = $(BUILD_PATH)/obj
BIN = build/$(NAME)
TEST_BIN = $(BIN)_test
OBJ_MAIN = $(addprefix $(OBJ_PATH)/, $(MAIN_SRC:.cpp=.o))
OBJ_CORE = $(addprefix $(OBJ_PATH)/, $(CORE_SRC:.cpp=.o))
OBJ_TEST = $(addprefix $(OBJ_PATH)/, $(TEST_SRC:.cpp=.o))

# Flags --------------------------------------- #
CXX = clang++
CPPFLAGS = $(addprefix -I,$(INC_PATH))
CXXFLAGS = -Wall -Wextra -std=c++98
CXXFLAGS_TEST = -Wall -Wextra -std=c++11 -g
LDFLAGS =
DEBUG = -g -Wpedantic -Wcast-qual -Wfloat-equal -Wswitch-default -Wsign-conversion
ASAN = -fsanitize=address,undefined,leak -fno-omit-frame-pointer
TSAN = -fsanitize=thread -fno-omit-frame-pointer
FAST = -march=native -O3 -ffast-math -fstrict-aliasing

# Pattern Rules: Compilation ------------------ #
$(OBJ_PATH)/%.o: %.cpp | $(OBJ_PATH)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(OBJ_TEST): CXXFLAGS = $(CXXFLAGS_TEST)

# Linking
$(BIN): $(OBJ_MAIN) $(OBJ_CORE) | $(BUILD_PATH)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ_MAIN) $(OBJ_CORE) $(LDLIBS)

$(TEST_BIN): $(OBJ_TEST) $(OBJ_CORE) | $(BUILD_PATH)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ_TEST) $(OBJ_CORE) $(LDLIBS)

# Directory
$(OBJ_PATH):
	@mkdir -p $@
$(BUILD_PATH):
	@mkdir -p $@

# Phonies ------------------------------------- #
all: $(BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

run: re
	clear
	./$(BIN) $(ARG)

vrun: re
	clear
	valgrind ./$(BIN) $(ARG)

compdb: | $(BUILD_PATH)
	$(RM) $(BUILD_PATH)/compile_commands.json
	bear --output $(BUILD_PATH)/compile_commands.json -- $(MAKE) clean asan

clean:
	$(RM) -r $(OBJ_PATH)

fclean: clean
	$(RM) $(BIN)
	$(RM) $(TEST_BIN)

re: fclean all

debug: CXXFLAGS += $(DEBUG)
debug: clean $(BIN)

asan: CXXFLAGS += $(DEBUG) $(ASAN)
asan: LDFLAGS += $(ASAN)
asan: clean $(BIN)

tsan: CXXFLAGS += $(DEBUG) $(TSAN)
tsan: LDFLAGS += $(TSAN)
tsan: clean $(BIN)

fast: CXXFLAGS += $(FAST)
fast: LDFLAGS += -flto
fast: clean $(BIN)

ffast: CXXFLAGS += $(FAST) -Ofast
ffast: LDFLAGS += -flto
ffast: clean $(BIN)

.PHONY: all test run vrun compdb clean fclean re debug asan tsan fast ffast
