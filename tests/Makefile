NAME		=	./tests
BUILD_DIR	= ./build
LIB			= ./lib

.PHONY: all
all:  $(NAME)

$(NAME): $(BUILD_DIR) $(LIB)
	cd $(BUILD_DIR) && \
	cmake .. && \
	make -C . && \
	$(NAME)


$(BUILD_DIR):
	mkdir build

$(LIB):
	git clone https://github.com/google/googletest.git lib

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: fclean
fclean: clean
