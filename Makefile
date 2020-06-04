ifeq ($(OS),Windows_NT)
	WATCOM := C:/WATCOM
	WCC := $(WATCOM)/binnt64/wcc386
	WLINK := $(WATCOM)/binnt64/wlink
	RM := del
	SRCDIR := src\\
	OBJDIR := obj\\
	PATHFIX = $(subst /,\,$1)
else
	WATCOM := /opt/watcom
	WCC := $(WATCOM)/binl64/wcc386
	WLINK := $(WATCOM)/binl64/wlink
	RM := rm
	SRCDIR := src/
	OBJDIR := obj/
	PATHFIX = $1
endif

CFLAGS := \
	-i$(call PATHFIX,$(WATCOM)/h) \
	-i$(call PATHFIX,$(WATCOM)/h/nt) \
	-i$(call PATHFIX,$(WATCOM)/h/nt/ddk) \
	-zl \
	-s \
	-bd \
	-os \
	-d0 \
	-fr= \
	-zq

LDFLAGS := \
	LIBPATH $(call PATHFIX,$(WATCOM)/lib386) \
	LIBPATH $(call PATHFIX,$(WATCOM)/lib386/nt)

OBJS := \
	$(call PATHFIX,obj/hooks/kernel32/inject.o) \
	$(call PATHFIX,obj/hooks/user32/window.o) \
	$(call PATHFIX,obj/hooks/ws2_32/redir.o) \
	$(call PATHFIX,obj/hooks/wininet/netredir.o) \
	$(call PATHFIX,obj/hooks/hooks.o) \
	$(call PATHFIX,obj/third_party/lend/ld32.o) \
	$(call PATHFIX,obj/common.o) \
	$(call PATHFIX,obj/ijlfwd.o) \
	$(call PATHFIX,obj/main.o) \
	$(call PATHFIX,obj/ntdll.o) \
	$(call PATHFIX,obj/patch.o)

OUT := $(call PATHFIX,out/ijl15.dll)

all: $(OUT)

.PHONY: clean

ifeq ($(OS),Windows_NT)
$(OBJDIR)%.o: $(SRCDIR)%.c
	@setlocal enableextensions
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@endlocal
	$(WCC) $(CFLAGS) "$<" "-fo=$@"
$(OUT): $(OBJS)
	@setlocal enableextensions
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@endlocal
	$(WLINK) $(LDFLAGS) NAME "$@" @export.def FILE {$(OBJS)}
else
$(OBJDIR)%.o: $(SRCDIR)%.c
	@mkdir -p "$(dir $@)"
	$(WCC) $(CFLAGS) "$<" "-fo=$@"
$(OUT): $(OBJS)
	@mkdir -p "$(dir $@)"
	$(WLINK) $(LDFLAGS) NAME "$@" @export.def FILE {$(OBJS)}
endif

clean:
	$(RM) $(OBJS) $(OUT)
