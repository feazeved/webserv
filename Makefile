NAME		= webserv
TEST_NAME	= test_runner

CC			= c++
CFLAGS		= -Wall -Werror -Wextra -std=c++98 -g
TEST_FLAGS	= -Wall -Wextra -std=c++11 -g
INCLUDES	= -Iincludes -I. -Itest -Isrc

SRC_DIR		= src
TEST_DIR	= test
OBJ_DIR		= objects

MAIN_SRC	= main.cpp
CORE_SRC	= parseConfig.cpp Server.cpp ServerManager.cpp
TEST_SRC	= test_main.cpp test_parseConfig.cpp

OBJ_CORE	= $(addprefix $(OBJ_DIR)/, $(CORE_SRC:.cpp=.o))
OBJ_MAIN	= $(addprefix $(OBJ_DIR)/, $(MAIN_SRC:.cpp=.o))
OBJ_TEST	= $(addprefix $(OBJ_DIR)/, $(TEST_SRC:.cpp=.o))

ARG			= config/default.conf

all: $(NAME)

$(NAME): $(OBJ_CORE) $(OBJ_MAIN)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ_CORE) $(OBJ_MAIN) -o $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(TEST_FLAGS) $(INCLUDES) -c $< -o $@

test_bin: $(OBJ_CORE) $(OBJ_TEST)
	$(CC) $(TEST_FLAGS) $(INCLUDES) $(OBJ_CORE) $(OBJ_TEST) -o $(TEST_NAME)

test: test_bin
	./$(TEST_NAME)

run:
	make re
	clear
	./$(NAME) $(ARG)

vrun:
	make re
	clear
	valgrind ./$(NAME) $(ARG)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)
	rm -f $(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re run vrun test
