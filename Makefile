CC := clang
CFLAGS := -g -Wall -Wextra -pedantic -Werror
SOURCES := main.c common.c
LIBS := -lraylib
OBJS :=$(SOURCES:.c=.o)

main: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
 

.PHONY: clean
clean:
	rm -rf *.o main