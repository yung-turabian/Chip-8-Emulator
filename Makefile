CC=gcc
LIBS=-lSDL2main -lSDL2 -lGL -lGLU -lGLEW -lreis -lm
SRCS=src/main.c
OBJS=$(patsubst src/%.c,src/%.o,$(SRCS))
DEPS=$(patsubst src/%.c,src/%.d,$(SRCS))
BIN_NAME=chipper
#WARNING= 
#-Wall -Wextra
CCFLAGS= -DDEBUG -ggdb -g -pg -O0

.PHONY: all clean

all: $(BIN_NAME)

clean:
	rm $(OBJS) $(DEPS) $(BIN_NAME)

$(BIN_NAME): $(OBJS)
	$(CC) $(WARNING) $(CCFLAGS) $^ -o $@ $(LIBS)

-include $(DEPS)

%.o: %.c Makefile
	$(CC) $(WARNING) $(CCFLAGS) -MMD -MP -c $< -o $@
