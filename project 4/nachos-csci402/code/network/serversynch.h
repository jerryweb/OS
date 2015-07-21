// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "table.h"
#include "network.h"
#include "system.h"
#include "post.h"
//#include <string>
//#include <sstream>

//using namespace std;

enum lockState {
	SL_FREE, SL_BUSY
};

bool tableItemExist(char* tName, Table* table, int tableType);
int getTableIndex(char* tName, Table* table, int tableType);
void ServerReply(char* sMsg, int outMachine, int outMailbox, int fromMailbox);

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
