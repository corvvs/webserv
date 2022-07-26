NAME		=	webserv
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror

SRC_DIR		=	./src
SRC_MAIN	=	./src/main.cpp
OBJ_DIR		=	./src_obj
DPS_DIR		=	./src_dps

OBJ_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(OBJ_DIR)/{})
DPS_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(DPS_DIR)/{})
SRCS			=	$(shell find $(SRC_DIR) -type f -name "*.cpp" -not -name "main.cpp")
OBJS			=	$(addprefix $(OBJ_DIR)/,  $(SRCS:.cpp=.o))
DPS				=	$(addprefix $(DPS_DIR)/,  $(SRCS:.cpp=.d))

TESTER					=	./unit_tester
TEST_CASE_SRC_DIR		=	./test_case
TEST_CASE_OBJ_DIR		=	./test_case_obj
TEST_CASE_DPS_DIR		=	./test_case_dps
GOOGLE_TEST				=	./googletest
GTEST_FLAGS				=	-isystem ./googletest/include -I./googletest -Wall -Wextra -pthread
GTEST_LIB				=	./gtest_main.a

TEST_CASE_OBJ_DIRS		=	$(shell find  $(TEST_CASE_SRC_DIR) -type d | xargs -I{} echo $(TEST_CASE_OBJ_DIR)/{})
TEST_CASE_DPS_DIRS		=	$(shell find  $(TEST_CASE_SRC_DIR) -type d | xargs -I{} echo $(TEST_CASE_DPS_DIR)/{})
TEST_CASE_SRCS			=	$(shell find $(TEST_CASE_SRC_DIR) -type f -name "*.cpp")
TEST_CASE_OBJS			=	$(addprefix $(TEST_CASE_OBJ_DIR)/,  $(TEST_CASE_SRCS:.cpp=.o))
TEST_CASE_DPS			=	$(addprefix $(TEST_CASE_DPS_DIR)/,  $(TEST_CASE_SRCS:.cpp=.d))

.PHONY: all
all: $(NAME)

-include $(DPS)

$(NAME): $(OBJ_DIRS) $(DPS_DIRS) $(OBJS) $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(SRC_MAIN)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -MMD -MP -MF $(DPS_DIR)/$(*).d -c $< -o $@

$(TEST_CASE_OBJ_DIR)/%.o: %.cpp
	$(CXX) $(GTEST_FLAGS) -MMD -MP -MF $(TEST_CASE_DPS_DIR)/$(*).d -c $< -o $@

$(OBJ_DIRS):
	mkdir -p $@

$(DPS_DIRS):
	mkdir -p $@

$(TEST_CASE_OBJ_DIRS):
	mkdir -p $@

$(TEST_CASE_DPS_DIRS):
	mkdir -p $@

$(GOOGLE_TEST):
	git clone -b release-1.7.0 https://github.com/google/googletest.git

$(GTEST_LIB): $(GOOGLE_TEST)
	$(CXX) $(GTEST_FLAGS) -c ./googletest/src/gtest-all.cc
	$(CXX) $(GTEST_FLAGS) -c ./googletest/src/gtest_main.cc
	$(AR) $(ARFLAGS) $@ gtest-all.o gtest_main.o
	rm -rf gtest-all.o  gtest_main.o

$(TESTER): $(OBJ_DIRS) $(DPS_DIRS) $(OBJS) $(GTEST_LIB) $(TEST_CASE_OBJ_DIRS) $(TEST_CASE_DPS_DIRS) $(TEST_CASE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(GTEST_LIB)  $(OBJS) $(TEST_CASE_OBJS)


.PHONY: run_test
run_test: $(TESTER)
	$(TESTER)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(DPS_DIR) $(TEST_CASE_OBJ_DIR) $(TEST_CASE_DPS_DIR) $(GTEST_LIB) $(TESTER)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all

.PHONY: up
up:	down all
	./$(NAME) &

.PHONY: down
down:
	pkill $(NAME) || :

