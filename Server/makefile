CCX64	:= x86_64-w64-mingw32-gcc
CCX86	:= i686-w64-mingw32-gcc

OUTX64	:= server.o

all: x64

x64:
	@ echo Compiling the BOF...
	@ $(CCX64) -c server.c -o $(OUTX64) -lws2_32
