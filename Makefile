CC := i686-w64-mingw32-gcc
RM := rm
SRCDIR := src/
OBJDIR := obj/
VERSION ?= $(shell git describe --tags --always --dirty)

CFLAGS := -DSTRSAFE_NO_DEPRECATE

LDFLAGS := -nodefaultlibs -nostartfiles -lws2_32 -lkernel32 -luser32 -Wl,--enable-stdcall-fixup -s

LDFLAGS_MSVCRT := third_party/msvcrt/msvcrt.lib third_party/msvcrt/rtsyms.o

OBJS := \
	obj/dll/rugburn/main.o \
	obj/hooks/kernel32/inject.o \
	obj/hooks/msvcr100/msvcr100.o \
	obj/hooks/projectg/us852/ranking.o \
	obj/hooks/user32/window.o \
	obj/hooks/ws2_32/redir.o \
	obj/hooks/wininet/netredir.o \
	obj/hooks/comctl32/dynamic_patch.o \
	obj/hooks/hooks.o \
	obj/ld.o \
	obj/common.o \
	obj/config.o \
	obj/hex.o \
	obj/json.o \
	obj/patch.o \
	obj/regex.o

IJL15OBJS := $(wildcard third_party/ijl/*.obj)

TESTOBJS := \
	$(OBJS) \
	obj/exe/test/main.o

OUT := out/ijl15.dll
TESTOUT := out/test.exe

ifneq ($(OS),Windows_NT)
	EXECWIN := wine
endif

all: $(OUT) $(TESTOUT)

.PHONY: check clean

# Rugburn
$(OBJDIR)%.o: $(SRCDIR)%.c
	@mkdir -p "$(dir $@)"
	$(CC) -c $(CFLAGS) "$<" -o "$@"
$(OUT): $(OBJS)
	@mkdir -p "$(dir $@)"
	$(CC) $(OBJS) $(IJL15OBJS) $(LDFLAGS_MSVCRT) $(LDFLAGS) -shared -o "$@" export.def -Wl,-e_DllMain
$(TESTOUT): $(TESTOBJS)
	@mkdir -p "$(dir $@)"
	$(CC) $(TESTOBJS) $(LDFLAGS) -o "$@" -Wl,-e_start
check: out/test.exe
	$(EXECWIN) out/test.exe | tappy
clean:
	$(RM) -f $(OBJS) $(TESTOBJS) $(OUT) $(TESTOUT)
