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
void createLock(char* lName, Table* sTable, int outAddr,int outBox,int fromBox);
void destroyLock(char* lName, Table* sTable, int outAddr,int outBox,int fromBox);
void createCV(char* cName,Table* cTable,int outAddr,int outBox,int fromBox);
void destroyCV(char* cName,Table* cTable,int outAddr,int outBox,int fromBox);
void createMV(char* lname,int len, Table* mTable, int outAddr, int outBox,int fromBox);
void destroyMV(char* mName, Table* mTable, int outAddr,int outBox,int fromBox);
#endif

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
	createMVRequests = 0;
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
		MVTable = new Table(2048);
		LTRArray = new unsigned int[5];
		for (int i=0;i<5;i++) {
			LTRArray[i] = 0;
		}
		pendingMsg = new List();

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
// 9  -> CV Broadcast
// 10 -> create Monitor Variable
// 11 -> destory Monitor Variable
// 12 -> get MV
// 13 -> set MV
// 14 -> update
//need to add Broadcast
void RunServer() {
	//TODO::change syscall.h for MV calls since they need less arguments
	//TODO::get machine ID

	int myId = netname;
	printf("my server id is %d/n",myId);

	while (true)
	{
		/************data declarition****************************/
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char buffer[MaxMailSize];
		stringstream ss;	//used to parse input
		stringstream css;//used to construct message
		ss.str("");
		ss.clear();
		css.str("");
		css.clear();
		int scIdentifier;//0 for client msg, 1 for server msg
		int request = -1;//request type
		int index = -1;
		int original = myId;//to indentify the orignal server,default to myself
		unsigned int tStamp;//timestamp from the buffer
		unsigned int sTStamp = 0;//smallest time stamp in LTR array
		string arg1,arg2,arg3;
		serverLock* sLock;
		serverCV* sCV;
		serverMV* sMV;
		char* cArg1;
		char* cArg2;
		char* cArg3;
		char* eMsg;
		int index2, index3,mValue,mvPos,dummy;//mvPos used as size for create, array index for other functions

		/************Server Forwarding*****************************/
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer); //TODO:change 0 to inbox
		ss << buffer;
		ss >> scIdentifier;

		//if it's a client message forward it to other servers
		if (request == 0) {
			arg1 = ss.str();
			css << "1 " << myId << " "<<arg1 << " ";
			arg1 = css.str();
			//else forward timestamp(only) to other servers
		} else {
			unsigned int fwdTime = getTimeStamp();
			css << "1 "<< myId << " " <<fwdTime << " 14 "; //request type 14 means it's only timestamp update
			arg1 = css.str();
		}
		//forward the above just constructed message/timestamp to other servers
		for (int i = 0;i<5;i++) {
			if (i != myId) {
				cArg1 = new char[MaxMailSize];
				strcpy(cArg1,(char*)arg1.c_str());
				ServerReply(cArg1,i,0,0); //it'fine to use 0,0 for to, from mailbox
										  //since server is single thread
			}
		}
		ss.str("");
		ss.clear();
		css.str("");
		css.clear();
		/************server forwarding ends**************************/

		/************total ordering**********************************/
		ss << buffer;
		ss >> dummy;
		// read in orgianl server if it's server fwd msg
		if (scIdentifier == 0) {
			ss >> original;
		}
		ss >> tStamp;
		//update LTRArray (last time received)
		if (LTRArray[original] < tStamp) {
			LTRArray[original] = tStamp;
		}
		//update smallest time stamp from LTR
		for (int i=0;i<5;i++) {
			if (sTStamp == 0) {
				sTStamp = LTRArray[i];
				continue;
			}
			if (sTStamp > LTRArray[i]) {
				sTStamp = LTRArray[i];
			}
		}
		//append the msg without scIdentifier to the pending message list(sorted)
		ss.str("");
		ss.clear();
		ss >> dummy;
		arg1 = ss.str();
		char* nMsg;
		nMsg = new char[MaxMailSize];
		strcpy(nMsg,(char*)arg1.c_str());
		pendingMsg->SortedInsert((void*)nMsg,tStamp);
		ss.str("");
		ss.clear();
		/*****************total ordering ends******************/

		/***************pending msg processing**************/
		while (true) {
			ss >> request;
			printf("request type is %d\n",request);
			switch (request)
			{
				case 1:   //create lock
				ss >> arg1;
				cArg1 = (char*) arg1.c_str();
				createLock(cArg1, serverLockTable, inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 2://destory lock
				ss >> arg1;
				cArg1 = (char*) arg1.c_str();
				destroyLock(cArg1, serverLockTable, inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 3://acquire lock
				ss >> index;
				sLock = (serverLock*)serverLockTable->Get(index);

				//check if the lock is valid
				if (sLock == NULL) {
					eMsg = "1";
					ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
					break;
				}
				sLock->Acquire(inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 4:   //release lock
				ss >> index;
				//index = getTableIndex(cArg1,serverLockTable,1);
				sLock = (serverLock*)serverLockTable->Get(index);
				//check if the lock is valid
				if (sLock == NULL) {
					eMsg = "1";
					ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
					break;
				}
				sLock->Release(inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 5:   //create CV
				ss >> arg1;
				cArg1 = (char*) arg1.c_str();
				createCV(cArg1,serverCVTable,inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 6://destroy CV
				ss >> arg1;
				cArg1 = (char*) arg1.c_str();
				destroyCV(cArg1,serverCVTable,inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 7://CV Signal
				ss >> index;
				ss>>index2;
				sCV = (serverCV*)serverCVTable->Get(index);
				sLock = (serverLock*)serverLockTable->Get(index2);
				if (sCV == NULL || sLock == NULL) {
					eMsg = "1";
					ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
					break;
				}
				sCV->Signal(sLock,inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 8:   //CV Wait
				ss >> index;
				ss>>index2;
				sCV = (serverCV*)serverCVTable->Get(index);
				sLock = (serverLock*)serverLockTable->Get(index2);
				if (sCV == NULL || sLock == NULL) {
					eMsg = "1";
					ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
					break;
				}
				sCV->Wait(sLock,inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 9:   //CV Broadcast
				ss >> index;
				ss>>index2;
				sCV = (serverCV*)serverCVTable->Get(index);
				sLock = (serverLock*)serverLockTable->Get(index2);
				if (sCV == NULL || sLock == NULL) {
					eMsg = "1";
					ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
					break;
				}
				sCV->Boardcast(sLock,inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 10:   //create mv
				ss>> arg1;
				ss>> mvPos;
				cArg1 = (char*) arg1.c_str();
				createMV(cArg1,mvPos, MVTable, inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 11://destroy mv
				ss >> arg1;
				cArg1 = (char*)arg1.c_str();
				destroyMV(cArg1, MVTable, inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 12://read monitor variable
				ss>> index;
				sMV = (serverMV*)MVTable->Get(index);
				if (sMV == NULL) {
					eMsg = new char[MaxMailSize];
					eMsg = "1";
					ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
					break;
				}
				ss >> mvPos;
				sMV->Read(mvPos,inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 13: //set the monitor variable
				ss>> index >> mValue;
				sMV = (serverMV*)MVTable->Get(index);
				if (sMV == NULL) {
					eMsg = new char[MaxMailSize];
					eMsg = "1";
					ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
					break;
				}
				ss >> mvPos;
				sMV->Set(mValue,mvPos,inPktHdr.from,inMailHdr.from,inMailHdr.to);
				break;

				case 14: //do nothing, since it's only a timestamp udpate from method 1
				break;

				default:
				printf("invalid request type\n");
				eMsg = new char[MaxMailSize];
				eMsg = "1";
				ServerReply(eMsg,inPktHdr.from,inMailHdr.from,inMailHdr.to);
			}
		}
		/*****************pending msg processing ends***********************/
	}
}

//perform creatLock syscall on server,update created lock count
void createLock(char* lName, Table* sTable, int outAddr,int outBox,int fromBox) {
	char* msg = new char[MaxMailSize];
	int location = 0;

	if (!tableItemExist(lName, sTable, 1)) {
		serverLock* toPut = new serverLock(lName, -1, -1);
		location = sTable->Put(toPut);
		createLockRequests++;
		string toSend;
		stringstream sss;
		sss <<"0 "<<location;
		toSend = sss.str();
		msg = (char*) toSend.c_str();
	}	//This returns the location if the lock already exists of that lock 
	else {
		location = getTableIndex(lName,sTable,1);
		string toSend;
		stringstream sss;
		sss <<"0 "<<location;
		toSend = sss.str();
		msg = (char*) toSend.c_str();
		//msg = "1";
	}

	ServerReply(msg,outAddr,outBox,fromBox);
}

//perform destoryLock syscall on server
//the actual destroy actions won't be perfromed until it's the last lock
void destroyLock(char* lName, Table* sTable, int outAddr,int outBox,int fromBox) {
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

	ServerReply(msg,outAddr,outBox,fromBox);

}

//perform createCV syscall on server
void createCV(char* cName,Table* cTable,int outAddr,int outBox,int fromBox) {
	char* msg = new char[MaxMailSize];
	int location = -1;

	if (!tableItemExist(cName,cTable,2)) {
		serverCV* toPut = new serverCV(cName);
		location = cTable->Put(toPut);
		createCVRequests++;
		msg = "0";
	} else {
		location = getTableIndex(cName,cTable,2);
		msg = "1";
	}

	ServerReply(msg,outAddr,outBox,fromBox);
}

//perform destroyCV syscall on server
void destroyCV(char* cName,Table* cTable,int outAddr,int outBox,int fromBox) {
	char* msg = new char[MaxMailSize];

	if(createCVRequests == 0) {
		if (!tableItemExist(cName, cTable, 2)) {
			msg = "1";
		}
		else { //Delete all the CVs from the. This should only run at the end of the program
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

	ServerReply(msg,outAddr,outBox,fromBox);
}

void createMV(char* lname, int len, Table* mTable, int outAddr, int outBox,int fromBox) {
	char* msg = new char[MaxMailSize];
	int location = 0;

	if(!tableItemExist(lname, mTable, 0)) {
		serverMV* toPut = new serverMV(lname, len);
		location = mTable->Put(toPut);
		createMVRequests++;
		string toSend;
		stringstream sss;
		sss <<"10 "<<location;
		toSend = sss.str();
		msg = (char*) toSend.c_str();
	}
	else {
		location = getTableIndex(lname,mTable,0);
		string toSend;
		stringstream sss;
		sss <<"10 "<< location;
		toSend = sss.str();
		msg = (char*) toSend.c_str();
	}

	ServerReply(msg,outAddr,outBox,fromBox);
}

void destroyMV(char* mName, Table* mTable, int outAddr,int outBox,int fromBox) {
	char* msg = new char[MaxMailSize];

	if(createMVRequests == 0) {
		if (!tableItemExist(mName, mTable, 0)) {
			msg = "1";
		}
		else { //Delete all the MVs from the. This should only run at the end of the program
			int toRemove = mTable->Size();
			toRemove--;
			while(mTable->getCount() != 0) {
				serverMV* tItem = (serverMV*) (mTable->Remove(toRemove));
				toRemove--;
				delete tItem;
				msg = "0";
			}
		}
	}
	else {
		createMVRequests--;
	}

	ServerReply(msg,outAddr,outBox,fromBox);

}

#endif // NETWORK
