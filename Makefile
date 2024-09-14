CC := clang
CFLAGS := -g -Wall -Wextra -Wpedantic -Werror -Iraylib/src 
SOURCES := main.c common.c chunk.c
LIBS := -Lraylib/src -lraylib -lm
OBJS :=$(SOURCES:.c=.o)

main: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
 

.PHONY: clean
clean:
	rm -rf *.o main