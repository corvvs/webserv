NAME		=	webserv
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror
GTEST_FLAGS	= 	./$(TESTS_DIR)/googletest/include -I./$(TESTS_DIR)/googletest -pthread

SRC_DIR		=	./src
SRC_MAIN	=	./src/main.cpp
OBJ_DIR		=	./obj
DPS_DIR		=	./dps

OBJ_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(OBJ_DIR)/{})
DPS_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(DPS_DIR)/{})
SRCS			=	$(shell find $(SRC_DIR) -type f -name "*.cpp" -not -name "main.cpp")
OBJS			=	$(addprefix $(OBJ_DIR)/,  $(SRCS:.cpp=.o))
DPS				=	$(addprefix $(DPS_DIR)/,  $(SRCS:.cpp=.d))

TESTER					=	./unit_tester
TESTS_DIR				=	./tests
TEST_CASE_SRC_DIR		=	./test_case
TEST_CASE_OBJ_DIR		=	./test_case_obj
TEST_CASE_DPS_DIR		=	./test_case_dps
GTEST_FLAGS				=	-isystem ./$(TESTS_DIR)/googletest/include -I./$(TESTS_DIR)/googletest -pthread
GTEST_LIB				=	$(TESTS_DIR)/gtest_main.a

TEST_CASE_OBJ_DIRS		=	$(shell find  $(TEST_CASE_SRC_DIR) -type d | xargs -I{} echo $(TEST_CASE_OBJ_DIR)/{})
TEST_CASE_DPS_DIRS		=	$(shell find  $(TEST_CASE_SRC_DIR) -type d | xargs -I{} echo $(TEST_CASE_DPS_DIR)/{})
TEST_CASE_SRCS			=	$(shell find $(TEST_CASE_SRC_DIR) -type f -name "*.cpp")
TEST_CASE_OBJS			=	$(addprefix $(TEST_CASE_OBJ_DIR)/,  $(TEST_CASE_SRCS:.cpp=.o))
TEST_CASE_DPS			=	$(addprefix $(TEST_CASE_DPS_DIR)/,  $(TEST_CASE_SRCS:.cpp=.d))

.PHONY: all
all: $(OBJ_DIRS) $(DPS_DIRS) $(NAME)

-include $(DPS)

$(NAME): $(OBJS) $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(SRC_MAIN)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -MMD -MP -MF $(DPS_DIR)/$(*).d -c $< -o $@

$(TEST_CASE_OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS)  $(GTEST_FLAGS) -MMD -MP -MF $(TEST_CASE_DPS_DIR)/$(*).d -c $< -o $@

$(OBJ_DIRS):
	mkdir -p $(OBJ_DIRS)

$(DPS_DIRS):
	mkdir -p $(DPS_DIRS)

$(TEST_CASE_OBJ_DIRS):
	mkdir -p $(TEST_CASE_OBJ_DIRS)

$(TEST_CASE_DPS_DIRS):
	mkdir -p $(TEST_CASE_DPS_DIRS)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(DPS_DIR) $(TEST_CASE_OBJ_DIR) $(TEST_CASE_DPS_DIR) $(TESTER)
	make -C $(TESTS_DIR) clean

.PHONY: fclean
fclean: clean
	rm -f $(NAME)
	make -C $(TESTS_DIR) fclean

.PHONY: re
re: fclean all

$(GTEST_LIB):
	make -C $(TESTS_DIR)

$(TESTER): $(OBJ_DIRS) $(DPS_DIRS) $(OBJS) $(TEST_CASE_OBJ_DIRS) $(TEST_CASE_DPS_DIRS) $(TEST_CASE_OBJS) $(GTEST_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(GTEST_LIB)  $(OBJS) $(TEST_CASE_OBJS)

.PHONY: run_test
run_test: $(TESTER)
	$(TESTER)

.PHONY: up
up:	down all
	./$(NAME) &

.PHONY: down
down:
	pkill $(NAME) || :

