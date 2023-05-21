GO := go

WATCOM ?= /usr/bin/watcom
WCC := $(WATCOM)/binl64/wcc386
WLINK := $(WATCOM)/binl64/wlink
RM := rm
SRCDIR := src/
OBJDIR := obj/
WEBASSETDIR := web/asset/
WEBDISTDIR := web/dist/

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

OBJS := \
	obj/dll/rugburn/main.o \
	obj/hooks/kernel32/inject.o \
	obj/hooks/msvcr100/msvcr100.o \
	obj/hooks/user32/window.o \
	obj/hooks/ws2_32/redir.o \
	obj/hooks/wininet/netredir.o \
	obj/hooks/hooks.o \
	obj/third_party/lend/ld32.o \
	obj/bootstrap.o \
	obj/common.o \
	obj/config.o \
	obj/ijlfwd.o \
	obj/json.o \
	obj/patch.o \
	obj/regex.o

TESTOBJS := \
	$(OBJS) \
	obj/exe/test/main.o

WEBASSET := \
	web/dist/index.html \
	web/dist/style.css \
	web/dist/main.js \
	web/dist/wasm_exec.js


OUT := out/rugburn.dll
OUTSS := out/ijl15.dll
TESTOUT := out/test.exe
WEBOUT := web/dist/patcher.wasm
FLYPROJECT := rugburn-gg

all: $(OUT) $(TESTOUT) $(WEBOUT)
slipstream: $(OUTSS)

.PHONY: clean slipstream

# Rugburn
$(OBJDIR)%.o: $(SRCDIR)%.c
	@mkdir -p "$(dir $@)"
	$(WCC) $(CFLAGS) "$<" "-fo=$@"
$(OUT): $(OBJS)
	@mkdir -p "$(dir $@)"
	$(WLINK) $(LDFLAGS) NAME "$@" @export.def FILE {$(OBJS)}
$(TESTOUT): $(TESTOBJS)
	@mkdir -p "$(dir $@)"
	$(WLINK) $(LDFLAGS) NAME "$@" @test.def FILE {${TESTOBJS}}

# Slipstream
ijl15.dll:
	@echo "Error: To use slipstream, place an original ijl15.dll in the source root."
	@exit 1
$(OUTSS): $(OUT) ijl15.dll
	$(GO) run ./slipstrm/cmd/slipstrm ijl15.dll $(OUT) $(OUTSS)

# Website/web patcher
$(WEBDISTDIR)%: $(WEBASSETDIR)%
	cp "$<" "$@"
$(WEBOUT): $(OUT) $(WEBASSET) web/patcher/patcher.go
	GOOS=js GOARCH=wasm $(GO) build -o "$@" "./web/patcher"
watch:
	while rm -f $(WEBOUT) && make $(WEBOUT) && go run ./web/testsrv.go -watch ./; do :; done
deploy:
	nix run nixpkgs#skopeo -- --insecure-policy --debug copy docker-archive:"$(shell nix build .#dockerImage --print-out-paths)" docker://registry.fly.io/$(FLYPROJECT):latest --dest-creds x:"$(shell flyctl auth token)" --format v2s2
	nix run nixpkgs#flyctl -- deploy -i registry.fly.io/$(FLYPROJECT):latest --remote-only

clean:
	$(RM) -f $(OBJS) $(OUT) $(OUTSS) $(TESTOUT)
