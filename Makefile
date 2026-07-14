NAME		= webserv

CC			= c++
CFLAGS		= -Wall -Werror -Wextra -std=c++98
INCLUDES	= -Iincludes

OBJ_DIR		= objects
SRC_DIR		= src

SRC			= main.cpp ServerConfig.cpp parseConfig.cpp
OBJ			= $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

ARG			= config/default.conf

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

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

re: fclean all

.PHONY: all clean fclean re run vrun
