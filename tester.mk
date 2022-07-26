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
all: $(TESTER)
	$(TESTER)

-include ./src.mk

$(TEST_CASE_OBJ_DIR)/%.o: %.cpp
	$(CXX) $(GTEST_FLAGS) -MMD -MP -MF $(TEST_CASE_DPS_DIR)/$(*).d -c $< -o $@

$(TEST_CASE_OBJ_DIRS):
	mkdir -p $(TEST_CASE_OBJ_DIRS)

$(TEST_CASE_DPS_DIRS):
	mkdir -p $(TEST_CASE_DPS_DIRS)

$(GOOGLE_TEST):
	git clone -b release-1.7.0 https://github.com/google/googletest.git

$(GTEST_LIB): $(GOOGLE_TEST)
	$(CXX) $(GTEST_FLAGS) -c ./googletest/src/gtest-all.cc
	$(CXX) $(GTEST_FLAGS) -c ./googletest/src/gtest_main.cc
	$(AR) $(ARFLAGS) $@ gtest-all.o gtest_main.o
	rm -rf gtest-all.o  gtest_main.o

$(TESTER): $(OBJ_DIRS) $(DPS_DIRS) $(OBJS) $(GTEST_LIB) $(TEST_CASE_OBJ_DIRS) $(TEST_CASE_DPS_DIRS) $(TEST_CASE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(GTEST_LIB)  $(OBJS) $(TEST_CASE_OBJS)

.PHONY: tester_clean
tester_clean:
	rm -rf $(TEST_CASE_OBJ_DIR) $(TEST_CASE_DPS_DIR) $(GTEST_LIB)

.PHONY: tester_fclean
tester_fclean: tester_clean
	rm -f $(TESTER)

.PHONY: re
re: tester_fclean all