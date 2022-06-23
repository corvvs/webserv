NAME		=	./tests
CXX			=	c++
CXXFLAGS	=	-isystem ./googletest/include -I./googletest -Wall -Wextra -pthread

SRC_DIR		=	./src
OBJ_DIR		=	./obj
DPS_DIR		=	./dps

OBJ_DIRS	=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(OBJ_DIR)/{})
DPS_DIRS	=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(DPS_DIR)/{})

GOOGLE_TEST	=	googletest
GTEST_LIB	=	gtest_main.a
SRCS		=	$(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS		=	$(addprefix $(OBJ_DIR)/,  $(SRCS:.cpp=.o))
DPS			=	$(addprefix $(DPS_DIR)/,  $(SRCS:.cpp=.d))
-include $(DPS)

.PHONY: all
all: $(NAME)

$(NAME): $(GOOGLE_TEST) $(GTEST_LIB) $(OBJ_DIRS) $(DPS_DIRS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(GTEST_LIB)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(DPS_DIR)/$(*).d -c $< -o $@


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
	rm -rf $(OBJ_DIR) $(DPS_DIR) $(GTEST_LIB)  gtest-all.o  gtest_main.o

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all

