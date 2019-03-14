CC = gcc
CFLAGS = -g -lreadline -Wall -Werror -std=c11
TARGETS = tosh siesta

# add to this list if you create new .c source files
TOSH_SRC = tosh.c history_queue.c parse_args.c

all: $(TARGETS)

tosh: $(TOSH_SRC) 
	$(CC) $(CFLAGS) -o $@ $(TOSH_SRC)

siesta: siesta.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(TARGETS)
