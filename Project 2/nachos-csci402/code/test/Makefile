# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ./
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR = 
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)


all: halt matmult sort testfiles forkTest printfTest lockTest lockTestHelper condTest execTest
	
start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

forkTest.o: forkTest.c
	$(CC) $(CFLAGS) -c forkTest.c
forkTest: forkTest.o start.o
	$(LD) $(LDFLAGS) start.o forkTest.o -o forkTest.coff
	../bin/coff2noff forkTest.coff forkTest
    
printfTest.o: printfTest.c
	$(CC) $(CFLAGS) -c printfTest.c
printfTest: printfTest.o start.o
	$(LD) $(LDFLAGS) start.o printfTest.o -o printfTest.coff
	../bin/coff2noff printfTest.coff printfTest
    
lockTest.o: lockTest.c
	$(CC) $(CFLAGS) -c lockTest.c
lockTest: lockTest.o start.o
	$(LD) $(LDFLAGS) start.o lockTest.o -o lockTest.coff
	../bin/coff2noff lockTest.coff lockTest
    
lockTestHelper.o: lockTestHelper.c
	$(CC) $(CFLAGS) -c lockTestHelper.c
lockTestHelper: lockTestHelper.o start.o
	$(LD) $(LDFLAGS) start.o lockTestHelper.o -o lockTestHelper.coff
	../bin/coff2noff lockTestHelper.coff lockTestHelper
    
condTest.o: condTest.c
	$(CC) $(CFLAGS) -c condTest.c
condTest: condTest.o start.o
	$(LD) $(LDFLAGS) start.o condTest.o -o condTest.coff
	../bin/coff2noff condTest.coff condTest

execTest.o: execTest.c
	$(CC) $(CFLAGS) -c execTest.c
execTest: execTest.o start.o
	$(LD) $(LDFLAGS) start.o execTest.o -o execTest.coff
	../bin/coff2noff execTest.coff execTest

exitTest.o: exitTest.c
	$(CC) $(CFLAGS) -c exitTest.c
exitTest: exitTest.o start.o
	$(LD) $(LDFLAGS) start.o exitTest.o -o exitTest.coff
	../bin/coff2noff exitTest.coff exitTest

clean:
	rm -f *.o *.coff
	rm halt sort matmult testfiles forkTest printfTest lockTest lockTestHelper condTest execTest
	rm *~
