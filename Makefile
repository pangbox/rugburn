CXX := i686-w64-mingw32-g++
RM := rm
SRCDIR := src/
OBJDIR := obj/
VERSION ?= $(shell git describe --tags --always --dirty)

CFLAGS := -DSTRSAFE_NO_DEPRECATE

LDFLAGS := \
	-nodefaultlibs \
	-nostartfiles \
	third_party/ijl/ijl15l.lib \
	-lws2_32 \
	-lkernel32 \
	-luser32 \
	third_party/msvcrt/msvcrt.lib \
	-Wl,--enable-stdcall-fixup \
	-s

OBJS := \
	obj/dll/rugburn/main.o \
	obj/hooks/kernel32/inject.o \
	obj/hooks/msvcr100/msvcr100.o \
	obj/hooks/projectg/us852/ranking.o \
	obj/hooks/user32/window.o \
	obj/hooks/ws2_32/redir.o \
	obj/hooks/wininet/netredir.o \
	obj/hooks/comctl32/dynamic_patch.o \
	obj/hooks/ole32/web_browser.o \
	obj/hooks/hooks.o \
	obj/ld.o \
	obj/common.o \
	obj/config.o \
	obj/hex.o \
	obj/ijl15.o \
	obj/json.o \
	obj/patch.o \
	obj/regex.o \
	obj/stubs.o

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
	$(CXX) -c $(CFLAGS) "$<" -o "$@"
$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@mkdir -p "$(dir $@)"
	$(CXX) -c $(CFLAGS) "$<" -o "$@"
$(OBJDIR)%.o: $(SRCDIR)%.S
	@mkdir -p "$(dir $@)"
	$(CXX) -c $(CFLAGS) "$<" -o "$@"
$(OUT): $(OBJS)
	@mkdir -p "$(dir $@)"
	$(CXX) $(OBJS) $(LDFLAGS) -shared -o "$@" export.def -Wl,-e_DllMain
$(TESTOUT): $(TESTOBJS)
	@mkdir -p "$(dir $@)"
	$(CXX) $(TESTOBJS) $(LDFLAGS) -o "$@" -Wl,-e_start
check: out/test.exe
	$(EXECWIN) out/test.exe | tappy
clean:
	$(RM) -f $(OBJS) $(TESTOBJS) $(OUT) $(TESTOUT)
