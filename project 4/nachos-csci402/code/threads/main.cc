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
		srand (time(NULL)); //for userprog calling random server
		serverLockTable = new Table(2048);
		serverCVTable = new Table(2048);
		MVTable = new Table(2048);
		LTRArray = new unsigned int[2];
		for (int i=0;i<2;i++) {
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

/*******************message coding format***************************
 client message:
 (after client message appended to the message list it will has the same format as server message)
 "scIdentifer timestamp requestType requestinfo(varies)"
 server message:
 "scIdentifer machineID mailboxID timestamp requestType requestinfo(varies)"
 *********************************************************************/
void RunServer() {
	//TODO::change syscall.h for MV calls since they need less arguments
	//TODO::get machine ID

	int myId = netname;
	printf("my server id is %d/n",myId);

	/************data declarition****************************/
	PacketHeader inPktHdr;
	MailHeader inMailHdr;
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
	string arg1,arg2,construct,fwd;
	serverLock* sLock;
	serverCV* sCV;
	serverMV* sMV;
	char* cArg1;
	char* cArg2;
	char* cArg3;
	cArg1 = new char[MaxMailSize];
	cArg2 = new char[MaxMailSize];
	cArg3 = new char[MaxMailSize];
	char* eMsg;
	char* reqMsg;
	char* fwdMsg;
	int mailboxID,index2, index3,mValue,mvPos,dummy;//mvPos used as size for create, array index for other functions

	while (true)
	{
		/************Server Forwarding*****************************/
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer); //TODO:change 0 to inbox
		//debug
		printf("\nreceive buffer : %s\n",buffer);

		ss << buffer;
		ss >> scIdentifier;

		//if it's a client message forward it to other servers
		if (scIdentifier == 0) {

			ss.getline(cArg1,MaxMailSize);
			css << "1 " << inPktHdr.from << " " << inMailHdr.from << " "<< cArg1 << " ";
			fwd = css.str();
			//else if it's server msg forward timestamp(only) to other servers
		} else if (scIdentifier == 1) {
			unsigned int fwdTime = getTimeStamp();
			css << "2 "<< myId << " 0 " <<fwdTime << " 14 "; //request type 14 means it's only timestamp update
			fwd = css.str();

			//it it's timestamp only msg, do nothing
		} else {

		}

		//as long as it's not update timestamp msg
		if (scIdentifier != 2) {
			//forward the above just constructed message/timestamp to other servers
			for (int i = 0;i<2;i++) {
				if (i != myId) {
					fwdMsg = new char[MaxMailSize];
					strcpy(fwdMsg,(char*)fwd.c_str());
					printf("forward msg %s to server %d\n",fwdMsg,i);
					ServerReply(fwdMsg,i,0,0); //it'fine to use 0,0 for to, from mailbox
											   //since server is single thread
				}
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
		// read in  original machine id and mailbox if it's server fwd msg
		if (scIdentifier != 0) {
			ss >> original;
			ss >> mailboxID;
		} else {
			original = inPktHdr.from;
			mailboxID = inMailHdr.from;
		}
		ss >> tStamp;
		//update LTRArray (last time received)
		if (LTRArray[original] == 0) {
			LTRArray[original] = tStamp;
		}
		if (LTRArray[original] < tStamp) {
			LTRArray[original] = tStamp;
		}
		//update smallest time stamp from LTR
		sTStamp = LTRArray[0];
		for (int i=1;i<2;i++) {
			if (sTStamp > LTRArray[i]) {
				sTStamp = LTRArray[i];
			}
		}
		if (sTStamp == 0) {   //avoid skipping the initial case
			sTStamp = tStamp;
		}
		//re-construct append the msg to the pending message list(sorted)
		//TODO:maybe skip the timestamp renew msg for my own, good for now
		char* nMsg;
		nMsg = new char[MaxMailSize];
		ss.str("");
		ss.clear();
		ss << buffer;
		ss >> dummy;
		//ss.getline(cArg1,MaxMailSize,' ');
		//ss.getline(cArg1,MaxMailSize,' ');
		ss.getline(cArg1,MaxMailSize);
		css << scIdentifier << " " << original << " " << mailboxID << " " << cArg1;

		//debug
		printf("\nappending pending msg to list\n");
		printf("msg type: %d\n",scIdentifier);
		printf("msg sender: %d\n",original);
		printf("msg mailbox id:  %d\n",mailboxID);
		printf("msg tstamp: %d\n",tStamp);
		printf("msg info: %s\n\n",cArg1);

		construct = css.str();
		strcpy(nMsg,(char*)construct.c_str());
		pendingMsg->SortedInsert((void*)nMsg,tStamp);

		ss.str("");
		ss.clear();
		css.str("");
		css.clear();
		/*****************total ordering ends******************/

		/***************pending msg processing**************/
		//TODO: think about the renew time stamp case, see if it has trap
		while (true) {
			original = myId;  //set orignial server to my id again
			reqMsg = NULL;
			reqMsg = (char*)pendingMsg->Remove();
			if (reqMsg == NULL) {
				break;
			}
			ss << reqMsg;
			ss >> scIdentifier >> original >> mailboxID >> tStamp;

			//debug
			printf("\nhandling pending msg\n ");
			printf("msg type: %d\n",scIdentifier);
			printf("msg sender: %d\n",original);
			printf("msg mailbox id:  %d\n",mailboxID);
			printf("msg tstamp: %d\n",tStamp);
			printf("msg info: %s\n\n",cArg1);

			//if time stamp larger than the smallest time stamp from LTR, abort
			//also re-append the message
			//debug
			printf("timestamp extract is: %d\n",tStamp);
			printf("smallest timestamp: %d\n",sTStamp);

			if (pendingMsg->IsEmpty()) {
				break;
			}

			if (tStamp > sTStamp) {
				pendingMsg->SortedInsert((void*)reqMsg,tStamp);
				printf("finished one loop since I'm on time\n");
				break;
			}

			ss >> request;
			printf("request type is %d\n",request);
			switch (request)
			{
				case 1:   //create lock
				ss >> arg1;
				cArg1 = (char*) arg1.c_str();
				createLock(cArg1, serverLockTable,original,mailboxID,0);
				break;

				case 2://destory lock
				ss >> arg1;
				cArg1 = (char*) arg1.c_str();
				destroyLock(cArg1, serverLockTable,original,mailboxID,0);
				break;

				case 3://acquire lock
				ss >> index;
				sLock = (serverLock*)serverLockTable->Get(index);

				//check if the lock is valid
				if (sLock == NULL) {
					strcpy(eMsg,"1 ");
					if (original == myId) {
						ServerReply(eMsg,original,mailboxID,0);
						break;
					}
					sLock->Acquire(original,mailboxID,0);
					break;

					case 4:   //release lock
					ss >> index;
					//index = getTableIndex(cArg1,serverLockTable,1);
					sLock = (serverLock*)serverLockTable->Get(index);
					//check if the lock is valid
					if (sLock == NULL) {
						strcpy(eMsg,"1 ");
						ServerReply(eMsg,original,mailboxID,0);
						break;
					}
					sLock->Release(original,mailboxID,0);
					break;

					case 5:   //create CV
					ss >> arg1;
					cArg1 = (char*) arg1.c_str();
					createCV(cArg1,serverCVTable,original,mailboxID,0);
					break;

					case 6://destroy CV
					ss >> arg1;
					cArg1 = (char*) arg1.c_str();
					destroyCV(cArg1,serverCVTable,original,mailboxID,0);
					break;

					case 7://CV Signal
					ss >> index;
					ss>>index2;
					sCV = (serverCV*)serverCVTable->Get(index);
					sLock = (serverLock*)serverLockTable->Get(index2);
					if (sCV == NULL || sLock == NULL) {
						strcpy(eMsg,"1 ");
						ServerReply(eMsg,original,mailboxID,0);
						break;
					}
					sCV->Signal(sLock,original,mailboxID,0);
					break;

					case 8:   //CV Wait
					ss >> index;
					ss>>index2;
					sCV = (serverCV*)serverCVTable->Get(index);
					sLock = (serverLock*)serverLockTable->Get(index2);
					if (sCV == NULL || sLock == NULL) {
						strcpy(eMsg,"1 ");
						ServerReply(eMsg,original,mailboxID,0);
						break;
					}
					sCV->Wait(sLock,original,mailboxID,0);
					break;

					case 9:   //CV Broadcast
					ss >> index;
					ss>>index2;
					sCV = (serverCV*)serverCVTable->Get(index);
					sLock = (serverLock*)serverLockTable->Get(index2);
					if (sCV == NULL || sLock == NULL) {
						strcpy(eMsg,"1 ");
						ServerReply(eMsg,original,mailboxID,0);
						break;
					}
					sCV->Boardcast(sLock,original,mailboxID,0);
					break;

					case 10:   //create mv
					ss>> arg1;
					ss>> mvPos;
					cArg1 = (char*) arg1.c_str();
					createMV(cArg1,mvPos, MVTable,original,mailboxID,0);
					break;

					case 11://destroy mv
					ss >> arg1;
					cArg1 = (char*)arg1.c_str();
					destroyMV(cArg1, MVTable,original,mailboxID,0);
					break;

					case 12://read monitor variable
					ss>> index;
					sMV = (serverMV*)MVTable->Get(index);
					if (sMV == NULL) {
						eMsg = new char[MaxMailSize];
						strcpy(eMsg,"1 ");
						ServerReply(eMsg,original,mailboxID,0);
						break;
					}
					ss >> mvPos;
					sMV->Read(mvPos,original,mailboxID,0);
					break;

					case 13: //set the monitor variable
					ss>> index >> mValue;
					sMV = (serverMV*)MVTable->Get(index);
					if (sMV == NULL) {
						eMsg = new char[MaxMailSize];
						strcpy(eMsg,"1 ");
						ServerReply(eMsg,original,mailboxID,0);
						break;
					}
					ss >> mvPos;
					sMV->Set(mValue,mvPos,original,mailboxID,0);
					break;

					case 14: //do nothing, since it's only a timestamp udpate from method 1
					break;

					default:
					printf("invalid request type\n");
					eMsg = new char[MaxMailSize];
					strcpy(eMsg,"1 ");
					ServerReply(eMsg,original,mailboxID,0);
				}

				//TODO:delete msg
			}
			/*****************pending msg processing ends***********************/
		}
		ss.str("");
		ss.clear();
		css.str("");
		css.clear();
	}
	delete [] cArg1;
	delete [] cArg2;
	delete [] cArg3;
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
		strcpy(msg,(char*)toSend.c_str());
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
			strcpy(msg,"1");
		}
		else {//Delete all the locks from the. This should only run at the end of the program
			int toRemove = sTable->Size();
			toRemove--;
			while(sTable->getCount() != 0) {
				serverLock* tItem = (serverLock*) (sTable->Remove(toRemove));
				toRemove--;
				delete tItem;
				strcpy(msg,"0");
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
		strcpy(msg,"0 ");
	} else {
		location = getTableIndex(cName,cTable,2);
		strcpy(msg,"1 ");
	}

	ServerReply(msg,outAddr,outBox,fromBox);
}

//perform destroyCV syscall on server
void destroyCV(char* cName,Table* cTable,int outAddr,int outBox,int fromBox) {
	char* msg = new char[MaxMailSize];

	if(createCVRequests == 0) {
		if (!tableItemExist(cName, cTable, 2)) {
			strcpy(msg,"1 ");
		}
		else { //Delete all the CVs from the. This should only run at the end of the program
			int toRemove = cTable->Size();
			toRemove--;
			while(cTable->getCount() != 0) {
				serverCV* cItem = (serverCV*) (cTable->Remove(toRemove));
				toRemove--;
				delete cItem;
				strcpy(msg,"0 ");
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
		strcpy(msg,(char*) toSend.c_str());
	}

	ServerReply(msg,outAddr,outBox,fromBox);
}

void destroyMV(char* mName, Table* mTable, int outAddr,int outBox,int fromBox) {
	char* msg = new char[MaxMailSize];

	if(createMVRequests == 0) {
		if (!tableItemExist(mName, mTable, 0)) {
			strcpy(msg,"1 ");
		}
		else { //Delete all the MVs from the. This should only run at the end of the program
			int toRemove = mTable->Size();
			toRemove--;
			while(mTable->getCount() != 0) {
				serverMV* tItem = (serverMV*) (mTable->Remove(toRemove));
				toRemove--;
				delete tItem;
				strcpy(msg,"0 ");
			}
		}
	}
	else {
		createMVRequests--;
	}

	ServerReply(msg,outAddr,outBox,fromBox);

}

#endif // NETWORK
