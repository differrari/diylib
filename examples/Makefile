EXAMPLE_FILE ?= helloworld.c

all: diylib
	
build:
	gcc $(EXAMPLE_FILE) -DCROSS -std=c99 -I../vendored/redlib ../vendored/redlib/clibshared.a ../diylib.a ../vendored/raylib/src/libraylib.a -lc -lm -o example
	chmod +x example

run: build
	./example

diylib:
	make -C ../