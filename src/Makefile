# add source files here
SRCS := main.c

# set the compiler
CC := cc

# set the compiler flags
CFLAGS = -I$(INC_DIR), -lncurses -lm

# add header files here
HDRS :=

# generate names of object files
OBJS := $(SRCS:.c=.o)

# name of executable
EXEC := exe

# default recipe
all: $(EXEC)

# recipe for building the final executable
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)
	
# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#	$(CC) -o $@ $(@:.o=.c) -c $(CFLAGS)

# recipe to clean the workspace
clean:
	rm -f $(OBJS) #$(EXEC)

.PHONY: all clean
