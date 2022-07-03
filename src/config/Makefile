NAME := a.out
# SRCS := test.cpp \
# 		ConfigParser.cpp

SRCS := ../utils/test_common.cpp \
		Lexer.cpp \
		Parser.cpp \
		Validator.cpp \
		File.cpp \
		main.cpp

OBJS := $(SRCS:.cpp=.o)
INC := ../utils
CXXFLAGS := -Wall -Wextra -Werror -g3 -I $(INC) -fsanitize=address -std=c++98

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
debug: CXXFLAGS += -DNDEBUG=1
debug: re test

t: test
test: $(NAME)
	./$(NAME)

.PHONY: all clean fclean re
.PHONY: test debug
