// main.cc 
//	Bootstrap code to initialize the operating system kernel.
//
//	Allows direct calls into internal operating system functions,
//	to simplify debugging and testing.  In practice, the
//	bootstrap code would just initialize data structures,
//	and start a user program to print the login prompt.
//
// 	Most of this file is not needed until later assignments.
//
// Usage: nachos -d <debugflags> -rs <random seed #>
//		-s -x <nachos file> -c <consoleIn> <consoleOut>
//		-f -cp <unix file> <nachos file>
//		-p <nachos file> -r <nachos file> -l -D -t
//              -n <network reliability> -m <machine id>
//              -o <other machine id>
//              -z
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -rs causes Yield to occur at random (but repeatable) spots
//    -z prints the copyright message
//
//  USER_PROGRAM
//    -s causes user programs to be executed in single-step mode
//    -x runs a user program
//    -c tests the console
//
//  FILESYS
//    -f causes the physical disk to be formatted
//    -cp copies a file from UNIX to Nachos
//    -p prints a Nachos file to stdout
//    -r removes a Nachos file from the file system
//    -l lists the contents of the Nachos directory
//    -D prints the contents of the entire file system 
//    -t tests the performance of the Nachos file system
//
//  NETWORK
//    -n sets the network reliability
//    -m sets this machine's host id (needed for the network)
//    -o runs a simple test of the Nachos network software
//
//  NOTE -- flags are ignored until the relevant assignment.
//  Some of the flags are interpreted here; some in system.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"
#include <sstream>

using namespace std;
// External functions used by this file

extern void ThreadTest(void), Copy(char *unixFile, char *nachosFile);
extern void Print(char *file), PerformanceTest(void);
extern void StartProcess(char *file), ConsoleTest(char *in, char *out);
extern void MailTest(int networkID);
extern void TestSuite();
extern void ManagerTest();
extern void LiaisonTest();
extern void PassengerFindsShortestLiaisonLine();
extern void PassengerFindsCorrectCISLine();
extern void CheckInTest();
extern void CargoTest();
extern void ScreenTest();
extern void InspectTest();
extern void MTest();
extern void AirportSim();
extern void MainMenu();
int currentBoxCountPointer;
Lock* boxCountIndexLock;

