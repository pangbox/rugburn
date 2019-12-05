WATCOM := $(HOME)/Programs/watcom
WCC := $(WATCOM)/binl/wcc386
WLINK := $(WATCOM)/binl/wlink
MKDIR := mkdir
CP := cp

CFLAGS := \
	-i$(WATCOM)/h \
	-i$(WATCOM)/h/nt \
	-i$(WATCOM)/h/nt/ddk \
	-zl \
	-s \
	-bd \
	-os \
	-d0 \
	-fr= \
	-zq

LDFLAGS := \
	LIBPATH $(WATCOM)/lib386 \
	LIBPATH $(WATCOM)/lib386/nt

OBJS   := \
	obj/hooks/kernel32/inject.o \
	obj/hooks/user32/window.o \
	obj/hooks/ws2_32/redir.o \
	obj/hooks/wininet/netredir.o \
	obj/hooks/hooks.o \
	obj/third_party/lend/ld32.o \
	obj/common.o \
	obj/ijlfwd.o \
	obj/main.o \
	obj/ntdll.o \
	obj/patch.o

OUT := out/ijl15.dll

all: $(OUT)

.PHONY: clean

obj/%.o: src/%.c
	@$(MKDIR) -p $(dir $@)
	$(WCC) $(CFLAGS) $< -fo=$@

$(OUT): $(OBJS)
	@$(MKDIR) -p $(dir $@)
	$(WLINK) $(LDFLAGS) NAME $@ @export.def FILE {$(OBJS)}

clean:
	$(RM) $(OBJS) $(OUT)

shared: $(OUT)
	$(RM) "${HOME}/Shared/ijl15.dll"
	$(CP) "$(OUT)" "${HOME}/Shared/ijl15.dll"
	$(RM) "${HOME}/Pangya/Clients/PangYa Japan/ijl15.dll"
	$(CP) "$(OUT)" "${HOME}/Pangya/Clients/PangYa Japan/ijl15.dll"
	$(RM) "${HOME}/Pangya/Clients/PangYa US 852.00/ijl15.dll"
	$(CP) "$(OUT)" "${HOME}/Pangya/Clients/PangYa US 852.00/ijl15.dll"
