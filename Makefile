CC		?= clang
WARN	:= -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
			-Wstrict-prototypes -Wmissing-prototypes \
			-Wmissing-variable-declarations -Wcast-align \
			-Wundef -Wformat=2 -Wwrite-strings -Wnull-dereference
CFLAGS	?= -O2 $(WARN)
LDFLAGS	:= -framework ApplicationServices -framework CoreServices

SRC := winman.c
OBJ := $(SRC:.c=.o)

winman: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f winman $(OBJ)
.PHONY: clean
