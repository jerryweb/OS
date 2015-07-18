// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "network.h"
#include "post.h"
//#include <string>
//#include <sstream>

//using namespace std;

enum lockState {
	SL_FREE, SL_BUSY
};

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int outAddr, int outBox);
	void Release(int outAddr, int outBox);

	char* name;
	lockState state;
	int ownerID;       //owner machine id
	int mailboxID;      //owner mailbox number
	List* waitQue;

};

class serverCV {
public:
	serverCV(char* dName);
	~serverCV();

	void Signal(serverLock *sLock, int outAddr, int outBox);
	void Wait(serverLock *sLock, int outAddr, int outBox);
	void Boardcast(serverLock *sLock, int outAddr, int outBox);

	char* name;
	serverLock* waitLock;
	List* waitQue;
};

#endif
