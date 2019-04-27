# Directories (Include, Object, Library, Source)
IDIR=include
ODIR=src/obj
#LDIR=lib
SDIR=src

# Compiler Options
CC=gcc
CFLAGS=-I$(IDIR) $(LIBS) -g
#CFLAGS+= -L$(LDIR)

#All
#LIBS +=-lm -pthread -ldl

# Dependencies and Objects lists
_DEPS = kgbash.h types.h input.h definitions.h cmds.h
DEPS  = $(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ = kgbash.o input.o cmds.o
OBJ  = $(patsubst %,$(ODIR)/%,$(_OBJ))

# Compile all C objects
$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Compile executable
kgbash: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

# Clean
.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
