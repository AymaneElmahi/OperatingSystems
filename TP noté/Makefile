# faire 'make' pour compiler et 'make test' pour tester
CFLAGS = -g -Wall -Wextra -Werror -fsanitize=address -pthread
tp: tp.c
test: tp test1.txt test2.txt
	./tp 2 < test1.txt
	./tp 2 < test2.txt
