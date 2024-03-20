$(shell mkdir -p build)

all:
	gcc -Wall -Wextra main.c -o build/main -lraylib -lm

release:
	gcc -Wall -Wextra -O3 -DRELEASE main.c -o build/main -lraylib -lm

run: all
	./build/main
