#!/bin/bash

echo "Compiling sources..."

echo "Compiling my app..."
gcc -m32 my.c -o my32
gcc my.c -o my64
echo "Compiling my app done"

echo "Compiling derby app..."
#gcc -m32 my.c -o my32
echo "Compiling derby app done"

if [ ! -f derby32 ]; then
	echo "derby32 not found"
else
	echo "derby32 found"
fi

if [ ! -f derby64 ]; then
	echo "derby64 not found"
else
	echo "derby64 found"
fi

if [ ! -f edb ]; then
	echo "edb not found"
else
	echo "edb found"
fi

if [ ! -f dDeflect ]; then
	echo "dDeflect not found"
else
	echo "dDeflect found"
fi

echo "Compiling sources done"

echo "Compiling assembly methods..."

echo "compiling ptrace..."
nasm ptrace.asm
echo "compiling ptrace done"

echo "compiling ptrace64..."
nasm ptrace64.asm
echo "compiling ptrace64 done"

echo "compiling thread..."
nasm thread.asm
echo "compiling thread done"

echo "compiling thread64..."
nasm thread64.asm
echo "compiling thread64 done"

echo "Compiling assembly methods done"

echo "Starting app"

./tester
