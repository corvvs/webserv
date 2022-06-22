NAME		=	webserv
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror

SRCDIR		=	./src
OBJDIR		=	./obj
DPSDIR		=	./dps

OBJDIRS		=	$(shell find  $(SRCDIR) -type d | xargs -I{} echo $(OBJDIR)/{})
DPSDIRS		=	$(shell find  $(SRCDIR) -type d | xargs -I{} echo $(DPSDIR)/{})

SRCS		=	$(shell find $(SRCDIR) -type f -name "*.cpp")
OBJS		=	$(addprefix $(OBJDIR)/,  $(SRCS:.cpp=.o))
DPS			=	$(addprefix $(DPSDIR)/,  $(SRCS:.cpp=.d))
-include $(DPS)

.PHONY: all
all: $(OBJDIRS) $(DPSDIRS) $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -MMD -MP -MF $(DPSDIR)/$(*).d -c $< -o $@


$(OBJDIRS):
	mkdir -p $(OBJDIRS)

$(DPSDIRS):
	mkdir -p $(DPSDIRS)

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(DPSDIR)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all
