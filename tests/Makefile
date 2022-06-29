NAME		=	./tester
CXX			=	c++
CXXFLAGS	=	-isystem ./googletest/include -I./googletest -Wall -Wextra -pthread

TEST_SRC_DIR		=	./src
TEST_OBJ_DIR		=	./obj
TEST_DPS_DIR		=	./dps

OBJ_DIR				=	../obj
SRC_LIB				=	./src.a

OBJ_DIRS	=	$(shell find  $(TEST_SRC_DIR) -type d | xargs -I{} echo $(TEST_OBJ_DIR)/{})
DPS_DIRS	=	$(shell find  $(TEST_SRC_DIR) -type d | xargs -I{} echo $(TEST_DPS_DIR)/{})

GOOGLE_TEST	=	googletest
GTEST_LIB	=	gtest_main.a
SRCS		=	$(shell find $(TEST_SRC_DIR) -type f -name "*.cpp")
OBJS		=	$(addprefix $(TEST_OBJ_DIR)/,  $(SRCS:.cpp=.o))
DPS			=	$(addprefix $(TEST_DPS_DIR)/,  $(SRCS:.cpp=.d))

.PHONY: all
all: $(NAME)

-include $(DPS)

$(NAME): $(GOOGLE_TEST) $(GTEST_LIB) $(SRC_LIB) $(OBJ_DIRS) $(DPS_DIRS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(GTEST_LIB) $(SRC_LIB)

$(TEST_OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(TEST_DPS_DIR)/$(*).d -c $< -o $@

$(SRC_LIB):
	make -C ..
	find $(OBJ_DIR) -type f -name '*.o' -not -name 'main.o' | xargs -I{} $(AR) $(ARFLAGS) $(SRC_LIB) {}


$(OBJ_DIRS):
	mkdir -p $(OBJ_DIRS)

$(DPS_DIRS):
	mkdir -p $(DPS_DIRS)

$(GTEST_LIB):
	$(CXX) $(CXXFLAGS) -c ./googletest/src/gtest-all.cc
	$(CXX) $(CXXFLAGS) -c ./googletest/src/gtest_main.cc
	$(AR) $(ARFLAGS) $@ gtest-all.o gtest_main.o

$(GOOGLE_TEST):
	git clone -b release-1.7.0 https://github.com/google/googletest.git

.PHONY: clean
clean:
	rm -rf $(TEST_OBJ_DIR) $(TEST_DPS_DIR) $(GTEST_LIB)  $(SRC_LIB) gtest-all.o  gtest_main.o

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all

