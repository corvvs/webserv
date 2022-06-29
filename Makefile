NAME		=	webserv
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror

SRC_DIR		=	./src
OBJ_DIR		=	./obj
DPS_DIR		=	./dps
TESTS_DIR	=	./tests

OBJ_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(OBJ_DIR)/{})
DPS_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(DPS_DIR)/{})

SRCS		=	$(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS		=	$(addprefix $(OBJ_DIR)/,  $(SRCS:.cpp=.o))
DPS			=	$(addprefix $(DPS_DIR)/,  $(SRCS:.cpp=.d))

.PHONY: all
all: $(OBJ_DIRS) $(DPS_DIRS) $(NAME)

-include $(DPS)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -MMD -MP -MF $(DPS_DIR)/$(*).d -c $< -o $@


$(OBJ_DIRS):
	mkdir -p $(OBJ_DIRS)

$(DPS_DIRS):
	mkdir -p $(DPS_DIRS)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(DPS_DIR)
	make -C $(TESTS_DIR) clean

.PHONY: fclean
fclean: clean
	rm -f $(NAME)
	make -C $(TESTS_DIR) fclean

.PHONY: re
re: fclean all

.PHONY: test
test:
	make -C $(TESTS_DIR)

.PHONY: run_test
run_test: test
	./tests/tester

