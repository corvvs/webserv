NAME		=	webserv
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror

SRC_DIR		=	./src
SRC_MAIN	=	./src/main.cpp
OBJ_DIR		=	./obj
DPS_DIR		=	./dps

OBJ_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(OBJ_DIR)/{})
DPS_DIRS		=	$(shell find  $(SRC_DIR) -type d | xargs -I{} echo $(DPS_DIR)/{})
SRCS			=	$(shell find $(SRC_DIR) -type f -name "*.cpp" -not -name "main.cpp")
OBJS			=	$(addprefix $(OBJ_DIR)/,  $(SRCS:.cpp=.o))
DPS				=	$(addprefix $(DPS_DIR)/,  $(SRCS:.cpp=.d))

.PHONY: all
all: $(OBJ_DIRS) $(DPS_DIRS) $(NAME)

-include $(DPS)

$(NAME): $(OBJS) $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(SRC_MAIN)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -MMD -MP -MF $(DPS_DIR)/$(*).d -c $< -o $@

$(OBJ_DIRS):
	mkdir -p $(OBJ_DIRS)

$(DPS_DIRS):
	mkdir -p $(DPS_DIRS)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(DPS_DIR)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all

