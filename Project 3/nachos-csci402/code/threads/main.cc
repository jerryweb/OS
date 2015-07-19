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
#ifdef NETWORK
#include "serversynch.h"
#include <sstream>
#include <string>
#endif

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

#ifdef NETWORK
void RunServer();
void createLock(char* lName, Table* sTable, int outAddr,int outBox);
void destroyLock(char* lName, Table* sTable, int outAddr,int outBox);
void createCV(char* cName,Table* cTable,int outAddr,int outBox);
void destroyCV(char* cName,Table* cTable,int outAddr,int outBox);
#endif
//bool tableItemExist(char* tName, Table* table, int tableType);
//int getTableIndex(char* tName, Table* table, int tableType);
//void ServerReply(char* sMsg, int outMachine, int outMailbox, int fromMailbox);

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
	// This keeps track of the number of threads that call the create lock function.
	// Whenever a lock is created, it will increment by 1. Whenevr the destroy lock 
	// function is called, it will decrement by 1. 

#ifdef NETWORK
	createLockRequests = 0;
	createCVRequests = 0;
#endif

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
		//Table* serverLockTable;
		serverLockTable = new Table(2048);
		//Table* serverCVTable;
		serverCVTable = new Table(2048);

		if (!strcmp(*argv, "-o")) {
			ASSERT(argc > 1);
			Delay(2); 				// delay for 2 seconds
			// to give the user time to
			// start up another nachos
			MailTest(atoi(*(argv + 1)));
			argCount = 2;
		}

		if (!strcmp(*argv, "-lock")) {
			ASSERT(argc > 1);
			Delay(2);
		}

		if (!strcmp(*argv,"-server")) {
			RunServer();
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
//need to add Broadcast
void RunServer() {
	//TODO:build lock and CV table here
	// Table* serverLockTable = new Table(2048);
	//Table* serverCVTable = new Table(2048);

	while (true)
	{
		//printf("in server while loop\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char buffer[MaxMailSize];
		stringstream ss;
		ss.str("");
		ss.clear();
		int request = -1;
		int index = -1;
		string arg1,arg2;

		//printf("waiting on in mail\n");
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		ss << buffer;
		ss >> request;
		printf("request type is %d\n",request);
		ss >> arg1;
		char* cArg1 = (char*) arg1.c_str();

		//declare these used for switch block
		serverLock* sLock;
		serverCV* sCV;
		char* cArg2;
		int index2;

		printf("before switch request\n");
		switch (request)
		{
			case 1:   //create lock
			createLock(cArg1, serverLockTable, inPktHdr.from,0);
			break;

			case 2://destory lock
			destroyLock(cArg1, serverLockTable, inPktHdr.from,0);
			break;

			case 3://acquire lock
			index = getTableIndex(cArg1,serverLockTable,1);
			sLock = (serverLock*)serverLockTable->Get(index);
			sLock->Acquire(inPktHdr.from,0);
			break;

			case 4://release lock
			index = getTableIndex(cArg1,serverLockTable,1);
			sLock = (serverLock*)serverLockTable->Get(index);
			sLock->Release(inPktHdr.from,0);
			break;

			case 5://create CV
			createCV(cArg1,serverCVTable,inPktHdr.from,0);
			break;

			case 6://destroy CV
			destroyCV(cArg1,serverCVTable,inPktHdr.from,0);
			break;

			case 7://CV Signal
			ss>>arg2;
			cArg2 = (char*) arg2.c_str();
			index = getTableIndex(cArg1,serverCVTable,2);
			index2 = getTableIndex(cArg2,serverLockTable,1);
			sCV = (serverCV*)serverCVTable->Get(index);
			sLock = (serverLock*)serverLockTable->Get(index2);
			sCV->Signal(sLock,inPktHdr.from,0);
			break;

			case 8://CV Wait
			ss>>arg2;
			cArg2 = (char*) arg2.c_str();
			index = getTableIndex(cArg1,serverCVTable,2);
			index2 = getTableIndex(cArg2,serverLockTable,1);
			sCV = (serverCV*)serverCVTable->Get(index);
			sLock = (serverLock*)serverLockTable->Get(index2);
			sCV->Wait(sLock,inPktHdr.from,0);
			break;

			case 9://CV Broadcast
			ss>>arg2;
			cArg2 = (char*) arg2.c_str();
			index = getTableIndex(cArg1,serverCVTable,2);
			index2 = getTableIndex(cArg2,serverLockTable,1);
			sCV = (serverCV*)serverCVTable->Get(index);
			sLock = (serverLock*)serverLockTable->Get(index2);
			sCV->Boardcast(sLock,inPktHdr.from,0);
			break;

			default:
			printf("invalid request type\n");
			char* eMsg = new char[MaxMailSize];
			eMsg = "1";
			ServerReply(eMsg,1,0,0);
		}
	}
}

//perform creatLock syscall on server,update created lock count
void createLock(char* lName, Table* sTable, int outAddr,int outBox) {
	char* msg = new char[MaxMailSize];
	int location = 0;

	if (!tableItemExist(lName, sTable, 1)) {
		serverLock* toPut = new serverLock(lName, -1, -1);
		location = sTable->Put(toPut);
		createLockRequests++;
		msg = "0";
	}	//This returns the location if the lock already exists of that lock 
	else {
		location = getTableIndex(lName,sTable,1);
		msg = "1";
	}

	//TODO:return location?
	ServerReply(msg,outAddr,outBox,0);
}

//perform destoryLock syscall on server
//the actual destroy actions won't be perfromed until it's the last lock
void destroyLock(char* lName, Table* sTable, int outAddr,int outBox) {
	char* msg = new char[MaxMailSize];

	if(createLockRequests == 0) {
		if (!tableItemExist(lName, sTable, 1)) {
			msg = "1";
		}
		else {//Delete all the locks from the. This should only run at the end of the program
			int toRemove = sTable->Size();
			toRemove--;
			while(sTable->getCount() != 0) {
				serverLock* tItem = (serverLock*) (sTable->Remove(toRemove));
				toRemove--;
				delete tItem;
				msg = "0";
			}
		}
	}
	else {
		createLockRequests--;
	}

	ServerReply(msg,outAddr,outBox,0);

}

//perform createCV syscall on server
void createCV(char* cName,Table* cTable,int outAddr,int outBox) {
	char* msg = new char[MaxMailSize];
	int location = -1;

	if (!tableItemExist(cName,cTable,2)) {
		serverCV* toPut = new serverCV(cName);
		location = cTable->Put(toPut);
		createCVRequests++;   //TODO: where to put this?
		msg = "0";
	} else {
		location = getTableIndex(cName,cTable,2);
		msg = "1";
	}

	//TODO: return location?
	ServerReply(msg,outAddr,outBox,0);
}

//perform destroyCV syscall on server
void destroyCV(char* cName,Table* cTable,int outAddr,int outBox) {
	char* msg = new char[MaxMailSize];

	if(createCVRequests == 0) {
		if (!tableItemExist(cName, cTable, 2)) {
			msg = "1";
		}
		else {//Delete all the CVs from the. This should only run at the end of the program
			int toRemove = cTable->Size();
			toRemove--;
			while(cTable->getCount() != 0) {
				serverCV* cItem = (serverCV*) (cTable->Remove(toRemove));
				toRemove--;
				delete cItem;
				msg = "0";
			}
		}
	}
	else {
		createCVRequests--;
	}

	ServerReply(msg,outAddr,outBox,0);
}

#endif // NETWORK
