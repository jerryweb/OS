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


all: halt matmult sort testfiles forkTest printfTest lockTest lockTestHelper condTest condTestHelper execTest exitTest airportSim airportTest airportSimSim matmultFork matmultExec serverLockTest airportPass airportLiaison airportCheckin airportCargo airportManager airportPassInit airportLCargoInit airportMCheckinInit
	
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
    
matmultFork.o: matmultFork.c
	$(CC) $(CFLAGS) -c matmultFork.c
matmultFork: matmultFork.o start.o
	$(LD) $(LDFLAGS) start.o matmultFork.o -o matmultFork.coff
	../bin/coff2noff matmultFork.coff matmultFork
    
matmultExec.o: matmultExec.c
	$(CC) $(CFLAGS) -c matmultExec.c
matmultExec: matmultExec.o start.o
	$(LD) $(LDFLAGS) start.o matmultExec.o -o matmultExec.coff
	../bin/coff2noff matmultExec.coff matmultExec

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
    
condTestHelper.o: condTestHelper.c
	$(CC) $(CFLAGS) -c condTestHelper.c
condTestHelper: condTestHelper.o start.o
	$(LD) $(LDFLAGS) start.o condTestHelper.o -o condTestHelper.coff
	../bin/coff2noff condTestHelper.coff condTestHelper

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

airportSim.o: airportSim.c
	$(CC) $(CFLAGS) -c airportSim.c
airportSim: airportSim.o start.o
	$(LD) $(LDFLAGS) start.o airportSim.o -o airportSim.coff
	../bin/coff2noff airportSim.coff airportSim

airportTest.o: airportTest.c
	$(CC) $(CFLAGS) -c airportTest.c
airportTest: airportTest.o start.o
	$(LD) $(LDFLAGS) start.o airportTest.o -o airportTest.coff
	../bin/coff2noff airportTest.coff airportTest

airportSimSim.o: airportSimSim.c
	$(CC) $(CFLAGS) -c airportSimSim.c
airportSimSim: airportSimSim.o start.o
	$(LD) $(LDFLAGS) start.o airportSimSim.o -o airportSimSim.coff
	../bin/coff2noff airportSimSim.coff airportSimSim
	
serverLockTest.o: serverLockTest.c
	$(CC) $(CFLAGS) -c serverLockTest.c
serverLockTest: serverLockTest.o start.o
	$(LD) $(LDFLAGS) start.o serverLockTest.o -o serverLockTest.coff
	../bin/coff2noff serverLockTest.coff serverLockTest

airportPass.o: airportPass.c
	$(CC) $(CFLAGS) -c airportPass.c
airportPass: airportPass.o start.o
	$(LD) $(LDFLAGS) start.o airportPass.o -o airportPass.coff
	../bin/coff2noff airportPass.coff airportPass

airportLiaison.o: airportLiaison.c
	$(CC) $(CFLAGS) -c airportLiaison.c
airportLiaison: airportLiaison.o start.o
	$(LD) $(LDFLAGS) start.o airportLiaison.o -o airportLiaison.coff
	../bin/coff2noff airportLiaison.coff airportLiaison

airportCheckin.o: airportCheckin.c
	$(CC) $(CFLAGS) -c airportCheckin.c
airportCheckin: airportCheckin.o start.o
	$(LD) $(LDFLAGS) start.o airportCheckin.o -o airportCheckin.coff
	../bin/coff2noff airportCheckin.coff airportCheckin

airportCargo.o: airportCargo.c
	$(CC) $(CFLAGS) -c airportCargo.c
airportCargo: airportCargo.o start.o
	$(LD) $(LDFLAGS) start.o airportCargo.o -o airportCargo.coff
	../bin/coff2noff airportCargo.coff airportCargo

airportManager.o: airportManager.c
	$(CC) $(CFLAGS) -c airportManager.c
airportManager: airportManager.o start.o
	$(LD) $(LDFLAGS) start.o airportManager.o -o airportManager.coff
	../bin/coff2noff airportManager.coff airportManager

airportPassInit.o: airportPassInit.c
	$(CC) $(CFLAGS) -c airportPassInit.c
airportPassInit: airportPassInit.o start.o
	$(LD) $(LDFLAGS) start.o airportPassInit.o -o airportPassInit.coff
	../bin/coff2noff airportPassInit.coff airportPassInit

airportLCargoInit.o: airportLCargoInit.c
	$(CC) $(CFLAGS) -c airportLCargoInit.c
airportLCargoInit: airportLCargoInit.o start.o
	$(LD) $(LDFLAGS) start.o airportLCargoInit.o -o airportLCargoInit.coff
	../bin/coff2noff airportLCargoInit.coff airportLCargoInit

airportMCheckinInit.o: airportMCheckinInit.c
	$(CC) $(CFLAGS) -c airportMCheckinInit.c
airportMCheckinInit: airportMCheckinInit.o start.o
	$(LD) $(LDFLAGS) start.o airportMCheckinInit.o -o airportMCheckinInit.coff
	../bin/coff2noff airportMCheckinInit.coff airportMCheckinInit

clean:
	rm -f *.o *.coff
	rm halt sort matmult testfiles forkTest printfTest lockTest lockTestHelper condTest condTestHelper execTest exitTest airportSim airportTest matmultFork matmultExec serverLockTest airportPass airportLiaison airportCheckin airportCargo airportManager airportPassInit airportLCargoInit airportMCheckinInit
	rm *~
