// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "network.h"
#include "post.h"

enum lockState {
	SL_FREE, SL_BUSY
};

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int out);
	void Release(int out);

	char* name;     //for debug
	lockState state;
	int owenerID;       //owner machine id
	int mailboxID;      //owner mailbox number
	List* waitQue;

};

class serverCV {
public:
	serverCV(char* dName);
	~serverCV();

	void Signal(serverLock *sLock);
	void Wait(serverLock *sLock);

	char* name;
	serverLock* lock;
	List* waitQue;
};

#endif
