EXAMPLE_FILE ?= helloworld.c

all: diylib build
	
build: diylib
	gcc $(EXAMPLE_FILE) -DCROSS -std=c99 -I../vendored/redlib ../vendored/redlib/clibshared.a ../diylib.a ../vendored/glfw/build/libglfw3.a -lc -lm -lGL -o example
	chmod +x example

run: build
	./example

diylib:
	make -C ../