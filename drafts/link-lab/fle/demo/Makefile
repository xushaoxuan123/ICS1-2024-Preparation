CC := ./cc
LD := ./ld

CFLAGS := -Wall -g -Os

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

# Build and link hello (fle)
hello: $(OBJS) tools
	$(LD) $(SRCS:.c=.fle) -o $@

%.o: %.c tools
	$(CC) $(CFLAGS) $< -o $@

# build/FLETools is all the binutils!
tools:
	@ln -f -s ../build/FLETools cc
	@ln -f -s ../build/FLETools ld
	@ln -f -s ../build/FLETools exec
	@ln -f -s ../build/FLETools objdump
	@ln -f -s ../build/FLETools readfle
	@ln -f -s ../build/FLETools nm

clean:
	rm -f $(OBJS) $(SRCS:.c=.fle) cc ld exec exec objdump readfle nm hello

.PHONY: all clean tools
