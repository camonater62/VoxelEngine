CC := clang
CFLAGS := -g -O0 -Wall -Wextra -Wpedantic -Werror -Iraylib/src -Iraylib/src/external -Iexternal
SOURCES := main.c common.c chunk.c world.c stb_ds.c
LIBS := -Lraylib/src -lraylib -lm
OBJS :=$(SOURCES:.c=.o)

main: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
 

.PHONY: clean
clean:
	rm -rf *.o main