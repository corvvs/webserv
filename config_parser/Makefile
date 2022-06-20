NAME := a.out
# SRCS := test.cpp \
# 		ConfigParser.cpp

SRCS := Lexer.cpp \
		test_common.cpp \
		main.cpp

OBJS := $(SRCS:.cpp=.o)
all: $(NAME)
CXXFLAGS := -Wall -Wextra -Werror -g -fsanitize=address -std=c++98

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)
	rm -rf $(NAME).dSYM

re: fclean all

t: test

test: $(NAME)
	./$(NAME)
