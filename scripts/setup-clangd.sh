#!/bin/sh

echo "Setting up clangd with OpenWatcom headers in $PWD/.tmp."

# Do not run if WATCOM is not set to something
if [ -z "$WATCOM" ]; then
    echo "Must set \$WATCOM variable."
    exit 1
fi

# Copy WATCOM NT headers to .tmp/h/nt
mkdir -p ".tmp/h"
rm -fr ".tmp/h/nt"
cp --dereference -r --no-preserve=mode,ownership "$WATCOM/h/"* ".tmp/h/"
cp --dereference -r --no-preserve=mode,ownership "$WATCOM/h/nt/"* ".tmp/h/"

# Undo 8.3 filename aliases (see h/nt/_w32ials.h)
cp ".tmp/h/winsdkve.h" ".tmp/h/winsdkver.h"
cp ".tmp/h/sdkddkve.h" ".tmp/h/sdkddkver.h"

# Write compile flags
echo "-I$PWD/.tmp/h" > compile_flags.txt
echo "-D__va_list=__builtin_va_list" >> compile_flags.txt
echo "-D_exception_code=__exception_code" >> compile_flags.txt
echo "-D__stdcall=" >> compile_flags.txt
echo "-fms-extensions" >> compile_flags.txt
