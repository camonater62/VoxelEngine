CC := gcc
CFLAGS := -g -Wall -Wextra -pedantic -Werror
SOURCES := main.c common.c
LIBS := -I./raylib/src -Lraylib/src -lraylib -lm
OBJS :=$(SOURCES:.c=.o)

main: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
 

.PHONY: clean
clean:
	rm -rf *.o main