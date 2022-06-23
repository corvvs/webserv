NAME := a.out
# SRCS := test.cpp \
# 		ConfigParser.cpp

SRCS := test_common.cpp \
		Lexer.cpp \
		Parser.cpp \
		Analyzer.cpp \
		main.cpp

OBJS := $(SRCS:.cpp=.o)
all: $(NAME)
CXXFLAGS := -Wall -Wextra -Werror -g3 -fsanitize=address -std=c++98

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
