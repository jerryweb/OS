// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "network.h"
#include "post.h"

enum lockState {
	FREE, BUSY
};

/*routine for sending proceed call
//and ackonwledging
int sendProceedMsg(int out) {

}*/

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int out);
	void Release();

	string name;
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
