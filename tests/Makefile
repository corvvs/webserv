NAME		=	gtest_main.a
CXX			=	c++
CXXFLAGS	=	-isystem ./googletest/include -I./googletest -Wall -Wextra -pthread

GOOGLE_TEST	=	googletest

.PHONY: all
all: $(NAME)


$(NAME): $(GOOGLE_TEST) $(GTEST_LIB)
	$(CXX) $(CXXFLAGS) -c ./googletest/src/gtest-all.cc
	$(CXX) $(CXXFLAGS) -c ./googletest/src/gtest_main.cc
	$(AR) $(ARFLAGS) $(NAME) gtest-all.o gtest_main.o

$(GOOGLE_TEST):
	git clone -b release-1.7.0 https://github.com/google/googletest.git

.PHONY: clean
clean:
	rm -rf gtest-all.o  gtest_main.o

.PHONY: fclean
fclean: clean
	rm -f $(NAME)


.PHONY: re
re: fclean all

