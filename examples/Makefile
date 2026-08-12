EXAMPLE_FILE ?= helloworld.c

all: diylib build
	
build: diylib
	gcc -I../vendored/redlib -g -DCROSS -std=gnu99  $(EXAMPLE_FILE) ../diylib.a -lglfw -lc -lm -lvulkan -o example
	chmod +x example

run: build
	./example

diylib:
	make -C ../

clean:
	make -C .. clean
	rm example