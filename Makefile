all: grep

grep: src/main_grep.o src/grep_keys.o src/common/parsing.o
	gcc -Wall -Wextra -Werror -std=c11 -g src/main_grep.o src/grep_keys.o src/common/parsing.o -o grep
	rm -f src/*.o src/common/*.o

src/main_grep.o: src/main_grep.c
	gcc -Wall -Wextra -Werror -std=c11 -g -c src/main_grep.c -o src/main_grep.o

src/grep_keys.o: src/grep_keys.c
	gcc -Wall -Wextra -Werror -std=c11 -g -c src/grep_keys.c -o src/grep_keys.o

src/common/parsing.o: src/common/parsing.c
	gcc -Wall -Wextra -Werror -std=c11 -g -c src/common/parsing.c -o src/common/parsing.o

clean:
	rm -f src/*.o src/common/*.o grep

rebuild: clean all

.PHONY: all clean rebuild