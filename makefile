cflags-obj=-Wall -Wextra -Werror -Iinclude -g
cc=gcc
build=build/
target=build/vi.out

# These will be in the form of `./src/vga/vga.c ./kernel/mm/kmalloc.c ./kernel/mm/virt-mm.c'
c-src=$(shell find . -name '*.c')

# These will be in the form of `build/vga.o build/kernel.o build/phy-mm.o ...'
c-obj=$(addprefix $(build), $(subst .c,.o, $(notdir $(c-src))))	 

# Collection of all objects
obj=$(c-obj)

# set search path for %.c and %.s wildcards to the subdirs of sources
vpath %.c $(dir $(c-src))

run: main
	./build/./vi.out

main: clean $(obj) 
	$(cc) -o $(target) $(cflags-obj) $(obj)

debug: main
	cd ./build/ && valgrind -s --log-file=log --leak-check=full --track-origins=yes ./vi.out
	cat ./build/log

build/%.o : %.c
	$(cc) $< $(cflags-obj) -c -o $@

clean: 
	rm -rf build
	mkdir build
