$(shell mkdir -p build)

all:
	gcc main.c -o build/main -lraylib

release:
	gcc -O3 -DRELEASE main.c -o build/main -lraylib

run: all
	./build/main