//void RunServer();
//----------------------------------------------------------------------
// main
// 	Bootstrap the operating system kernel.  
//	
//	Check command line arguments
//	Initialize data structures
//	(optionally) Call test procedure
//
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int main(int argc, char **argv) {
	int argCount;			// the number of arguments
	// for a particular command

	DEBUG('t', "Entering main");
	(void) Initialize(argc, argv);
	//arCount = 1;
	for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
		argCount = 1;
// #ifdef THREADS
		if (!strcmp(*argv, "-T")) {	// Lock and Condition Test
			TestSuite();
		}

		if (!strcmp(*argv, "-manager")) {	// Manager and full simulation test
			ManagerTest();
		}

		if (!strcmp(*argv, "-mtest")) {	// Manager and full simulation test
			MTest();
		}

		if (!strcmp(*argv, "-findCIS")) {// Tests if passengers go the the correct CIS line.
			PassengerFindsCorrectCISLine();
		}

		if (!strcmp(*argv, "-findL")) {	//Tests to see if the passenger goes to the shortest line
			PassengerFindsShortestLiaisonLine();
		}

		if (!strcmp(*argv, "-liaison")) { // Tests if liaison directs customer to correct check-in area.
			LiaisonTest();
		}

		if (!strcmp(*argv, "-checkin")) { // Tests if check-in prioritizes executive line.
			CheckInTest();
		}

		if (!strcmp(*argv, "-cargo")) {   // Tests if cargo handler functions.
			CargoTest();
		}

		if (!strcmp(*argv, "-screen")) {
			ScreenTest();
		}

		if (!strcmp(*argv, "-inspect")) {
			InspectTest();
		}

		if (!strcmp(*argv, "-airport")) {   // Runs the full airport simulation.
			AirportSim();
		}

		if (!strcmp(*argv, "-menu")) {   // Runs the full airport simulation.
			MainMenu();
		}

// #endif

		if (!strcmp(*argv, "-z"))               // print copyright
			printf(copyright);
#ifdef USER_PROGRAM
		if (!strcmp(*argv, "-x")) {        	// run a user program
			ASSERT(argc > 1);
			DEBUG('t', "filename: %s\n", *(argv + 1));
			StartProcess(*(argv + 1));
			argCount = 2;
		} else if (!strcmp(*argv, "-c")) {      // test the console
			if (argc == 1)
			ConsoleTest(NULL, NULL);
			else {
				ASSERT(argc > 2);
				ConsoleTest(*(argv + 1), *(argv + 2));
				argCount = 3;
			}
			interrupt->Halt();		// once we start the console, then
			// Nachos will loop forever waiting
			// for console input
		}
#endif // USER_PROGRAM
#ifdef FILESYS
		if (!strcmp(*argv, "-cp")) { 		// copy from UNIX to Nachos
			ASSERT(argc > 2);
			Copy(*(argv + 1), *(argv + 2));
			argCount = 3;
		} else if (!strcmp(*argv, "-p")) {	// print a Nachos file
			ASSERT(argc > 1);
			Print(*(argv + 1));
			argCount = 2;
		} else if (!strcmp(*argv, "-r")) {	// remove Nachos file
			ASSERT(argc > 1);
			fileSystem->Remove(*(argv + 1));
			argCount = 2;
		} else if (!strcmp(*argv, "-l")) {	// list Nachos directory
			fileSystem->List();
		} else if (!strcmp(*argv, "-D")) {	// print entire filesystem
			fileSystem->Print();
		} else if (!strcmp(*argv, "-t")) {	// performance test
			PerformanceTest();
		}
#endif // FILESYS
#ifdef NETWORK
        if (!strcmp(*argv, "-o")) {
	    ASSERT(argc > 1);
            Delay(2); 				// delay for 2 seconds
						// to give the user time to 
						// start up another nachos
            MailTest(atoi(*(argv + 1)));
            argCount = 2;
        }

        if (!strcmp(*argv,"-server")) {
        //	RunServer();
        }

#endif // NETWORK
	}

	currentThread->Finish();	// NOTE: if the procedure "main"
	// returns, then the program "nachos"
	// will exit (as any other normal program
	// would).  But there may be other
	// threads on the ready list.  We switch
	// to those threads by saying that the
	// "main" thread is finished, preventing
	// it from returning.
	return (0);			// Not reached...
}

#ifdef NETWORK

//put the client code in test because it's a user prog
//server code goes in main
// request type:
// 1  -> create lock
// 2  -> destory lock
// 3  -> lock acquire
// 4  -> lock release
// 5  -> create CV
// 6  -> destory CV
// 7  -> CV signal
// 8  -> CV wait

void RunServer() {
	//TODO:build lock and CV table here
	Table* serverLockTable = new Table(2048);
	Table* serverCVTable = new Table(2048);

	while (true)
    {
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char buffer[MaxMailSize];
		stringstream ss;
		ss.str("");
		ss.clear();
		int request = -1;
		string arg1;

		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		ss << buffer;
		ss >> request;
		ss >> arg1;

        switch (request)
        {
            case 1:
                createLock(arg1, serverLockTable, inPktHdr.from);
                break;
            case 2:
                destroyLock(arg1, serverLockTable, inPktHdr.from);
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                break;
            default:
                printf("invalid request type\n");
        }
	}
}

void createLock(string lName, Table* sTable, int out) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *msg;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailHdr.from = out;

	if (!tableItemExist(lName, sTable, 1)) {
		serverLock* toPut = new serverLock(lName, -1, -1);
		sTable->Put(toPut);
		msg = "0";
	} else {
		msg = "1";
	}

}

void destoryLock(string lName, Table* sTable, int out) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *msg;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailHdr.from = out;

	if (!tableItemExist(lName, sTable, 1)) {
		msg = "1";
	} else {
		serverLock* tItem = (serverLock*) (sTable->Remove(toRemove));
		delete tItem;
		msg = "0";
	}

}

bool acquireLock(string lName, Table* sTable) {
	if (!tableItemExist(lName, sTable, 1)) {
	}
}

#endif // NETWORK
