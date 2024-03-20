$(shell mkdir -p build)

all:
	gcc main.c -o build/main -lraylib -lm

release:
	gcc -O3 -DRELEASE main.c -o build/main -lraylib -m

run: all
	./build/main
