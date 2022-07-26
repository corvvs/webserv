.PHONY: all
all:
	make -f ./src.mk

.PHONY: run_test
run_test:
	make -f ./tester.mk

.PHONY: clean
clean:
	make -f ./tester.mk tester_clean
	make -f ./src.mk clean

.PHONY: fclean
fclean: clean
	make -f ./tester.mk tester_fclean
	make -f ./src.mk fclean

.PHONY: re
re: fclean all

.PHONY: up
up:
	make -f ./src.mk up

.PHONY: down
down:
	make -f ./src.mk down

