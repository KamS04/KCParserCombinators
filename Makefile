ifeq ($(OS), Windows_NT)
RELEASE=./builds/win/libparselib.a
DEBUG=./builds/win/libdparselib.a

OBJEXT=.w.o
DEPEXT=.w.d
TESTEXE=test/test.exe
else
RELEASE=./builds/linux/libparselib.a
DEBUG=./builds/linux/libdparselib.a

OBJEXT=.l.o
DEPEXT=.l.d

TESTEXE=test/test.a
endif

CODEDIRS=libs src src/basic src/core
INCDIRS=include/ libs/include/

OBJDIR=objbuilds
DEPDIR=depfiles

CC=gcc
AR=ar
OPT=
# automatically grab all the headers
CFLAGS=-Wall -Wextra $(foreach D,$(INCDIRS),-I$(D)) $(OPT)

# grab every *.c file in every CODEDIR
CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
# regular expression replacement
OBJECTS=$(patsubst %.c,$(OBJDIR)/%$(OBJEXT),$(CFILES))
DEPFILES=$(patsubst %.c,$(DEPDIR)/%$(DEPEXT),$(CFILES))

DEBUGOBJECTS=$(patsubst %.c,$(OBJDIR)/%.d$(OBJEXT),$(CFILES))
DEBUGDEPFILES=$(patsubst %.c,$(DEPDIR)/%.d$(DEPEXT),$(CFILES))

DEPENDENTFLAGS=-MP -MMD -MF

define depflags
	$(DEPENDENTFLAGS) $(patsubst %.c,$(DEPDIR)/%$(2),$(1))
endef

all: $(RELEASE) $(DEBUG)

test: $(TESTEXE)

$(TESTEXE): test/ack$(OBJEXT) test/korolib$(OBJEXT) $(DEBUGOBJECTS)
		$(CC) $(CFLAGS) -o $@ $^ -lm
		@echo "Success"

test/%$(OBJEXT): test/%.c
		$(CC) $(CFLAGS) $(DEPENDENTFLAGS) $(patsubst %.c,%$(DEPEXT),$<) -c -o $@ $<

ifeq ($(OS), Windows_NT)
setup:
		mkdir builds
		mkdir builds\\win
		mkdir depfiles
		mkdir depfiles\\libs
		mkdir depfiles\\src
		mkdir depfiles\\src\\basic
		mkdir depfiles\\src\\core
		mkdir objbuilds
		mkdir objbuilds\\src
		mkdir objbuilds\\src\\basic
		mkdir objbuilds\\src\\core
else
setup:
		mkdir -p builds
		mkdir -p builds/linux
		mkdir -p depfiles
		mkdir -p depfiles/libs
		mkdir -p depfiles/src
		mkdir -p depfiles/src/basic
		mkdir -p depfiles/src/core
		mkdir -p objbuilds
		mkdir -p objbuilds/libs
		mkdir -p objbuilds/src
		mkdir -p objbuilds/src/basic
		mkdir -p objbuilds/src/core
endif

$(RELEASE): $(OBJECTS)
		$(AR) rcs $@ $^
		@echo "Success Release Build"

$(OBJDIR)/%$(OBJEXT): %.c
		$(CC) $(CFLAGS) $(call depflags,$<,$(DEPEXT)) -c -o $@ $<

$(DEBUG): $(DEBUGOBJECTS)
		$(AR) rcs $@ $^
		@echo "Success Debug Build"

$(OBJDIR)/%.d$(OBJEXT): %.c
		$(CC) $(CFLAGS) $(call depflags,$<,.d$(DEPEXT)) -DDEBUG -ggdb -c -o $@ $<

ifeq ($(OS), Windows_NT)
clean:
	del  $(subst /,\\,$(RELEASE) $(DEBUG) $(OBJECTS) $(DEPFILES) $(DEBUGOBJECTS) $(DEBUGDEPFILES) test/ack$(OBJEXT) test/ack$(DEPEXT) test/korolib$(OBJEXT) test/korolib$(DEPEXT) $(TESTEXE))
else
clean:
	rm -rf $(RELEASE) $(DEBUG) $(OBJECTS) $(DEPFILES) $(DEBUGOBJECTS) $(DEBUGDEPFILES) test/ack$(OBJEXT) test/ack$(DEPEXT) test/korolib$(OBJEXT) test/korolib$(DEPEXT) $(TESTEXE)
endif

-include $(DEPFILES) $(DEBUGDEPFILES) test/ack$(DEPEXT)

# non-target files ????
.PHONY: all clean
