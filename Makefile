NAME=libnocta.a
CC=gcc
CFLAGS=-std=gnu99 -g
VPATH=src
SOURCES=engine.c unit.c gainer.c filter.c
OBJECTS=$(SOURCES:.c=.o)

all: $(NAME)

$(NAME): $(OBJECTS)
	rm -f $(NAME)
	ar rcs $(NAME) $(OBJECTS)

clean:
	rm *.o $(NAME)
