NAME := a.out
# SRCS := test.cpp \
# 		ConfigParser.cpp

SRCS := test_common.cpp \
		Lexer.cpp \
		Parser.cpp \
		Analyzer.cpp \
		main.cpp

OBJS := $(SRCS:.cpp=.o)
CXXFLAGS := -Wall -Wextra -Werror -g3 -fsanitize=address -std=c++98

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)
	rm -rf $(NAME).dSYM

re: fclean all

d: debug
debug: CXXFLAGS += -DDEBUG=1
debug: re test

t: test
test: $(NAME)
	./$(NAME)

.PHONY: all clean fclean re
.PHONY: test debug
