NAME=libnocta.a
CC=gcc
CFLAGS=-std=gnu99 -g
VPATH=src
SOURCES=engine.c minipack.c unit.c gainer.c bqfilter.c svfilter.c delay.c
OBJECTS=$(SOURCES:.c=.o)

all: $(NAME)

$(NAME): $(OBJECTS)
	rm -f $(NAME)
	ar rcs $(NAME) $(OBJECTS)

clean:
	rm -f *.o $(NAME)
